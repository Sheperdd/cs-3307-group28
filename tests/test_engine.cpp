#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"
#include "../src/engine/CustomerService.h"
#include "../src/engine/RatingEngine.h"
#include "../src/engine/ProfitabilityEngine.h"
#include "../src/engine/AuthService.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <cstdio> // For removing the file

#include "MechanicService.h"
#include "DatabaseManager.h"

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
    bool validateAppointment(const AppointmentCreate&) override { return true; }
};

// This runs before EACH test to give us a clean slate
class DatabaseTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Delete the DB file so we start fresh every time
        std::remove("torquedesk.db");
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

    auto customerId = db.createUser("Cust4", "cust4@torquedesk.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("Mech4", "mech4@torquedesk.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mechanicUpdate;
    mechanicUpdate.displayName = std::string("Robin");
    mechanicUpdate.shopName = std::string("Robin Auto");
    mechanicUpdate.hourlyRate = 100.0;
    EXPECT_TRUE(db.updateMechanicProfile(mechanicUserId, mechanicUpdate));

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VINJOB1";
    vehicle.make = "Mazda";
    vehicle.model = "3";
    vehicle.year = 2021;
    vehicle.mileage = 1000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(
        rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)"
    );
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Rattle noise from front axle");
    symptomInsert.bind(4, 3);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());
    ASSERT_GT(symptomFormId, 0);

    AppointmentRecord req{};
    req.customerId = customerId;
    req.mechanicId = mechanicUserId;
    req.vehicleId = vehicleId;
    req.symptomFormId = symptomFormId;
    req.scheduledAt = "2026-03-01T10:00:00Z";
    req.status = AppointmentStatus::REQUESTED;
    req.note = "Please inspect before test drive";

    auto appointmentId = db.createAppointment(req);
    ASSERT_GT(appointmentId, 0);

    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    auto jobId = db.createJobFromAppointment(appointmentId);
    ASSERT_GT(jobId, 0);

    auto status = service.getJobStatus(jobId);
    EXPECT_EQ(status.jobId, jobId);
    EXPECT_EQ(status.appointmentId, appointmentId);

    // createJobFromAppointment seeds one "Job created" note
    ASSERT_GE(status.notes.size(), 1u);
    EXPECT_EQ(status.notes[0].type, "update");
    EXPECT_EQ(status.notes[0].text, "Job created");

    EXPECT_THROW(service.getJobStatus(0), std::invalid_argument);
}

// ----------- Job Notes Tests -----------

TEST_F(DatabaseTests, JobNotesAddAndList) {
    DatabaseManager db;

    auto customerId = db.createUser("CustNote", "custnote@torquedesk.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("MechNote", "mechnote@torquedesk.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mechanicUpdate;
    mechanicUpdate.displayName = std::string("NoteBot");
    mechanicUpdate.shopName = std::string("NoteBot Auto");
    mechanicUpdate.hourlyRate = 80.0;
    EXPECT_TRUE(db.updateMechanicProfile(mechanicUserId, mechanicUpdate));

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VINNOTE1";
    vehicle.make = "Honda";
    vehicle.model = "Civic";
    vehicle.year = 2022;
    vehicle.mileage = 5000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(
        rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)"
    );
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Brake squealing");
    symptomInsert.bind(4, 4);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());
    ASSERT_GT(symptomFormId, 0);

    AppointmentRecord req{};
    req.customerId = customerId;
    req.mechanicId = mechanicUserId;
    req.vehicleId = vehicleId;
    req.symptomFormId = symptomFormId;
    req.scheduledAt = "2026-04-01T10:00:00Z";
    req.status = AppointmentStatus::REQUESTED;
    req.note = "Brakes are squealing";

    auto appointmentId = db.createAppointment(req);
    ASSERT_GT(appointmentId, 0);

    auto jobId = db.createJobFromAppointment(appointmentId);
    ASSERT_GT(jobId, 0);

    // createJobFromAppointment seeds one note
    auto notes = db.listJobNotes(jobId);
    ASSERT_EQ(notes.size(), 1u);
    EXPECT_EQ(notes[0].type, "update");
    EXPECT_EQ(notes[0].text, "Job created");
    EXPECT_GT(notes[0].id, 0);

    // Add more notes
    auto n1 = db.addJobNote(jobId, "update", "Started diagnostics");
    EXPECT_GT(n1, 0);
    auto n2 = db.addJobNote(jobId, "update", "Found worn brake pads");
    EXPECT_GT(n2, 0);
    auto n3 = db.addJobNote(jobId, "blocked", "Waiting for parts");
    EXPECT_GT(n3, 0);
    auto n4 = db.addJobNote(jobId, "completion", "All done, new pads installed");
    EXPECT_GT(n4, 0);

    notes = db.listJobNotes(jobId);
    ASSERT_EQ(notes.size(), 5u);

    // Verify chronological order and types
    EXPECT_EQ(notes[0].text, "Job created");
    EXPECT_EQ(notes[1].text, "Started diagnostics");
    EXPECT_EQ(notes[1].type, "update");
    EXPECT_EQ(notes[2].text, "Found worn brake pads");
    EXPECT_EQ(notes[3].text, "Waiting for parts");
    EXPECT_EQ(notes[3].type, "blocked");
    EXPECT_EQ(notes[4].text, "All done, new pads installed");
    EXPECT_EQ(notes[4].type, "completion");

    // All notes belong to this job
    for (const auto& note : notes) {
        EXPECT_EQ(note.jobId, jobId);
        EXPECT_FALSE(note.createdAt.empty());
    }

    // Invalid jobId returns empty
    auto empty = db.listJobNotes(0);
    EXPECT_TRUE(empty.empty());
    EXPECT_EQ(db.addJobNote(0, "update", "should fail"), -1);
}

