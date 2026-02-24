#include "DatabaseManager.h"
#include "CustomerService.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <cmath>

CustomerService::CustomerService(DatabaseManager* db, RatingEngine& ratingEngine,
								 ProfitabilityEngine& profitabilityEngine, CustomerValidator& validator)
	: db(db)
	, ratingEngine(ratingEngine)
	, profitabilityEngine(profitabilityEngine)
	, validator(validator)
{
	if (!this->db) {
		throw std::invalid_argument("CustomerService: db is null");
	}
}

VehicleId CustomerService::addVehicle(UserId customerId, const VehicleCreate& vehicle)
{
	if (customerId <= 0) {
		throw std::invalid_argument("addVehicle: invalid customerId");
	}

	if (!validator.validateVehicle(vehicle)) {
		throw std::invalid_argument("addVehicle: vehicle validation failed");
	}

	auto user = db->getUserRecordById(customerId);
	if (!user.has_value()) {
		throw std::runtime_error("addVehicle: customer not found");
	}
	if (user->role != UserRole::CUSTOMER) {
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

std::vector<VehicleDTO> CustomerService::listVehicles(UserId customerId)
{
	if (customerId <= 0) {
		throw std::invalid_argument("listVehicles: invalid customerId");
	}

	std::vector<VehicleDTO> result;
	auto vehicles = db->listVehiclesForUser(customerId);
	result.reserve(vehicles.size());

	for (const auto& v : vehicles) {
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

bool CustomerService::updateVehicle(VehicleId vehicleId, const VehicleUpdate& updates)
{
	if (vehicleId <= 0) {
		return false;
	}

	return db->updateVehicle(vehicleId, updates);
}

bool CustomerService::removeVehicle(VehicleId vehicleId)
{
	if (vehicleId <= 0) {
		return false;
	}

	return db->deleteVehicle(vehicleId);
}

std::vector<MechanicMatch> CustomerService::findMatchingMechanics(UserId customerId, SymptomFormId formId)
{
	if (customerId <= 0 || formId <= 0) {
		throw std::invalid_argument("findMatchingMechanics: invalid ids");
	}

	auto user = db->getUserRecordById(customerId);
	if (!user.has_value()) {
		throw std::runtime_error("findMatchingMechanics: customer not found");
	}
	if (user->role != UserRole::CUSTOMER) {
		throw std::runtime_error("findMatchingMechanics: user is not a customer");
	}

	MechanicSearchFilter filters{};
	auto candidates = db->searchMechanics(filters);

	std::vector<MechanicMatch> matches;
	matches.reserve(candidates.size());

	for (const auto& m : candidates) {
		MechanicMatch match{};
		match.mechanicId = m.userId;
		match.matchScore = ratingEngine.getechanicScore(m.userId);
		match.reasons.push_back("default match");
		matches.push_back(std::move(match));
	}

	std::sort(matches.begin(), matches.end(),
		[](const MechanicMatch& a, const MechanicMatch& b) {
			return a.matchScore > b.matchScore;
		});

	return matches;
}

MechanicDTO CustomerService::viewMechanicProfile(MechanicId mechanicId)
{
	if (mechanicId <= 0) {
		throw std::invalid_argument("viewMechanicProfile: invalid mechanicId");
	}

	auto mechanic = db->getMechanicByUserId(mechanicId);
	if (!mechanic.has_value()) {
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
	if (mechanicId <= 0 || formId <= 0) {
		throw std::invalid_argument("requestEstimate: invalid ids");
	}

	auto mechanic = db->getMechanicByUserId(mechanicId);
	if (!mechanic.has_value()) {
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
	if (jobId <= 0) {
		throw std::invalid_argument("getJobStatus: invalid jobId");
	}

	auto job = db->getJobById(jobId);
	if (!job.has_value()) {
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
	if (subscriptionId <= 0) {
		throw std::invalid_argument("unsubscribeFromJobUpdates: invalid subscriptionId");
	}
	// No-op: subscription storage not implemented in current engine.
}
