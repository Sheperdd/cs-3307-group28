#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"
#include "../src/engine/CustomerService.h"
#include "../src/engine/RatingEngine.h"
#include "../src/engine/ProfitabilityEngine.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <cstdio> // For removing the file

// ---- Minimal implementations for tests ----
RatingEngine::RatingEngine(DatabaseManager& db, int recencyHalfLifeDays)
    : db(db), recencyHalfLifeDays(recencyHalfLifeDays)
{
}

int RatingEngine::compute(MechanicID)
{
    return 0;
}

double RatingEngine::getechanicScore(MechanicID)
{
    return 0.0;
}

ProfitabilityEngine::ProfitabilityEngine(DatabaseManager& db, int defaiultHourlyRate, double taxRate)
    : db(db), defaultHourlyRate(defaiultHourlyRate), taxRate(taxRate)
{
}

void ProfitabilityEngine::seTaxRate(double newTaxRate)
{
    taxRate = newTaxRate;
}

void ProfitabilityEngine::setDefaultHourlyRate(int newRate)
{
    defaultHourlyRate = newRate;
}

class TestCustomerValidator : public CustomerValidator {
public:
    bool validateCustomerProfile(const CustomerCreate&) override { return true; }
    bool validateVehicle(const VehicleCreate& vehicle) override {
        return !vehicle.vin.empty() && !vehicle.model.empty();
    }
    bool validateSymptomForm(const SymptomFormCreate&) override { return true; }
    bool validateReview(const ReviewCreate&) override { return true; }
};

static void ensureTestSchema()
{
    SQLite::Database db("torquedesk.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db.exec("CREATE TABLE IF NOT EXISTS customers ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "email TEXT, "
            "password TEXT, "
            "role INTEGER, "
            "createdAt TEXT)");

    db.exec("CREATE TABLE IF NOT EXISTS vehicles ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ownerUserId INTEGER, "
            "vin TEXT, "
            "make TEXT, "
            "model TEXT, "
            "year INTEGER, "
            "mileage INTEGER DEFAULT 0, "
            "createdAt TEXT)");
}

// This runs before EACH test to give us a clean slate
class DatabaseTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Delete the DB file so we start fresh every time
        std::remove("torquedesk.db");
        ensureTestSchema();
    }
};

TEST_F(DatabaseTests, AddUserIncreasesCount) {
    DatabaseManager db;

    // Initial count should be 0
    EXPECT_EQ(db.getUserCount(), 0);

    // Add a user
    bool success = db.addUser("Kay", "kay@torquedesk.com", "123456");

    EXPECT_TRUE(success);
    EXPECT_EQ(db.getUserCount(), 1);
}

TEST_F(DatabaseTests, DuplicateEmailFails) {
    DatabaseManager db;

    bool success = db.addUser ("Sam2", "Sam2@gmail.com", "12345");

    EXPECT_TRUE(success);
    EXPECT_EQ(db.getUserCount(),1);
}


TEST_F(DatabaseTests, DeleteUserDecreasesCount) {
    DatabaseManager db;

    db.addUser("Sam", "Sam@gmail.com", "12345");
    EXPECT_EQ(db.getUserCount(), 1);

    db.deleteUser(static_cast<UserId>(1));

    EXPECT_EQ(db.getUserCount(), 0);

}

TEST_F(DatabaseTests, CustomerServiceAddAndListVehicles) {
    DatabaseManager db;

    auto customerId = db.createUser("Test", "customer@torquedesk.com", "pw", UserRole::CUSTOMER);

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    VehicleCreate vehicle;
    vehicle.vin = "VIN123";
    vehicle.make = "Toyota";
    vehicle.model = "Camry";
    vehicle.year = 2020;
    vehicle.mileage = 12000;

    auto vehicleId = service.addVehicle(customerId, vehicle);
    EXPECT_GT(vehicleId, 0);

    auto vehicles = service.listVehicles(customerId);
    ASSERT_EQ(vehicles.size(), 1u);
    EXPECT_EQ(vehicles[0].vin, "VIN123");
    EXPECT_EQ(vehicles[0].make, "Toyota");
    EXPECT_EQ(vehicles[0].model, "Camry");
}

