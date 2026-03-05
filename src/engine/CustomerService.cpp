#include "DatabaseManager.h"
#include "CustomerService.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <cmath>

CustomerService::CustomerService(DatabaseManager *db, RatingEngine &ratingEngine,
								 ProfitabilityEngine &profitabilityEngine, CustomerValidator &validator)
	: db(db), ratingEngine(ratingEngine), profitabilityEngine(profitabilityEngine), validator(validator)
{
	if (!this->db)
	{
		throw std::invalid_argument("CustomerService: db is null");
	}
}

VehicleId CustomerService::addVehicle(UserId customerId, const VehicleCreate &vehicle)
{
	if (customerId <= 0)
	{
		throw std::invalid_argument("addVehicle: invalid customerId");
	}

	if (!validator.validateVehicle(vehicle))
	{
		throw std::invalid_argument("addVehicle: vehicle validation failed");
	}

	auto user = db->getUserRecordById(customerId);
	if (!user.has_value())
	{
		throw std::runtime_error("addVehicle: customer not found");
	}
	if (user->role != UserRole::CUSTOMER)
	{
		throw std::runtime_error("addVehicle: user is not a customer");
	}

	VehicleRecord record{};
	record.ownerUserId = customerId;
	record.vin = vehicle.vin;
	record.make = vehicle.make;
	record.model = vehicle.model;
	record.year = vehicle.year;
	record.mileage = vehicle.mileage;

	return db->createVehicle(customerId, record);
}

VehicleDTO CustomerService::getVehicle(VehicleId vehicleId)
{
	if (vehicleId <= 0)
	{
		throw std::invalid_argument("getVehicle: invalid vehicleId");
	}

	auto vehicle = db->getVehicleById(vehicleId);
	if (!vehicle.has_value())
	{
		throw std::runtime_error("getVehicle: vehicle not found");
	}

	VehicleDTO dto{};
	dto.vehicleId = vehicle->id;
	dto.ownerId = vehicle->ownerUserId;
	dto.vin = vehicle->vin;
	dto.make = vehicle->make;
	dto.model = vehicle->model;
	dto.year = vehicle->year;
	dto.mileage = vehicle->mileage;

	return dto;
}

std::vector<VehicleDTO> CustomerService::listVehicles(UserId customerId)
{
	if (customerId <= 0)
	{
		throw std::invalid_argument("listVehicles: invalid customerId");
	}

	std::vector<VehicleDTO> result;
	auto vehicles = db->listVehiclesForUser(customerId);
	result.reserve(vehicles.size());

	for (const auto &v : vehicles)
	{
		VehicleDTO dto{};
		dto.vehicleId = v.id;
		dto.ownerId = v.ownerUserId;
		dto.vin = v.vin;
		dto.make = v.make;
		dto.model = v.model;
		dto.year = v.year;
		dto.mileage = v.mileage;
		result.push_back(std::move(dto));
	}

	return result;
}

bool CustomerService::updateVehicle(VehicleId vehicleId, const VehicleUpdate &updates)
{
	if (vehicleId <= 0)
	{
		return false;
	}

	return db->updateVehicle(vehicleId, updates);
}

bool CustomerService::removeVehicle(VehicleId vehicleId)
{
	if (vehicleId <= 0)
	{
		return false;
	}

	return db->deleteVehicle(vehicleId);
}

std::vector<MechanicMatch> CustomerService::findMatchingMechanics(UserId customerId, SymptomFormId formId)
{
	if (customerId <= 0 || formId <= 0)
	{
		throw std::invalid_argument("findMatchingMechanics: invalid ids");
	}

	auto user = db->getUserRecordById(customerId);
	if (!user.has_value())
	{
		throw std::runtime_error("findMatchingMechanics: customer not found");
	}
	if (user->role != UserRole::CUSTOMER)
	{
		throw std::runtime_error("findMatchingMechanics: user is not a customer");
	}

	MechanicSearchFilter filters{};
	auto candidates = db->searchMechanics(filters);

	std::vector<MechanicMatch> matches;
	matches.reserve(candidates.size());

	for (const auto &m : candidates)
	{
		MechanicMatch match{};
		match.mechanicId = m.userId;
		match.matchScore = ratingEngine.getechanicScore(m.userId);
		match.reasons.push_back("default match");
		matches.push_back(std::move(match));
	}

	std::sort(matches.begin(), matches.end(),
			  [](const MechanicMatch &a, const MechanicMatch &b)
			  {
				  return a.matchScore > b.matchScore;
			  });

	return matches;
}