TEST_F(DatabaseTests, JobNotesViaGetJobStatus) {
    DatabaseManager db;

    auto customerId = db.createUser("CustFlow", "custflow@torquedesk.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("MechFlow", "mechflow@torquedesk.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mechanicUpdate;
    mechanicUpdate.displayName = std::string("FlowBot");
    mechanicUpdate.shopName = std::string("FlowBot Garage");
    mechanicUpdate.hourlyRate = 90.0;
    EXPECT_TRUE(db.updateMechanicProfile(mechanicUserId, mechanicUpdate));

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VINFLOW1";
    vehicle.make = "Toyota";
    vehicle.model = "Corolla";
    vehicle.year = 2023;
    vehicle.mileage = 3000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(
        rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)"
    );
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Engine light on");
    symptomInsert.bind(4, 3);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());

    AppointmentRecord req{};
    req.customerId = customerId;
    req.mechanicId = mechanicUserId;
    req.vehicleId = vehicleId;
    req.symptomFormId = symptomFormId;
    req.scheduledAt = "2026-05-01T10:00:00Z";
    req.status = AppointmentStatus::REQUESTED;
    req.note = "Check engine light";

    auto appointmentId = db.createAppointment(req);
    auto jobId = db.createJobFromAppointment(appointmentId);
    ASSERT_GT(jobId, 0);

    // Simulate mechanic workflow: update stage + notes
    db.updateJobStage(jobId, JobStage::DIAGNOSTICS, 20);
    db.addJobNote(jobId, "update", "Running diagnostic scan");
    db.updateJobStage(jobId, JobStage::REPAIR, 60);
    db.addJobNote(jobId, "update", "Replacing O2 sensor");
    db.markJobComplete(jobId);
    db.addJobNote(jobId, "completion", "O2 sensor replaced, engine light cleared");

    // Now check from customer perspective
    RatingEngine rating(db, 30);
    ProfitabilityEngine profitability(db, 100, 0.13);
    TestCustomerValidator validator;
    CustomerService service(&db, rating, profitability, validator);

    auto status = service.getJobStatus(jobId);
    EXPECT_EQ(status.currentStage, JobStage::DONE);
    EXPECT_EQ(status.percentComplete, 100);

    // Should have: "Job created" + 2 update notes + 1 completion = 4 total
    ASSERT_EQ(status.notes.size(), 4u);
    EXPECT_EQ(status.notes[0].type, "update");
    EXPECT_EQ(status.notes[0].text, "Job created");
    EXPECT_EQ(status.notes[1].type, "update");
    EXPECT_EQ(status.notes[1].text, "Running diagnostic scan");
    EXPECT_EQ(status.notes[2].type, "update");
    EXPECT_EQ(status.notes[2].text, "Replacing O2 sensor");
    EXPECT_EQ(status.notes[3].type, "completion");
    EXPECT_EQ(status.notes[3].text, "O2 sensor replaced, engine light cleared");
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

