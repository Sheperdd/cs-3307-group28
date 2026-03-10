/**
 * @file test_mechanic_service.cpp
 * @brief Unit tests for MechanicService (GoogleTest).
 *
 * Covers: mechanic profiles, job lifecycle, job notes,
 *         search, incoming requests, and reviews.
 */
#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"
#include "../src/engine/MechanicService.h"
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
 * @class MechanicServiceTests
 * @brief GoogleTest fixture for MechanicService.
 *
 * Deletes the SQLite database file before each test so every case
 * starts with a clean, empty schema.  Provides convenience helpers
 * for creating the entities that most tests need.
 */
class MechanicServiceTests : public ::testing::Test
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
   * @brief Insert a MECHANIC user into the database (no profile yet).
   * @param db    Active DatabaseManager instance.
   * @param name  Display name (default "TestMechanic").
   * @param email Email address (default "mech@test.com").
   * @return The new user's UserId.
   */
  static UserId createTestMechanicUser(DatabaseManager &db,
                                       const std::string &name = "TestMechanic",
                                       const std::string &email = "mech@test.com")
  {
    return db.createUser(name, email, "pw", UserRole::MECHANIC);
  }

  /**
   * @brief Insert a MECHANIC user and create their profile via the service.
   * @param db    Active DatabaseManager instance.
   * @param svc   MechanicService instance.
   * @param name  Display name (default "TestMechanic").
   * @param email Email address (default "mech@test.com").
   * @return The mechanic's UserId (same as MechanicId).
   */
  static UserId createTestMechanicWithProfile(DatabaseManager &db,
                                              MechanicService &svc,
                                              const std::string &name = "TestMechanic",
                                              const std::string &email = "mech@test.com")
  {
    auto uid = createTestMechanicUser(db, name, email);
    MechanicCreate mc;
    mc.displayName = name;
    mc.shopName = "TestShop";
    mc.hourlyRate = 100.0;
    mc.specialties = {"BRAKES", "ENGINE"};
    svc.createMechanicProfile(uid, mc);
    return uid;
  }

  /**
   * @brief Add a default vehicle directly via the database.
   * @param db     Active DatabaseManager instance.
   * @param custId Owner customer's UserId.
   * @return The new VehicleId.
   */
  static VehicleId createTestVehicle(DatabaseManager &db, UserId custId)
  {
    VehicleRecord v;
    v.ownerUserId = custId;
    v.vin = "TESTVIN001";
    v.make = "Toyota";
    v.model = "Camry";
    v.year = 2022;
    v.mileage = 10000;
    return db.createVehicle(custId, v);
  }

  /**
   * @brief Create a default symptom form directly via the database.
   * @param db     Active DatabaseManager instance.
   * @param custId Customer who owns the form.
   * @param vehId  Vehicle the form describes.
   * @return The new SymptomFormId.
   */
  static SymptomFormId createTestSymptomForm(DatabaseManager &db,
                                             UserId custId,
                                             VehicleId vehId)
  {
    SymptomFormRecord f;
    f.customerId = custId;
    f.vehicleId = vehId;
    f.description = "Rattle under hood";
    f.severity = 3;
    return db.createSymptomForm(f);
  }

  /**
   * @brief Create a REQUESTED appointment directly via the database.
   * @param db     Active DatabaseManager instance.
   * @param custId Customer requesting the appointment.
   * @param mechId Mechanic assigned to the appointment.
   * @param vehId  Vehicle for the appointment.
   * @param formId Symptom form linked to the appointment.
   * @return The new AppointmentId.
   */
  static AppointmentId createTestAppointment(DatabaseManager &db,
                                             UserId custId,
                                             UserId mechId,
                                             VehicleId vehId,
                                             SymptomFormId formId)
  {
    AppointmentRecord ar;
    ar.customerId = custId;
    ar.mechanicId = mechId;
    ar.vehicleId = vehId;
    ar.symptomFormId = formId;
    ar.scheduledAt = "2026-06-01T10:00:00Z";
    ar.note = "Test appointment";
    ar.status = AppointmentStatus::REQUESTED;
    return db.createAppointment(ar);
  }

  /**
   * @brief Create an appointment and set its status to CONFIRMED.
   * @param db     Active DatabaseManager instance.
   * @param custId Customer requesting the appointment.
   * @param mechId Mechanic assigned to the appointment.
   * @param vehId  Vehicle for the appointment.
   * @param formId Symptom form linked to the appointment.
   * @return The new AppointmentId (now CONFIRMED).
   */
  static AppointmentId createConfirmedAppointment(DatabaseManager &db,
                                                  UserId custId,
                                                  UserId mechId,
                                                  VehicleId vehId,
                                                  SymptomFormId formId)
  {
    auto apptId = createTestAppointment(db, custId, mechId, vehId, formId);
    db.updateAppointmentStatus(apptId, AppointmentStatus::CONFIRMED);
    return apptId;
  }

  /// @brief Return type for createFullJobSetup.
  struct FullSetup
  {
    UserId custId;
    UserId mechId;
    VehicleId vehId;
    SymptomFormId formId;
    AppointmentId apptId;
    JobId jobId;
  };

  /**
   * @brief Create all prerequisite entities and start a job.
   *
   * Creates a customer, mechanic with profile, vehicle, symptom form,
   * confirmed appointment, and starts a job from it.
   *
   * @param db  Active DatabaseManager instance.
   * @param svc MechanicService instance.
   * @return FullSetup containing all created entity IDs.
   */
  static FullSetup createFullJobSetup(DatabaseManager &db, MechanicService &svc)
  {
    auto cid = createTestCustomer(db, "JobCust", "jobcust@test.com");
    auto mid = createTestMechanicWithProfile(db, svc, "JobMech", "jobmech@test.com");
    auto vid = createTestVehicle(db, cid);
    auto fid = createTestSymptomForm(db, cid, vid);
    auto apptId = createConfirmedAppointment(db, cid, mid, vid, fid);
    auto jobId = svc.startJobFromAppointment(mid, apptId);
    return {cid, mid, vid, fid, apptId, jobId};
  }

  /// @}
};