MechanicDTO CustomerService::viewMechanicProfile(MechanicId mechanicId)
{
	if (mechanicId <= 0)
	{
		throw std::invalid_argument("viewMechanicProfile: invalid mechanicId");
	}

	auto mechanic = db->getMechanicByUserId(mechanicId);
	if (!mechanic.has_value())
	{
		throw std::runtime_error("viewMechanicProfile: mechanic not found");
	}

	MechanicDTO dto{};
	dto.mechanicId = mechanic->userId;
	dto.userId = mechanic->userId;
	dto.displayName = mechanic->displayName;
	dto.shopName = mechanic->shopName;
	dto.hourlyRate = mechanic->hourlyRate;
	dto.specialties = mechanic->specialties;
	dto.averageRating = ratingEngine.getechanicScore(mechanic->userId);
	dto.reviewCount = 0;

	return dto;
}

PriceEstimate CustomerService::requestEstimate(MechanicId mechanicId, SymptomFormId formId)
{
	if (mechanicId <= 0 || formId <= 0)
	{
		throw std::invalid_argument("requestEstimate: invalid ids");
	}

	auto mechanic = db->getMechanicByUserId(mechanicId);
	if (!mechanic.has_value())
	{
		throw std::runtime_error("requestEstimate: mechanic not found");
	}

	PriceEstimate estimate{};
	int labor = mechanic->hourlyRate > 0.0 ? static_cast<int>(std::lround(mechanic->hourlyRate)) : 0;
	estimate.laborCost = labor;
	estimate.partsCost = 0;
	estimate.tax = 0;
	estimate.total = estimate.laborCost + estimate.partsCost + estimate.tax;
	estimate.note = "estimate";

	return estimate;
}

JobDTO CustomerService::getJobStatus(JobId jobId)
{
	if (jobId <= 0)
	{
		throw std::invalid_argument("getJobStatus: invalid jobId");
	}

	auto job = db->getJobById(jobId);
	if (!job.has_value())
	{
		throw std::runtime_error("getJobStatus: job not found");
	}

	JobDTO dto{};
	dto.jobId = job->id;
	dto.appointmentId = job->appointmentId;
	dto.customerId = job->customerId;
	dto.mechanicId = job->mechanicId;
	dto.currentStage = job->stage;
	dto.percentComplete = job->percentComplete;
	dto.lastNote = job->lastNote;
	dto.updatedAt = job->updatedAt;
	dto.startedAt = job->startedAt;
	dto.completedAt = job->completedAt;
	dto.completionNote = job->completionNote;

	return dto;
}

void CustomerService::unsubscribeFromJobUpdates(SubscriptionId subscriptionId)
{
	if (subscriptionId <= 0)
	{
		throw std::invalid_argument("unsubscribeFromJobUpdates: invalid subscriptionId");
	}
	// No-op: subscription storage not implemented in current engine.
}

// =====================================================================
//  Private helper stubs
// =====================================================================

// TODO: Implement ownership check — verify customerId owns vehicleId
void CustomerService::validateOwnership(UserId customerId, VehicleId vehicleId)
{
	// TODO: Implement — throw if customer does not own vehicle
}

// TODO: Implement form ownership check — verify customerId owns formId
void CustomerService::validateFormOwnership(UserId customerId, SymptomFormId formId)
{
	// TODO: Implement — throw if customer does not own symptom form
}