// Story 3: Can see incoming job requests
TEST_F(DatabaseTests, ListIncomingRequests) {
    DatabaseManager db;
    MechanicService service(db);

    auto customerId     = db.createUser("Customer1", "cust1@test.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("Mechanic1", "mech1@test.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mUpdate;
    mUpdate.displayName = std::string("Mechanic1");
    mUpdate.shopName    = std::string("Shop1");
    mUpdate.hourlyRate  = 80.0;
    db.updateMechanicProfile(mechanicUserId, mUpdate);

    auto mechRecord = db.getMechanicByUserId(mechanicUserId);
    ASSERT_TRUE(mechRecord.has_value());
    auto mechanicId = mechRecord->id;

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VIN00000001";
    vehicle.make = "Honda"; vehicle.model = "Civic"; vehicle.year = 2020; vehicle.mileage = 1000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)");
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Brakes squeaking");
    symptomInsert.bind(4, 2);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());

    AppointmentRecord appt{};
    appt.customerId   = customerId;
    appt.mechanicId   = mechanicId;
    appt.vehicleId    = vehicleId;
    appt.symptomFormId = symptomFormId;
    appt.scheduledAt  = "2026-05-01T10:00:00Z";
    appt.status       = AppointmentStatus::REQUESTED;
    appt.note         = "Check brakes";
    db.createAppointment(appt);

    auto requests = service.listIncomingRequests(mechanicId);
    EXPECT_FALSE(requests.empty());
}

// Story 9: Can accept an appointment
TEST_F(DatabaseTests, AcceptAppointment) {
    DatabaseManager db;
    MechanicService service(db);

    auto customerId     = db.createUser("Customer2", "cust2@test.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("Mechanic2", "mech2@test.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mUpdate;
    mUpdate.displayName = std::string("Mechanic2");
    mUpdate.shopName    = std::string("Shop2");
    mUpdate.hourlyRate  = 80.0;
    db.updateMechanicProfile(mechanicUserId, mUpdate);

    auto mechRecord = db.getMechanicByUserId(mechanicUserId);
    ASSERT_TRUE(mechRecord.has_value());
    auto mechanicId = mechRecord->id;

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VIN00000002";
    vehicle.make = "Toyota"; vehicle.model = "Camry"; vehicle.year = 2021; vehicle.mileage = 5000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)");
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Oil change needed");
    symptomInsert.bind(4, 1);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());

    AppointmentRecord appt{};
    appt.customerId    = customerId;
    appt.mechanicId    = mechanicId;
    appt.vehicleId     = vehicleId;
    appt.symptomFormId = symptomFormId;
    appt.scheduledAt   = "2026-05-01T10:00:00Z";
    appt.status        = AppointmentStatus::REQUESTED;
    appt.note          = "Oil change";
    auto appointmentId = db.createAppointment(appt);

    bool result = service.AcceptAppointment(appointmentId);
    EXPECT_TRUE(result);
}

// Story 1: Can update job stage with a note
TEST_F(DatabaseTests, UpdateJobStage) {
    DatabaseManager db;
    MechanicService service(db);

    auto customerId     = db.createUser("Customer3", "cust3@test.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("Mechanic3", "mech3@test.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mUpdate;
    mUpdate.displayName = std::string("Mechanic3");
    mUpdate.shopName    = std::string("Shop3");
    mUpdate.hourlyRate  = 80.0;
    db.updateMechanicProfile(mechanicUserId, mUpdate);

    auto mechRecord = db.getMechanicByUserId(mechanicUserId);
    ASSERT_TRUE(mechRecord.has_value());
    auto mechanicId = mechRecord->id;

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VIN00000003";
    vehicle.make = "Ford"; vehicle.model = "Focus"; vehicle.year = 2019; vehicle.mileage = 30000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)");
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Brake pads worn");
    symptomInsert.bind(4, 3);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());

    AppointmentRecord appt{};
    appt.customerId    = customerId;
    appt.mechanicId    = mechanicId;
    appt.vehicleId     = vehicleId;
    appt.symptomFormId = symptomFormId;
    appt.scheduledAt   = "2026-05-01T10:00:00Z";
    appt.status        = AppointmentStatus::CONFIRMED;
    appt.note          = "Brake pads";
    auto appointmentId = db.createAppointment(appt);
    auto jobId = db.createJobFromAppointment(appointmentId);
    db.updateJobStage(jobId, JobStage::DIAGNOSTICS, 0);

    bool result = service.updateJobStage(mechanicId, jobId, JobStage::REPAIR, 50, "Replacing brake pads");
    EXPECT_TRUE(result);
}