// =====================================================================
/// @name 1. createMechanicProfile Tests
/// @{
// =====================================================================

/// @brief Verify that createMechanicProfile returns a valid MechanicId on success.
TEST_F(MechanicServiceTests, CreateMechanicProfile_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto uid = createTestMechanicUser(db, "Alice", "alice@test.com");

  MechanicCreate mc;
  mc.displayName = "Alice";
  mc.shopName = "Alice's Garage";
  mc.hourlyRate = 80.0;
  mc.specialties = {"BRAKES"};

  auto mechId = svc.createMechanicProfile(uid, mc);
  EXPECT_GT(mechId, 0);
}

/// @brief Verify that createMechanicProfile throws runtime_error for non-mechanic user.
TEST_F(MechanicServiceTests, CreateMechanicProfile_NonMechanicUser)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto uid = createTestCustomer(db, "Bob", "bob@test.com");

  MechanicCreate mc;
  mc.displayName = "Bob";
  mc.shopName = "Shop";
  mc.hourlyRate = 50.0;
  mc.specialties = {"ENGINE"};

  EXPECT_THROW(svc.createMechanicProfile(uid, mc), std::runtime_error);
}

/// @brief Verify that createMechanicProfile throws runtime_error for duplicate profile.
TEST_F(MechanicServiceTests, CreateMechanicProfile_DuplicateProfile)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto uid = createTestMechanicUser(db, "Carol", "carol@test.com");

  MechanicCreate mc;
  mc.displayName = "Carol";
  mc.shopName = "Carol's Shop";
  mc.hourlyRate = 90.0;
  mc.specialties = {"BRAKES"};

  svc.createMechanicProfile(uid, mc);
  EXPECT_THROW(svc.createMechanicProfile(uid, mc), std::runtime_error);
}

