/**
 * @file test_customer_service.cpp
 * @brief Unit tests for CustomerService (GoogleTest).
 *
 * Covers: customer profiles, vehicles, symptom forms,
 *         appointments, and reviews.
 */
#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"
#include "../src/engine/CustomerService.h"
#include "../src/engine/RatingEngine.h"
#include "../src/engine/ProfitabilityEngine.h"
#include <cstdio>

/// @name Minimal stub implementations
/// @brief Required for linking — the real engine logic is not under test here.
/// @{

RatingEngine::RatingEngine(DatabaseManager &db, int recencyHalfLifeDays)
    : db(db), recencyHalfLifeDays(recencyHalfLifeDays) {}

int RatingEngine::compute(MechanicID) { return 0; }

double RatingEngine::getechanicScore(MechanicID) { return 0.0; }

ProfitabilityEngine::ProfitabilityEngine(DatabaseManager &db, int defaiultHourlyRate, double taxRate)
    : db(db), defaultHourlyRate(defaiultHourlyRate), taxRate(taxRate) {}

void ProfitabilityEngine::seTaxRate(double newTaxRate) { taxRate = newTaxRate; }

void ProfitabilityEngine::setDefaultHourlyRate(int newRate) { defaultHourlyRate = newRate; }

/// @}

/**
 * @class TestCustomerValidator
 * @brief Permissive validator for testing — always passes except
 *        validateVehicle, which requires a non-empty VIN and model.
 */
class TestCustomerValidator : public CustomerValidator
{
public:
  bool validateCustomerProfile(const CustomerCreate &) override { return true; }
  bool validateVehicle(const VehicleCreate &v) override
  {
    return !v.vin.empty() && !v.model.empty();
  }
  bool validateSymptomForm(const SymptomFormCreate &) override { return true; }
  bool validateReview(const ReviewCreate &) override { return true; }
  bool validateAppointment(const AppointmentCreate &) override { return true; }
};

/**
 * @class CustomerServiceTests
 * @brief GoogleTest fixture for CustomerService.
 *
 * Deletes the SQLite database file before each test so every case
 * starts with a clean, empty schema.  Provides convenience helpers
 * for creating the entities that most tests need.
 */
class CustomerServiceTests : public ::testing::Test
{
protected:
  /// @brief Remove the DB file so every test begins with a blank database.
  void SetUp() override
  {
    std::remove("torquedesk.db");
  }

  /// @name Factory helpers
  /// @{

  /**
   * @brief Insert a CUSTOMER user into the database.
   * @param db    Active DatabaseManager instance.
   * @param name  Display name (default "TestCustomer").
   * @param email Email address (default "cust@test.com").
   * @return The new user's UserId.
   */
  static UserId createTestCustomer(DatabaseManager &db,
                                   const std::string &name = "TestCustomer",
                                   const std::string &email = "cust@test.com")
  {
    return db.createUser(name, email, "pw", UserRole::CUSTOMER);
  }

  /**
   * @brief Insert a MECHANIC user and populate their mechanic profile.
   * @param db    Active DatabaseManager instance.
   * @param name  Display name (default "TestMechanic").
   * @param email Email address (default "mech@test.com").
   * @return The new user's UserId.
   */
  static UserId createTestMechanic(DatabaseManager &db,
                                   const std::string &name = "TestMechanic",
                                   const std::string &email = "mech@test.com")
  {
    auto uid = db.createUser(name, email, "pw", UserRole::MECHANIC);
    MechanicUpdate mu;
    mu.displayName = name;
    mu.shopName = std::string("TestShop");
    mu.hourlyRate = 100.0;
    mu.specialties = std::vector<std::string>{"BRAKES"};
    db.updateMechanicProfile(uid, mu);
    return uid;
  }

  /**
   * @brief Add a default vehicle via CustomerService.
   * @param svc    CustomerService instance.
   * @param custId Owner customer's UserId.
   * @return The new VehicleId.
   */
  static VehicleId createTestVehicle(CustomerService &svc, UserId custId)
  {
    VehicleCreate v;
    v.vin = "TESTVIN001";
    v.make = "Toyota";
    v.model = "Camry";
    v.year = 2022;
    v.mileage = 10000;
    return svc.addVehicle(custId, v);
  }