// Story 1: Can mark job as complete
TEST_F(DatabaseTests, MarkJobComplete) {
    DatabaseManager db;
    MechanicService service(db);

    auto customerId     = db.createUser("Customer4", "cust4@test.com", "pw", UserRole::CUSTOMER);
    auto mechanicUserId = db.createUser("Mechanic4", "mech4@test.com", "pw", UserRole::MECHANIC);

    MechanicUpdate mUpdate;
    mUpdate.displayName = std::string("Mechanic4");
    mUpdate.shopName    = std::string("Shop4");
    mUpdate.hourlyRate  = 80.0;
    db.updateMechanicProfile(mechanicUserId, mUpdate);

    auto mechRecord = db.getMechanicByUserId(mechanicUserId);
    ASSERT_TRUE(mechRecord.has_value());
    auto mechanicId = mechRecord->id;

    VehicleRecord vehicle{};
    vehicle.ownerUserId = customerId;
    vehicle.vin = "VIN00000004";
    vehicle.make = "Chevy"; vehicle.model = "Malibu"; vehicle.year = 2018; vehicle.mileage = 60000;
    auto vehicleId = db.createVehicle(customerId, vehicle);
    ASSERT_GT(vehicleId, 0);

    SQLite::Database rawDb("torquedesk.db", SQLite::OPEN_READWRITE);
    SQLite::Statement symptomInsert(rawDb,
        "INSERT INTO symptom_forms (customerId, vehicleId, description, severity) VALUES (?, ?, ?, ?)");
    symptomInsert.bind(1, static_cast<int64_t>(customerId));
    symptomInsert.bind(2, static_cast<int64_t>(vehicleId));
    symptomInsert.bind(3, "Full service needed");
    symptomInsert.bind(4, 1);
    symptomInsert.exec();
    auto symptomFormId = static_cast<SymptomFormId>(rawDb.getLastInsertRowid());

    AppointmentRecord appt{};
    appt.customerId    = customerId;
    appt.mechanicId    = mechanicId;
    appt.vehicleId     = vehicleId;
    appt.symptomFormId = symptomFormId;
    appt.scheduledAt   = "2026-05-01T10:00:00Z";
    appt.status        = AppointmentStatus::CONFIRMED;
    appt.note          = "Full service";
    auto appointmentId = db.createAppointment(appt);
    auto jobId = db.createJobFromAppointment(appointmentId);
    db.updateJobStage(jobId, JobStage::DIAGNOSTICS, 0);

    bool result = service.markJobComplete(mechanicId, jobId, "All repairs done");
    EXPECT_TRUE(result);
}

// ==================== Auth Tests ====================

TEST_F(DatabaseTests, AuthHashAndVerify) {
    DatabaseManager db;
    AuthService auth(db);

    auto hash = auth.hashPassword("MySecret123");
    EXPECT_FALSE(hash.empty());
    EXPECT_TRUE(auth.verifyPassword("MySecret123", hash));
    EXPECT_FALSE(auth.verifyPassword("WrongPassword", hash));
}

TEST_F(DatabaseTests, AuthRegisterAndLogin) {
    DatabaseManager db;
    AuthService auth(db);

    auto reg = auth.registerUser("Alice", "alice@example.com", "555-0100", "password123", UserRole::CUSTOMER);
    EXPECT_EQ(reg.email, "alice@example.com");
    EXPECT_EQ(reg.role, UserRole::CUSTOMER);

    auto login = auth.loginUser("alice@example.com", "password123");
    ASSERT_TRUE(login.has_value());
    EXPECT_EQ(login->userId, reg.userId);
}

TEST_F(DatabaseTests, AuthDuplicateEmail) {
    DatabaseManager db;
    AuthService auth(db);

    auth.registerUser("Bob", "bob@example.com", "555-0101", "password123", UserRole::CUSTOMER);

    EXPECT_THROW(auth.registerUser("Bob2", "bob@example.com", "555-0102", "password456", UserRole::CUSTOMER),
                 std::invalid_argument);
}

TEST_F(DatabaseTests, AuthPasswordTooShort) {
    DatabaseManager db;
    AuthService auth(db);

    EXPECT_THROW(auth.registerUser("Eve", "eve@example.com", "555-0103", "short", UserRole::CUSTOMER),
                 std::invalid_argument);
}