/// @}

// =====================================================================
/// @name 2. getMechanicProfile Tests
/// @{
// =====================================================================

/// @brief Verify that getMechanicProfile returns correct fields for an existing mechanic.
TEST_F(MechanicServiceTests, GetMechanicProfile_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto mid = createTestMechanicWithProfile(db, svc, "Dave", "dave@test.com");

  auto dto = svc.getMechanicProfile(mid);
  EXPECT_EQ(dto.userId, mid);
  EXPECT_EQ(dto.displayName, "Dave");
  EXPECT_DOUBLE_EQ(dto.hourlyRate, 100.0);
  EXPECT_EQ(dto.reviewCount, 0);
  EXPECT_DOUBLE_EQ(dto.averageRating, 0.0);
}

/// @brief Verify that getMechanicProfile throws runtime_error for non-existent mechanic.
TEST_F(MechanicServiceTests, GetMechanicProfile_NotFound)
{
  DatabaseManager db;
  MechanicService svc(db);

  EXPECT_THROW(svc.getMechanicProfile(9999), std::runtime_error);
}

/// @brief Verify that getMechanicProfile computes averageRating from reviews.
TEST_F(MechanicServiceTests, GetMechanicProfile_WithReviews)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "Rev1", "rev1@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "Eve", "eve@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);

  // Each review needs its own job (UNIQUE constraint on reviews.jobId)
  auto apptId1 = createConfirmedAppointment(db, cid, mid, vid, fid);
  auto jobId1 = svc.startJobFromAppointment(mid, apptId1);

  auto apptId2 = createConfirmedAppointment(db, cid, mid, vid, fid);
  auto jobId2 = svc.startJobFromAppointment(mid, apptId2);

  ReviewRecord r1;
  r1.customerId = cid;
  r1.mechanicId = mid;
  r1.jobId = jobId1;
  r1.rating = 4;
  r1.comment = "Good";
  db.createReview(r1);

  ReviewRecord r2;
  r2.customerId = cid;
  r2.mechanicId = mid;
  r2.jobId = jobId2;
  r2.rating = 2;
  r2.comment = "Meh";
  db.createReview(r2);

  auto dto = svc.getMechanicProfile(mid);
  EXPECT_EQ(dto.reviewCount, 2);
  EXPECT_DOUBLE_EQ(dto.averageRating, 3.0);
}

/// @}

// =====================================================================
/// @name 3. updateMechanicProfile Tests
/// @{
// =====================================================================

/// @brief Verify that updateMechanicProfile persists displayName and hourlyRate changes.
TEST_F(MechanicServiceTests, UpdateMechanicProfile_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto mid = createTestMechanicWithProfile(db, svc, "Frank", "frank@test.com");

  MechanicUpdateDTO upd;
  upd.displayName = std::string("Franklin");
  upd.hourlyRate = 120.0;
  EXPECT_TRUE(svc.updateMechanicProfile(mid, upd));

  auto dto = svc.getMechanicProfile(mid);
  EXPECT_EQ(dto.displayName, "Franklin");
  EXPECT_DOUBLE_EQ(dto.hourlyRate, 120.0);
}

/// @brief Verify that updateMechanicProfile throws runtime_error for non-existent mechanic.
TEST_F(MechanicServiceTests, UpdateMechanicProfile_NotFound)
{
  DatabaseManager db;
  MechanicService svc(db);

  MechanicUpdateDTO upd;
  upd.displayName = std::string("Nobody");
  EXPECT_THROW(svc.updateMechanicProfile(9999, upd), std::runtime_error);
}

/// @brief Verify that partial update only changes the specified field.
TEST_F(MechanicServiceTests, UpdateMechanicProfile_PartialUpdate)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto mid = createTestMechanicWithProfile(db, svc, "Grace", "grace@test.com");

  MechanicUpdateDTO upd;
  upd.shopName = std::string("Grace's New Shop");
  EXPECT_TRUE(svc.updateMechanicProfile(mid, upd));

  auto dto = svc.getMechanicProfile(mid);
  EXPECT_EQ(dto.displayName, "Grace");
  EXPECT_DOUBLE_EQ(dto.hourlyRate, 100.0);
}