  /**
   * @brief Create a default symptom form via CustomerService.
   * @param svc    CustomerService instance.
   * @param custId Customer who owns the form.
   * @param vehId  Vehicle the form describes.
   * @return The new SymptomFormId.
   */
  static SymptomFormId createTestSymptomForm(CustomerService &svc,
                                             UserId custId,
                                             VehicleId vehId)
  {
    SymptomFormCreate f;
    f.customerId = custId;
    f.vehicleId = vehId;
    f.description = "Rattle under hood";
    f.severity = 3;
    return svc.createSymptomForm(f);
  }

  /// @brief Return type for createTestAppointmentAndJob.
  struct ApptJob
  {
    AppointmentId apptId;
    JobId jobId;
  };

  /**
   * @brief Create an appointment via CustomerService and immediately
   *        spawn a job from it through DatabaseManager.
   * @param db     Active DatabaseManager instance.
   * @param svc    CustomerService instance.
   * @param custId Customer requesting the appointment.
   * @param mechId Mechanic assigned to the appointment.
   * @param vehId  Vehicle for the appointment.
   * @param formId Symptom form linked to the appointment.
   * @return ApptJob containing both the AppointmentId and JobId.
   */
  static ApptJob createTestAppointmentAndJob(DatabaseManager &db,
                                             CustomerService &svc,
                                             UserId custId,
                                             UserId mechId,
                                             VehicleId vehId,
                                             SymptomFormId formId)
  {
    AppointmentCreate ac;
    ac.customerId = custId;
    ac.mechanicId = mechId;
    ac.vehicleId = vehId;
    ac.formId = formId;
    ac.scheduledAt = "2026-06-01T10:00:00Z";
    ac.note = "Test appointment";
    auto apptId = svc.requestAppointment(ac);
    auto jobId = db.createJobFromAppointment(apptId);
    return {apptId, jobId};
  }

  /// @}
};

// =====================================================================
/// @name 1. Customer Profile Tests
/// @{
// =====================================================================

/// @brief Verify that getCustomerProfile returns correct fields for an existing customer.
TEST_F(CustomerServiceTests, GetCustomerProfile_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto id = createTestCustomer(db, "Alice", "alice@test.com");
  auto dto = svc.getCustomerProfile(id);

  EXPECT_EQ(dto.userId, id);
  EXPECT_EQ(dto.fullName, "Alice");
  EXPECT_EQ(dto.email, "alice@test.com");
}

/// @brief Verify that getCustomerProfile throws invalid_argument for IDs <= 0.
TEST_F(CustomerServiceTests, GetCustomerProfile_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getCustomerProfile(0), std::invalid_argument);
  EXPECT_THROW(svc.getCustomerProfile(-1), std::invalid_argument);
}

/// @brief Verify that getCustomerProfile throws runtime_error for a non-existent user.
TEST_F(CustomerServiceTests, GetCustomerProfile_NotFound)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getCustomerProfile(9999), std::runtime_error);
}

/// @brief Verify that updateCustomerProfile persists name and phone changes.
TEST_F(CustomerServiceTests, UpdateCustomerProfile_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto id = createTestCustomer(db, "Bob", "bob@test.com");

  CustomerProfileUpdate upd;
  upd.fullName = std::string("Robert");
  upd.phone = std::string("555-1234");
  EXPECT_TRUE(svc.updateCustomerProfile(id, upd));

  auto dto = svc.getCustomerProfile(id);
  EXPECT_EQ(dto.fullName, "Robert");
  EXPECT_EQ(dto.phone, "555-1234");
}

/// @brief Verify that updateCustomerProfile throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, UpdateCustomerProfile_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  CustomerProfileUpdate upd;
  upd.fullName = std::string("X");
  EXPECT_THROW(svc.updateCustomerProfile(0, upd), std::invalid_argument);
}

/// @brief Verify that deleteCustomerProfile removes the user so subsequent get throws.
TEST_F(CustomerServiceTests, DeleteCustomerProfile_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto id = createTestCustomer(db);
  EXPECT_TRUE(svc.deleteCustomerProfile(id));
  EXPECT_THROW(svc.getCustomerProfile(id), std::runtime_error);
}

