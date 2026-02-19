#include "DatabaseManager.h"
#include "CustomerService.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>
CustomerService::CustomerService(DatabaseManager* db)
	: db(db)
{
	if (!db) {
		throw std::invalid_argument("CustomerService: db cannot be null");
	}
}
void CustomerService::validateOwnership(UserId userId, VehicleId vehicleId)
{
	auto rec = db->getVehicleById(vehicleId);
	if (!rec.has_value()) {
		throw std::runtime_error("validateOwnership: vehicle not found after creation");
	}

	if (rec->ownerUserId != userId) {
		throw std::runtime_error("validateOwnership: vehicle ownership mismatch");
	}
}


VehicleId CustomerService::addVehicle(UserId customerId, const VehicleCreate& vehicle) {
	//This function assigns a vehicle to a user
	/*
		The inital checks that this function must do are the following:
			- Checks if this vechicle is already assigned to the user
			- Checks all fields in the Vechile Create DTO to make sure none are null
			- Creates record in Database and assigns the vechile to the user
	
	*/
	if (!db.userExists(customerId)) {
		throw std::invalid_argument("addVehicle: customer not found");
	}

	// 1) Validate VehicleCreate (service/business validation)
	// In C++ you don't check "null" for std::string; you check empty.
	if (vehicle.vin.empty()) {
		throw std::invalid_argument("addVehicle: VIN is required");
	}
	if (vehicle.model.empty()) {
		throw std::invalid_argument("addVehicle: model is required");
	}
	auto existing = db.getVehicleByVin(vehicle.vin);
	if (existing.has_value()) {
		// NOTE: requires VehicleRecord to include ownerUserId and id
		if (existing->ownerUserId == customerId) {
			// Idempotent: vehicle already assigned to this customer
			return existing->id;
		}
		throw std::runtime_error("addVehicle: VIN already registered to another customer");
	}
	VehicleRecord rec{};
	rec.vin = vehicle.vin;
	rec.make = vehicle.make;   // if optional, empty string is fine
	rec.model = vehicle.model;
	rec.year = vehicle.year;   // if optional, use 0 (your DB code stores NULL when <=0)

	// 4) Persist (this “assigns” to customer via ownerUserId stored in the vehicle row)
	VehicleId newVehicleId = db.createVehicle(customerId, rec);
	validateOwnership(customerId, newVehicleId);

	return newVehicleId;


}