/// @}

// =====================================================================
/// @name 4. searchMechanics Tests
/// @{
// =====================================================================

/// @brief Verify that searchMechanics returns all mechanics when no filter is set.
TEST_F(MechanicServiceTests, SearchMechanics_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  createTestMechanicWithProfile(db, svc, "M1", "m1@test.com");
  createTestMechanicWithProfile(db, svc, "M2", "m2@test.com");

  MechanicSearchFilter filter;
  auto results = svc.searchMechanics(filter);
  EXPECT_GE(results.size(), 2u);
}

/// @brief Verify that searchMechanics returns empty when no mechanics exist.
TEST_F(MechanicServiceTests, SearchMechanics_Empty)
{
  DatabaseManager db;
  MechanicService svc(db);

  MechanicSearchFilter filter;
  auto results = svc.searchMechanics(filter);
  EXPECT_TRUE(results.empty());
}

/// @brief Verify that searchMechanics filters by specialty.
TEST_F(MechanicServiceTests, SearchMechanics_BySpecialty)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto uid1 = createTestMechanicUser(db, "Brake1", "brake1@test.com");
  MechanicCreate mc1;
  mc1.displayName = "Brake1";
  mc1.shopName = "Shop1";
  mc1.hourlyRate = 80.0;
  mc1.specialties = {"BRAKES"};
  svc.createMechanicProfile(uid1, mc1);

  auto uid2 = createTestMechanicUser(db, "Engine1", "engine1@test.com");
  MechanicCreate mc2;
  mc2.displayName = "Engine1";
  mc2.shopName = "Shop2";
  mc2.hourlyRate = 90.0;
  mc2.specialties = {"ENGINE"};
  svc.createMechanicProfile(uid2, mc2);

  MechanicSearchFilter filter;
  filter.specialty = std::string("BRAKES");
  auto results = svc.searchMechanics(filter);
  EXPECT_GE(results.size(), 1u);
  for (const auto &r : results)
  {
    bool hasBrakes = false;
    for (const auto &s : r.specialties)
    {
      if (s == "BRAKES")
        hasBrakes = true;
    }
    EXPECT_TRUE(hasBrakes);
  }
}

/// @}

// =====================================================================
/// @name 5. startJobFromAppointment Tests
/// @{
// =====================================================================

/// @brief Verify that startJobFromAppointment returns a positive JobId on success.
TEST_F(MechanicServiceTests, StartJobFromAppointment_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "SJ_C", "sjc@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "SJ_M", "sjm@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);
  auto apptId = createConfirmedAppointment(db, cid, mid, vid, fid);

  auto jobId = svc.startJobFromAppointment(mid, apptId);
  EXPECT_GT(jobId, 0);
}

/// @brief Verify that startJobFromAppointment throws for non-CONFIRMED appointment.
TEST_F(MechanicServiceTests, StartJobFromAppointment_WrongStatus)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "WS_C", "wsc@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "WS_M", "wsm@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);
  // Appointment stays REQUESTED (not CONFIRMED)
  auto apptId = createTestAppointment(db, cid, mid, vid, fid);

  EXPECT_THROW(svc.startJobFromAppointment(mid, apptId), std::runtime_error);
}

/// @brief Verify that startJobFromAppointment throws when mechanic doesn't own appointment.
TEST_F(MechanicServiceTests, StartJobFromAppointment_WrongMechanic)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "WM_C", "wmc@test.com");
  auto mid1 = createTestMechanicWithProfile(db, svc, "WM_M1", "wmm1@test.com");
  auto mid2 = createTestMechanicWithProfile(db, svc, "WM_M2", "wmm2@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);
  auto apptId = createConfirmedAppointment(db, cid, mid1, vid, fid);

  EXPECT_THROW(svc.startJobFromAppointment(mid2, apptId), std::runtime_error);
}