/// @brief Verify that deleteCustomerProfile throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, DeleteCustomerProfile_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.deleteCustomerProfile(0), std::invalid_argument);
}

/// @brief Verify that updateCustomerPasswordHash succeeds for an existing customer.
TEST_F(CustomerServiceTests, UpdateCustomerPasswordHash_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto id = createTestCustomer(db);
  EXPECT_TRUE(svc.updateCustomerPasswordHash(id, "newhash123"));
}

/// @brief Verify that updateCustomerPasswordHash throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, UpdateCustomerPasswordHash_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.updateCustomerPasswordHash(0, "h"), std::invalid_argument);
}

/// @}

// =====================================================================
/// @name 2. Vehicle Tests
/// @{
// =====================================================================

/// @brief Verify that addVehicle returns a positive VehicleId on success.
TEST_F(CustomerServiceTests, AddVehicle_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  VehicleCreate vc;
  vc.vin = "VIN100";
  vc.make = "Honda";
  vc.model = "Civic";
  vc.year = 2021;
  vc.mileage = 20000;

  auto vid = svc.addVehicle(cid, vc);
  EXPECT_GT(vid, 0);
}

/// @brief Verify that addVehicle throws invalid_argument when VIN and model are empty.
TEST_F(CustomerServiceTests, AddVehicle_ValidationFail)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  VehicleCreate vc;
  vc.vin = "";
  vc.make = "Honda";
  vc.model = "";
  EXPECT_THROW(svc.addVehicle(cid, vc), std::invalid_argument);
}

/// @brief Verify that addVehicle throws invalid_argument for customer ID 0.
TEST_F(CustomerServiceTests, AddVehicle_InvalidCustomerId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  VehicleCreate vc;
  vc.vin = "VIN";
  vc.make = "X";
  vc.model = "Y";
  EXPECT_THROW(svc.addVehicle(0, vc), std::invalid_argument);
}

/// @brief Verify that getVehicle returns all fields correctly for an existing vehicle.
TEST_F(CustomerServiceTests, GetVehicle_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);

  auto dto = svc.getVehicle(vid);
  EXPECT_EQ(dto.vehicleId, vid);
  EXPECT_EQ(dto.vin, "TESTVIN001");
  EXPECT_EQ(dto.make, "Toyota");
  EXPECT_EQ(dto.model, "Camry");
  EXPECT_EQ(dto.year, 2022);
  EXPECT_EQ(dto.mileage, 10000);
}

/// @brief Verify that getVehicle throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, GetVehicle_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getVehicle(0), std::invalid_argument);
}

/// @brief Verify that getVehicle throws runtime_error for a non-existent vehicle.
TEST_F(CustomerServiceTests, GetVehicle_NotFound)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getVehicle(9999), std::runtime_error);
}

/// @brief Verify that listVehicles returns all vehicles belonging to a customer.
TEST_F(CustomerServiceTests, ListVehicles_Multiple)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  createTestVehicle(svc, cid);

  VehicleCreate v2;
  v2.vin = "VIN200";
  v2.make = "Ford";
  v2.model = "Focus";
  v2.year = 2019;
  v2.mileage = 40000;
  svc.addVehicle(cid, v2);

  auto list = svc.listVehicles(cid);
  EXPECT_EQ(list.size(), 2u);
}

/// @brief Verify that listVehicles returns an empty vector when no vehicles exist.
TEST_F(CustomerServiceTests, ListVehicles_Empty)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto list = svc.listVehicles(cid);
  EXPECT_TRUE(list.empty());
}

/// @brief Verify that listVehicles throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, ListVehicles_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.listVehicles(0), std::invalid_argument);
}

/// @brief Verify that updateVehicle persists make, model, and year changes.
TEST_F(CustomerServiceTests, UpdateVehicle_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);

  VehicleUpdate upd;
  upd.customerId = cid;
  upd.make = std::string("Lexus");
  upd.model = std::string("IS");
  upd.year = 2023;

  EXPECT_TRUE(svc.updateVehicle(vid, upd));

  auto dto = svc.getVehicle(vid);
  EXPECT_EQ(dto.make, "Lexus");
  EXPECT_EQ(dto.model, "IS");
  EXPECT_EQ(dto.year, 2023);
}