// TODO: Implement review eligibility check
bool CustomerService::canReviewJob(UserId customerId, JobId jobId)
{
	// TODO: Implement — return true if customer completed the job and hasn't reviewed yet
	return false;
}

// TODO: Implement candidate mechanic loading based on symptom form and filters
std::vector<MechanicRecord> CustomerService::loadCandidateMechanics(const SymptomFormDTO &form, const MechanicSearchFilter &filters)
{
	// TODO: Implement — query DB for mechanics matching the form/filters
	return {};
}

// TODO: Implement scoring and ranking of candidate mechanics
std::vector<MechanicMatch> CustomerService::scoreAndRank(const std::vector<MechanicRecord> &candidates, const SymptomFormDTO &form)
{
	// TODO: Implement — use ratingEngine/profitabilityEngine to score and sort candidates
	return {};
}

// =====================================================================
//  Customer Profile Management stubs
// =====================================================================

// TODO: Implement customer profile creation
UserId CustomerService::createCustomerProfile(const CustomerCreate &profile)
{
	if(!validator.validateCustomerProfile(profile)) throw std::invalid_argument("createCustomerProfile: invalid profile");
	UserRecord rec{};
	rec.name = profile.fullName;
	rec.email = profile.email;
	rec.passwordHash = profile.passwordHash;
	rec.role = UserRole::CUSTOMER;
	rec.createdAt = profile.createdAt;
	rec.phone = profile.phone;
	
	return db->createUser(rec);
}

// TODO: Implement get customer profile
CustomerDTO CustomerService::getCustomerProfile(UserId customerId)
{
	if (customerId <= 0)
	{
		throw std::invalid_argument("getCustomerProfile: invalid customerId");
	}
	auto user = db->getUserRecordById(customerId);
	if (!user.has_value())
	{
		throw std::runtime_error("getCustomerProfile: user not found");
	}
	CustomerDTO dto;
	dto.userId = user->id;
	dto.email = user->email;
	dto.fullName = user->name;
	dto.phone = user->phone;
	dto.createdAt = user->createdAt;
	return dto;
}


// TODO: Implement customer profile update
bool CustomerService::updateCustomerProfile(UserId id,CustomerProfileUpdate &update)
{
	
	if(id <= 0) throw std::invalid_argument("updateCustomerProfile: invalid customerId");
	UserUpdate recUpdate{};
	if(update.fullName.has_value()) recUpdate.fullname = update.fullName;
	if(update.email.has_value()) recUpdate.email = update.email;
	if(update.phone.has_value()) recUpdate.phone = update.phone;
	return db->updateUserRecord(id, recUpdate);
}

bool CustomerService::deleteCustomerProfile(UserId customerId)
{
	if (customerId <= 0)
	{
		throw std::invalid_argument("deleteCustomerProfile: invalid customerId");
	}
	return db->deleteUser(customerId);
}

bool CustomerService::updateCustomerPasswordHash(UserId customerId, const std::string &newHash)
{
	if(customerId <=0) throw std::invalid_argument("updateCustomerPasswordHash: invalid customerId");
	return db->updatePasswordHash(customerId, newHash);
}
// =====================================================================
//  Symptom Form Management stubs
// =====================================================================

// TODO: Implement symptom form creation
SymptomFormId CustomerService::createSymptomForm(UserId customerId, VehicleId vehicleId, const SymptomFormCreate &form)
{
	// TODO: Implement — validate ownership, validate form, call db->createSymptomForm()
	return -1;
}

// TODO: Implement get single symptom form
SymptomFormDTO CustomerService::getSymptomForm(SymptomFormId formId)
{
	// TODO: Implement — fetch from DB, convert SymptomFormRecord to SymptomFormDTO
	return {};
}

// TODO: Implement list symptom forms for customer
std::vector<SymptomFormDTO> CustomerService::listSymptomForms(UserId customerId)
{
	// TODO: Implement — call db->listSymptomFormsForCustomer(), convert to DTOs
	return {};
}

// TODO: Implement symptom form update
bool CustomerService::updateSymptomForm(SymptomFormId formId, const SymptomFormUpdate &updates)
{
	// TODO: Implement — validate, call db->updateSymptomForm()
	return false;
}