/// @}

// =====================================================================
/// @name 6. getJob Tests
/// @{
// =====================================================================

/// @brief Verify that getJob returns correct fields for an existing job.
TEST_F(MechanicServiceTests, GetJob_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  auto dto = svc.getJob(setup.jobId);
  EXPECT_EQ(dto.jobId, setup.jobId);
  EXPECT_EQ(dto.mechanicId, setup.mechId);
  EXPECT_EQ(dto.customerId, setup.custId);
  EXPECT_EQ(dto.appointmentId, setup.apptId);
}

/// @brief Verify that getJob throws runtime_error for a non-existent job.
TEST_F(MechanicServiceTests, GetJob_NotFound)
{
  DatabaseManager db;
  MechanicService svc(db);

  EXPECT_THROW(svc.getJob(9999), std::runtime_error);
}

/// @brief Verify that getJob includes notes in the returned DTO.
TEST_F(MechanicServiceTests, GetJob_WithNotes)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);
  svc.addJobNote(setup.mechId, setup.jobId, "First note");
  svc.addJobNote(setup.mechId, setup.jobId, "Second note");

  auto dto = svc.getJob(setup.jobId);
  // startJobFromAppointment adds an initial "Job started" note,
  // plus our 2 manual notes
  EXPECT_GE(dto.notes.size(), 3u);
}

/// @}

// =====================================================================
/// @name 7. updateJobStage Tests
/// @{
// =====================================================================

/// @brief Verify that updateJobStage changes stage and percentComplete.
TEST_F(MechanicServiceTests, UpdateJobStage_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  EXPECT_TRUE(svc.updateJobStage(setup.mechId, setup.jobId,
                                 JobStage::REPAIR, 50, "Half done"));

  auto dto = svc.getJob(setup.jobId);
  EXPECT_EQ(dto.currentStage, JobStage::REPAIR);
  EXPECT_EQ(dto.percentComplete, 50);
}

/// @brief Verify that updateJobStage throws when mechanic doesn't own the job.
TEST_F(MechanicServiceTests, UpdateJobStage_WrongMechanic)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);
  auto otherMech = createTestMechanicWithProfile(db, svc, "Other", "other@test.com");

  EXPECT_THROW(svc.updateJobStage(otherMech, setup.jobId,
                                  JobStage::REPAIR, 50, ""),
               std::runtime_error);
}

/// @brief Verify that updateJobStage with a note persists that note.
TEST_F(MechanicServiceTests, UpdateJobStage_WithNote)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  svc.updateJobStage(setup.mechId, setup.jobId,
                     JobStage::PARTS, 20, "Ordering parts");

  auto notes = svc.listJobNotes(setup.jobId);
  bool found = false;
  for (const auto &n : notes)
  {
    if (n.text == "Ordering parts")
      found = true;
  }
  EXPECT_TRUE(found);
}

/// @}

// =====================================================================
/// @name 8. markJobComplete Tests
/// @{
// =====================================================================

/// @brief Verify that markJobComplete marks the job done and appointment COMPLETED.
TEST_F(MechanicServiceTests, MarkJobComplete_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  EXPECT_TRUE(svc.markJobComplete(setup.mechId, setup.jobId, "All done"));

  // Appointment should now be COMPLETED
  auto appt = db.getAppointmentById(setup.apptId);
  ASSERT_TRUE(appt.has_value());
  EXPECT_EQ(appt->status, AppointmentStatus::COMPLETED);
}

/// @brief Verify that markJobComplete throws when mechanic doesn't own the job.
TEST_F(MechanicServiceTests, MarkJobComplete_WrongMechanic)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);
  auto otherMech = createTestMechanicWithProfile(db, svc, "Other2", "other2@test.com");

  EXPECT_THROW(svc.markJobComplete(otherMech, setup.jobId, "Done"),
               std::runtime_error);
}