/// @brief Verify that updateVehicle returns false for vehicle ID 0.
TEST_F(CustomerServiceTests, UpdateVehicle_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  VehicleUpdate upd;
  upd.make = std::string("X");
  EXPECT_FALSE(svc.updateVehicle(0, upd));
}

/// @brief Verify that removeVehicle deletes the vehicle so subsequent get throws.
TEST_F(CustomerServiceTests, RemoveVehicle_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);

  EXPECT_TRUE(svc.removeVehicle(vid));
  EXPECT_THROW(svc.getVehicle(vid), std::runtime_error);
}

/// @brief Verify that removeVehicle returns false for vehicle ID 0.
TEST_F(CustomerServiceTests, RemoveVehicle_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_FALSE(svc.removeVehicle(0));
}

/// @}

// =====================================================================
/// @name 3. Symptom Form Tests
/// @{
// =====================================================================

/// @brief Verify that createSymptomForm returns a positive form ID.
TEST_F(CustomerServiceTests, CreateSymptomForm_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);
  EXPECT_GT(fid, 0);
}

/// @brief Verify that createSymptomForm throws invalid_argument when customerId is 0.
TEST_F(CustomerServiceTests, CreateSymptomForm_InvalidCustomerId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  SymptomFormCreate f;
  f.customerId = 0;
  f.vehicleId = 1;
  f.description = "X";
  f.severity = 2;
  EXPECT_THROW(svc.createSymptomForm(f), std::invalid_argument);
}

/// @brief Verify that createSymptomForm throws invalid_argument when vehicleId is 0.
TEST_F(CustomerServiceTests, CreateSymptomForm_InvalidVehicleId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  SymptomFormCreate f;
  f.customerId = 1;
  f.vehicleId = 0;
  f.description = "X";
  f.severity = 2;
  EXPECT_THROW(svc.createSymptomForm(f), std::invalid_argument);
}

/// @brief Verify that getSymptomForm returns all stored fields correctly.
TEST_F(CustomerServiceTests, GetSymptomForm_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  auto dto = svc.getSymptomForm(fid);
  EXPECT_EQ(dto.formId, fid);
  EXPECT_EQ(dto.customerId, cid);
  EXPECT_EQ(dto.vehicleId, vid);
  EXPECT_EQ(dto.description, "Rattle under hood");
  EXPECT_EQ(dto.severity, 3);
}

/// @brief Verify that getSymptomForm throws invalid_argument for form ID 0.
TEST_F(CustomerServiceTests, GetSymptomForm_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getSymptomForm(0), std::invalid_argument);
}

/// @brief Verify that listSymptomForms returns all forms belonging to a customer.
TEST_F(CustomerServiceTests, ListSymptomForms_Multiple)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);
  createTestSymptomForm(svc, cid, vid);

  SymptomFormCreate f2;
  f2.customerId = cid;
  f2.vehicleId = vid;
  f2.description = "Steering vibration";
  f2.severity = 2;
  svc.createSymptomForm(f2);

  auto list = svc.listSymptomForms(cid);
  EXPECT_EQ(list.size(), 2u);
}

/// @brief Verify that listSymptomForms returns an empty vector when none exist.
TEST_F(CustomerServiceTests, ListSymptomForms_Empty)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto list = svc.listSymptomForms(cid);
  EXPECT_TRUE(list.empty());
}

/// @brief Verify that listSymptomForms throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, ListSymptomForms_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.listSymptomForms(0), std::invalid_argument);
}

/// @brief Verify that updateSymptomForm persists description and severity changes.
TEST_F(CustomerServiceTests, UpdateSymptomForm_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  SymptomFormUpdate upd;
  upd.description = std::string("Updated description");
  upd.severity = 5;
  EXPECT_TRUE(svc.updateSymptomForm(fid, upd));

  auto dto = svc.getSymptomForm(fid);
  EXPECT_EQ(dto.description, "Updated description");
  EXPECT_EQ(dto.severity, 5);
}

