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
	// TODO: Implement — validate profile, call db->createUser(), return new UserId
	return -1;
}

// TODO: Implement get customer profile
CustomerDTO CustomerService::getCustomerProfile(UserId customerId)
{
	// TODO: Implement — fetch user record from DB, convert to CustomerDTO
	return {};
}

// TODO: Implement customer profile update
bool CustomerService::updateCustomerProfile(UserId customerId, const CustomerProfileUpdate &updates)
{
	// TODO: Implement — validate updates, call db->updateUserRecord()
	return false;
}

bool CustomerService::deleteCustomerProfile(UserId customerId)
{
	// TODO implement — call db->deleteUser(customerId)
	return false;
}

bool CustomerService::updateCustomerPasswordHash(UserId customerId, const std::string &newHash)
{
	// TODO implement — validate new hash, call db->updatePasswordHash(customerId, newHash)
	return false;
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
AppointmentId CustomerService::requestAppointment(UserId customerId, MechanicId mechanicId,
												  SymptomFormId formId, const std::string &scheduledAt)
{
	// TODO: Implement — validate inputs, create AppointmentRecord, call db->createAppointment()
	return -1;
}

// TODO: Implement appointment confirmation
bool CustomerService::confirmAppointment(AppointmentId appointmentId)
{
	// TODO: Implement — call db->updateAppointmentStatus(appointmentId, CONFIRMED)
	return false;
}

// TODO: Implement appointment cancellation
bool CustomerService::cancelAppointment(AppointmentId appointmentId, const std::string &reason)
{
	// TODO: Implement — call db->cancelAppointment(appointmentId, reason)
	return false;
}

// TODO: Implement list appointments for customer
std::vector<AppointmentDTO> CustomerService::listAppointments(UserId customerId)
{
	// TODO: Implement — call db->listAppointmentsForCustomer(), convert to DTOs
	return {};
}

// TODO: Implement get single appointment
AppointmentDTO CustomerService::getAppointment(AppointmentId appointmentId)
{
	// TODO: Implement — call db->getAppointmentById(), convert to AppointmentDTO
	return {};
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