// TODO: Implement symptom form deletion
bool CustomerService::deleteSymptomForm(SymptomFormId formId)
{
	// TODO: Implement — call db->deleteSymptomForm()
	return false;
}

// =====================================================================
//  Appointment Management stubs
// =====================================================================

// TODO: Implement appointment request

AppointmentId CustomerService::requestAppointment(AppointmentCreate appointment){
    if(!validator.validateAppointment(appointment))     throw std::invalid_argument("requestAppointment: invalid appointment");
    AppointmentRecord rec{};
    rec.customerId = appointment.customerId;
    rec.mechanicId = appointment.mechanicId;
    rec.symptomFormId = appointment.formId;
    rec.vehicleId = appointment.vehicleId;
    rec.scheduledAt = appointment.scheduledAt;
    rec.note = appointment.note;
    AppointmentId id = db->createAppointment(rec);
    return id;  
}
bool CustomerService::cancelAppointment(AppointmentId appointmentId, const std::string& reason){
    if(appointmentId <=0) throw std::invalid_argument("cancelAppointment: invalid appointmentId");
    bool result = db->cancelAppointment(appointmentId, reason);
    return result;

}
AppointmentDTO CustomerService::getAppointment(AppointmentId appointmentId){
    if(appointmentId <= 0) {
        throw std::invalid_argument("getAppointment: invalid appointmentId");
    }

    auto appointment = db->getAppointmentById(appointmentId);
    if(!appointment.has_value()) {
        throw std::runtime_error("getAppointment: appointment not found");
    }

    AppointmentDTO dto{};
    dto.appointmentId = appointment->appointmentId;
    dto.customerId = appointment->customerId;
    dto.mechanicId = appointment->mechanicId;
    dto.formId = appointment->symptomFormId;
    dto.vehicleId = appointment->vehicleId;
    dto.scheduledAt = appointment->scheduledAt;
    dto.status = appointment->status;
    dto.note = appointment->note;
    dto.createdAt = appointment->createdAt;
    dto.symptoms = appointment->symptomForm;

    return dto;
}

bool CustomerService::confirmAppointment(AppointmentId appointmentId)
{
    if (appointmentId <= 0) {
        throw std::invalid_argument("confirmAppointment: invalid appointmentId");
    }
    return db->updateAppointmentStatus(appointmentId,AppointmentStatus::CONFIRMED);
}

std::vector<AppointmentDTO> CustomerService::listAppointments(UserId customerId)
{
    auto appointments = db->listAppointmentsForCustomer(customerId);
    std::vector<AppointmentDTO> dtos;
    for (const auto &appointment : appointments) {
        AppointmentDTO dto{};
        dto.appointmentId = appointment.appointmentId;
        dto.customerId = appointment.customerId;
        dto.mechanicId = appointment.mechanicId;
        dto.formId = appointment.symptomFormId;
        dto.vehicleId = appointment.vehicleId;
        dto.scheduledAt = appointment.scheduledAt;
        dto.status = appointment.status;
        dto.note = appointment.note;
        dto.createdAt = appointment.createdAt;
        dtos.push_back(dto);
    }
    return dtos;
}


// =====================================================================
//  Review Management stubs
// =====================================================================

// TODO: Implement review submission
ReviewId CustomerService::submitReview(UserId customerId, JobId jobId, const ReviewCreate &review)
{
	// TODO: Implement — validate with canReviewJob(), create ReviewRecord, call db->createReview()
	return -1;
}

// TODO: Implement list reviews by customer
std::vector<ReviewDTO> CustomerService::listMyReviews(UserId customerId)
{
	// TODO: Implement — call db->listReviewsForCustomer(), convert to DTOs
	return {};
}

// TODO: Implement review deletion
bool CustomerService::deleteMyReview(UserId customerId, ReviewId reviewId)
{
	// TODO: Implement — verify ownership, call db->deleteReview()
	return false;
}