/// @brief Verify that updateSymptomForm throws invalid_argument for form ID 0.
TEST_F(CustomerServiceTests, UpdateSymptomForm_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  SymptomFormUpdate upd;
  upd.description = std::string("X");
  EXPECT_THROW(svc.updateSymptomForm(0, upd), std::invalid_argument);
}

/// @brief Verify that deleteSymptomForm removes the form so listSymptomForms is empty.
TEST_F(CustomerServiceTests, DeleteSymptomForm_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  EXPECT_TRUE(svc.deleteSymptomForm(fid));

  auto list = svc.listSymptomForms(cid);
  EXPECT_TRUE(list.empty());
}

/// @brief Verify that deleteSymptomForm throws invalid_argument for form ID 0.
TEST_F(CustomerServiceTests, DeleteSymptomForm_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.deleteSymptomForm(0), std::invalid_argument);
}

// =====================================================================
/// @name 4. Appointment Tests
/// @{
// =====================================================================

/// @brief Verify that requestAppointment returns a positive AppointmentId.
TEST_F(CustomerServiceTests, RequestAppointment_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto mid = createTestMechanic(db);
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  AppointmentCreate ac;
  ac.customerId = cid;
  ac.mechanicId = mid;
  ac.vehicleId = vid;
  ac.formId = fid;
  ac.scheduledAt = "2026-07-01T09:00:00Z";
  ac.note = "Oil change";

  auto aid = svc.requestAppointment(ac);
  EXPECT_GT(aid, 0);
}

/// @brief Verify that getAppointment returns correct fields including REQUESTED status.
TEST_F(CustomerServiceTests, GetAppointment_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db, "Cust", "c@t.com");
  auto mid = createTestMechanic(db, "Mech", "m@t.com");
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  AppointmentCreate ac;
  ac.customerId = cid;
  ac.mechanicId = mid;
  ac.vehicleId = vid;
  ac.formId = fid;
  ac.scheduledAt = "2026-07-01T09:00:00Z";
  ac.note = "Brakes";
  auto aid = svc.requestAppointment(ac);

  auto dto = svc.getAppointment(aid);
  EXPECT_EQ(dto.appointmentId, aid);
  EXPECT_EQ(dto.customerId, cid);
  EXPECT_EQ(dto.mechanicId, mid);
  EXPECT_EQ(dto.note, "Brakes");
  EXPECT_EQ(dto.status, AppointmentStatus::REQUESTED);
}

/// @brief Verify that getAppointment throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, GetAppointment_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getAppointment(0), std::invalid_argument);
}

/// @brief Verify that getAppointment throws runtime_error for a non-existent appointment.
TEST_F(CustomerServiceTests, GetAppointment_NotFound)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.getAppointment(9999), std::runtime_error);
}

/// @brief Verify that listAppointments returns all appointments for a customer.
TEST_F(CustomerServiceTests, ListAppointments_Multiple)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db, "C", "c@t.com");
  auto mid = createTestMechanic(db, "M", "m@t.com");
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  AppointmentCreate ac1;
  ac1.customerId = cid;
  ac1.mechanicId = mid;
  ac1.vehicleId = vid;
  ac1.formId = fid;
  ac1.scheduledAt = "2026-07-01T09:00:00Z";
  ac1.note = "A";
  svc.requestAppointment(ac1);

  AppointmentCreate ac2;
  ac2.customerId = cid;
  ac2.mechanicId = mid;
  ac2.vehicleId = vid;
  ac2.formId = fid;
  ac2.scheduledAt = "2026-07-02T09:00:00Z";
  ac2.note = "B";
  svc.requestAppointment(ac2);

  auto list = svc.listAppointments(cid);
  EXPECT_EQ(list.size(), 2u);
}

/// @brief Verify that listAppointments returns an empty vector when none exist.
TEST_F(CustomerServiceTests, ListAppointments_Empty)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto list = svc.listAppointments(cid);
  EXPECT_TRUE(list.empty());
}