/// @brief Verify that markJobComplete adds a completion note to the job.
TEST_F(MechanicServiceTests, MarkJobComplete_CompletionNote)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  svc.markJobComplete(setup.mechId, setup.jobId, "Final summary");

  auto notes = svc.listJobNotes(setup.jobId);
  bool found = false;
  for (const auto &n : notes)
  {
    if (n.text == "Final summary" && n.type == "completion")
      found = true;
  }
  EXPECT_TRUE(found);
}

/// @}

// =====================================================================
/// @name 9. listJobNotes Tests
/// @{
// =====================================================================

/// @brief Verify that listJobNotes returns all notes for a job.
TEST_F(MechanicServiceTests, ListJobNotes_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);
  svc.addJobNote(setup.mechId, setup.jobId, "Note A");
  svc.addJobNote(setup.mechId, setup.jobId, "Note B");

  auto notes = svc.listJobNotes(setup.jobId);
  // "Job started" from startJobFromAppointment + "Note A" + "Note B"
  EXPECT_GE(notes.size(), 3u);
}

/// @brief Verify that listJobNotes returns empty when job has no manual notes.
TEST_F(MechanicServiceTests, ListJobNotes_OnlyInitialNote)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  auto notes = svc.listJobNotes(setup.jobId);
  // Should have at least the "Job started" note from startJobFromAppointment
  EXPECT_GE(notes.size(), 1u);
}

/// @}

// =====================================================================
/// @name 10. addJobNote Tests
/// @{
// =====================================================================

/// @brief Verify that addJobNote returns a positive JobNoteId.
TEST_F(MechanicServiceTests, AddJobNote_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  auto noteId = svc.addJobNote(setup.mechId, setup.jobId, "Customer called");
  EXPECT_GT(noteId, 0);
}

/// @brief Verify that addJobNote throws when mechanic doesn't own the job.
TEST_F(MechanicServiceTests, AddJobNote_WrongMechanic)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);
  auto otherMech = createTestMechanicWithProfile(db, svc, "Other3", "other3@test.com");

  EXPECT_THROW(svc.addJobNote(otherMech, setup.jobId, "Hack"),
               std::runtime_error);
}

/// @}

// =====================================================================
/// @name 11. listOpenJobs Tests
/// @{
// =====================================================================

/// @brief Verify that listOpenJobs returns open jobs for a mechanic.
TEST_F(MechanicServiceTests, ListOpenJobs_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  auto jobs = svc.listOpenJobs(setup.mechId);
  EXPECT_GE(jobs.size(), 1u);

  bool found = false;
  for (const auto &j : jobs)
  {
    if (j.jobId == setup.jobId)
      found = true;
  }
  EXPECT_TRUE(found);
}

/// @brief Verify that listOpenJobs returns empty when no jobs exist.
TEST_F(MechanicServiceTests, ListOpenJobs_Empty)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto mid = createTestMechanicWithProfile(db, svc, "Empty", "empty@test.com");

  auto jobs = svc.listOpenJobs(mid);
  EXPECT_TRUE(jobs.empty());
}

/// @brief Verify that completed jobs are excluded from listOpenJobs.
TEST_F(MechanicServiceTests, ListOpenJobs_CompletedExcluded)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto setup = createFullJobSetup(db, svc);

  // Complete the job
  svc.markJobComplete(setup.mechId, setup.jobId, "Finished");

  auto jobs = svc.listOpenJobs(setup.mechId);
  for (const auto &j : jobs)
  {
    EXPECT_NE(j.jobId, setup.jobId);
  }
}

/// @}

// =====================================================================
/// @name 12. listIncomingRequests Tests
/// @{
// =====================================================================

/// @brief Verify that listIncomingRequests returns REQUESTED appointments.
TEST_F(MechanicServiceTests, ListIncomingRequests_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "IR_C", "irc@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "IR_M", "irm@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);
  createTestAppointment(db, cid, mid, vid, fid);

  auto requests = svc.listIncomingRequests(mid);
  EXPECT_EQ(requests.size(), 1u);
  EXPECT_EQ(requests[0].status, AppointmentStatus::REQUESTED);
}