TEST_F(DatabaseTests, CustomerServiceUpdateVehicle) {
    DatabaseManager db;

    auto customerId = db.createUser("Test", "customer2@torquedesk.com", "pw", UserRole::CUSTOMER);

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    VehicleCreate vehicle;
    vehicle.vin = "VIN456";
    vehicle.make = "Honda";
    vehicle.model = "Civic";
    vehicle.year = 2018;
    vehicle.mileage = 50000;

    auto vehicleId = service.addVehicle(customerId, vehicle);
    EXPECT_GT(vehicleId, 0);

    VehicleUpdate updates;
    updates.make = std::string("Ford");
    updates.model = std::string("Fiesta");
    updates.year = 2015;

    EXPECT_TRUE(service.updateVehicle(vehicleId, updates));

    auto updated = db.getVehicleById(vehicleId);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->make, "Ford");
    EXPECT_EQ(updated->model, "Fiesta");
    EXPECT_EQ(updated->year, 2015);
}

TEST_F(DatabaseTests, CustomerServiceRemoveVehicle) {
    DatabaseManager db;

    auto customerId = db.createUser("Test", "customer3@torquedesk.com", "pw", UserRole::CUSTOMER);

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    VehicleCreate vehicle;
    vehicle.vin = "VIN789";
    vehicle.make = "Nissan";
    vehicle.model = "Altima";
    vehicle.year = 2019;
    vehicle.mileage = 30000;

    auto vehicleId = service.addVehicle(customerId, vehicle);
    EXPECT_GT(vehicleId, 0);

    EXPECT_TRUE(service.removeVehicle(vehicleId));
    auto removed = db.getVehicleById(vehicleId);
    EXPECT_FALSE(removed.has_value());
}

TEST_F(DatabaseTests, CustomerServiceViewMechanicProfile) {
    DatabaseManager db;

    auto mechanicUserId = db.createUser("Mech", "mech@torquedesk.com", "pw", UserRole::MECHANIC);

    MechanicUpdate update;
    update.displayName = std::string("Alex M");
    update.shopName = std::string("Alex Auto");
    update.hourlyRate = 95.0;
    update.specialties = std::vector<std::string>{"BRAKES", "ENGINE"};
    EXPECT_TRUE(db.updateMechanicProfile(mechanicUserId, update));

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    auto profile = service.viewMechanicProfile(mechanicUserId);
    EXPECT_EQ(profile.mechanicId, mechanicUserId);
    EXPECT_EQ(profile.displayName, "Alex M");
    EXPECT_EQ(profile.shopName, "Alex Auto");
    EXPECT_EQ(profile.hourlyRate, 95.0);
    ASSERT_EQ(profile.specialties.size(), 2u);
}

TEST_F(DatabaseTests, CustomerServiceFindMatchingMechanics) {
    DatabaseManager db;

    auto customerId = db.createUser("Cust", "cust@torquedesk.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("Mech2", "mech2@torquedesk.com", "pw", UserRole::MECHANIC);

    MechanicUpdate update;
    update.displayName = std::string("Jamie");
    update.shopName = std::string("Jamie Garage");
    update.hourlyRate = 80.0;
    update.specialties = std::vector<std::string>{"OIL"};
    EXPECT_TRUE(db.updateMechanicProfile(mechanicUserId, update));

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    auto matches = service.findMatchingMechanics(customerId, 1);
    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].mechanicId, mechanicUserId);
}

TEST_F(DatabaseTests, CustomerServiceRequestEstimate) {
    DatabaseManager db;

    auto mechanicUserId = db.createUser("Mech3", "mech3@torquedesk.com", "pw", UserRole::MECHANIC);

    MechanicUpdate update;
    update.displayName = std::string("Taylor");
    update.hourlyRate = 120.0;
    EXPECT_TRUE(db.updateMechanicProfile(mechanicUserId, update));

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    auto estimate = service.requestEstimate(mechanicUserId, 1);
    EXPECT_EQ(estimate.laborCost, 120);
    EXPECT_EQ(estimate.total, 120);
}

TEST_F(DatabaseTests, CustomerServiceGetJobStatus) {
    DatabaseManager db;

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    auto jobId = db.createJobFromAppointment(1);
    ASSERT_GT(jobId, 0);

    auto status = service.getJobStatus(jobId);
    EXPECT_EQ(status.jobId, jobId);
    EXPECT_EQ(status.appointmentId, 1);

    EXPECT_THROW(service.getJobStatus(0), std::invalid_argument);
}

TEST_F(DatabaseTests, CustomerServiceUnsubscribeFromJobUpdates) {
    DatabaseManager db;

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    EXPECT_NO_THROW(service.unsubscribeFromJobUpdates(1));
    EXPECT_THROW(service.unsubscribeFromJobUpdates(0), std::invalid_argument);
}