/// @brief Verify that updateAppointmentStatus changes status from REQUESTED to CONFIRMED.
TEST_F(CustomerServiceTests, UpdateAppointmentStatus_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db, "C2", "c2@t.com");
  auto mid = createTestMechanic(db, "M2", "m2@t.com");
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);

  AppointmentCreate ac;
  ac.customerId = cid;
  ac.mechanicId = mid;
  ac.vehicleId = vid;
  ac.formId = fid;
  ac.scheduledAt = "2026-07-01T09:00:00Z";
  ac.note = "N";
  auto aid = svc.requestAppointment(ac);

  EXPECT_TRUE(svc.updateAppointmentStatus(aid, AppointmentStatus::CONFIRMED));

  auto dto = svc.getAppointment(aid);
  EXPECT_EQ(dto.status, AppointmentStatus::CONFIRMED);
}

/// @brief Verify that updateAppointmentStatus throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, UpdateAppointmentStatus_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.updateAppointmentStatus(0, AppointmentStatus::CONFIRMED),
               std::invalid_argument);
}

/// @}

// =====================================================================
/// @name 5. Review Tests
/// @{
// =====================================================================

/// @brief Verify that submitReview returns a positive ReviewId after full job setup.
TEST_F(CustomerServiceTests, SubmitReview_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db, "RC", "rc@t.com");
  auto mid = createTestMechanic(db, "RM", "rm@t.com");
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);
  auto [apptId, jobId] = createTestAppointmentAndJob(db, svc, cid, mid, vid, fid);

  ReviewCreate rc;
  rc.customerId = cid;
  rc.mechanicId = mid;
  rc.jobId = jobId;
  rc.rating = 5;
  rc.comment = "Great work!";

  auto rid = svc.submitReview(rc);
  EXPECT_GT(rid, 0);
}

/// @brief Verify that listMyReviews returns a review with the correct rating and comment.
TEST_F(CustomerServiceTests, ListMyReviews_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db, "LR", "lr@t.com");
  auto mid = createTestMechanic(db, "LM", "lm@t.com");
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);
  auto [apptId, jobId] = createTestAppointmentAndJob(db, svc, cid, mid, vid, fid);

  ReviewCreate rc;
  rc.customerId = cid;
  rc.mechanicId = mid;
  rc.jobId = jobId;
  rc.rating = 4;
  rc.comment = "Good";
  svc.submitReview(rc);

  auto list = svc.listMyReviews(cid);
  ASSERT_EQ(list.size(), 1u);
  EXPECT_EQ(list[0].rating, 4);
  EXPECT_EQ(list[0].comment, "Good");
  EXPECT_EQ(list[0].customerId, cid);
}

/// @brief Verify that listMyReviews returns an empty vector when no reviews exist.
TEST_F(CustomerServiceTests, ListMyReviews_Empty)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db);
  auto list = svc.listMyReviews(cid);
  EXPECT_TRUE(list.empty());
}

/// @brief Verify that listMyReviews throws invalid_argument for ID 0.
TEST_F(CustomerServiceTests, ListMyReviews_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.listMyReviews(0), std::invalid_argument);
}

/// @brief Verify that deleteMyReview removes the review so listMyReviews is empty.
TEST_F(CustomerServiceTests, DeleteMyReview_HappyPath)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  auto cid = createTestCustomer(db, "DR", "dr@t.com");
  auto mid = createTestMechanic(db, "DM", "dm@t.com");
  auto vid = createTestVehicle(svc, cid);
  auto fid = createTestSymptomForm(svc, cid, vid);
  auto [apptId, jobId] = createTestAppointmentAndJob(db, svc, cid, mid, vid, fid);

  ReviewCreate rc;
  rc.customerId = cid;
  rc.mechanicId = mid;
  rc.jobId = jobId;
  rc.rating = 3;
  rc.comment = "OK";
  auto rid = svc.submitReview(rc);

  EXPECT_TRUE(svc.deleteMyReview(rid));
  auto list = svc.listMyReviews(cid);
  EXPECT_TRUE(list.empty());
}

/// @brief Verify that deleteMyReview throws invalid_argument for ID -1.
TEST_F(CustomerServiceTests, DeleteMyReview_InvalidId)
{
  DatabaseManager db;
  RatingEngine r(db, 30);
  ProfitabilityEngine p(db, 100, 0.13);
  TestCustomerValidator v;
  CustomerService svc(&db, r, p, v);

  EXPECT_THROW(svc.deleteMyReview(-1), std::invalid_argument);
}

/// @}