/// @brief Verify that listIncomingRequests returns empty when no requests exist.
TEST_F(MechanicServiceTests, ListIncomingRequests_Empty)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto mid = createTestMechanicWithProfile(db, svc, "IR_E", "ire@test.com");

  auto requests = svc.listIncomingRequests(mid);
  EXPECT_TRUE(requests.empty());
}

/// @brief Verify that CONFIRMED appointments are NOT included in incoming requests.
TEST_F(MechanicServiceTests, ListIncomingRequests_NonRequestedFiltered)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "IR_F", "irf@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "IR_FM", "irfm@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);

  // Create a CONFIRMED appointment (should be filtered out)
  createConfirmedAppointment(db, cid, mid, vid, fid);

  auto requests = svc.listIncomingRequests(mid);
  EXPECT_TRUE(requests.empty());
}

/// @}

// =====================================================================
/// @name 13. listMyReviews Tests
/// @{
// =====================================================================

/// @brief Verify that listMyReviews returns reviews with correct fields.
TEST_F(MechanicServiceTests, ListMyReviews_HappyPath)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "RV_C", "rvc@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "RV_M", "rvm@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);
  auto apptId = createConfirmedAppointment(db, cid, mid, vid, fid);
  auto jobId = svc.startJobFromAppointment(mid, apptId);

  ReviewRecord r;
  r.customerId = cid;
  r.mechanicId = mid;
  r.jobId = jobId;
  r.rating = 5;
  r.comment = "Excellent work!";
  db.createReview(r);

  auto reviews = svc.listMyReviews(mid);
  ASSERT_EQ(reviews.size(), 1u);
  EXPECT_EQ(reviews[0].rating, 5);
  EXPECT_EQ(reviews[0].comment, "Excellent work!");
  EXPECT_EQ(reviews[0].mechanicId, mid);
  EXPECT_EQ(reviews[0].customerName, "RV_C");
}

/// @brief Verify that listMyReviews returns empty when no reviews exist.
TEST_F(MechanicServiceTests, ListMyReviews_Empty)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto mid = createTestMechanicWithProfile(db, svc, "RV_E", "rve@test.com");

  auto reviews = svc.listMyReviews(mid);
  EXPECT_TRUE(reviews.empty());
}

/// @brief Verify that listMyReviews returns multiple reviews.
TEST_F(MechanicServiceTests, ListMyReviews_Multiple)
{
  DatabaseManager db;
  MechanicService svc(db);

  auto cid = createTestCustomer(db, "RV_MC", "rvmc@test.com");
  auto mid = createTestMechanicWithProfile(db, svc, "RV_MM", "rvmm@test.com");
  auto vid = createTestVehicle(db, cid);
  auto fid = createTestSymptomForm(db, cid, vid);

  // Each review needs its own job (UNIQUE constraint on reviews.jobId)
  auto apptId1 = createConfirmedAppointment(db, cid, mid, vid, fid);
  auto jobId1 = svc.startJobFromAppointment(mid, apptId1);

  auto apptId2 = createConfirmedAppointment(db, cid, mid, vid, fid);
  auto jobId2 = svc.startJobFromAppointment(mid, apptId2);

  ReviewRecord r1;
  r1.customerId = cid;
  r1.mechanicId = mid;
  r1.jobId = jobId1;
  r1.rating = 4;
  r1.comment = "Good";
  db.createReview(r1);

  ReviewRecord r2;
  r2.customerId = cid;
  r2.mechanicId = mid;
  r2.jobId = jobId2;
  r2.rating = 2;
  r2.comment = "Could be better";
  db.createReview(r2);

  auto reviews = svc.listMyReviews(mid);
  EXPECT_EQ(reviews.size(), 2u);
}

/// @}
