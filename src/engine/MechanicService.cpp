//
// Created by svkol on 2026-02-13.
//

#include "MechanicService.h"
#include <stdexcept>

MechanicService::MechanicService(DatabaseManager& db) : db(db){}

MechanicId MechanicService::createMechanicProfile(UserId userId, const MechanicProfileCreate& profile){
    auto user = db.getUserRecordById(userId);
    if (!user.has_value()){
        throw std::runtime_error("user not found");
    }

    if (user -> role != UserRole::MECHANIC){
        throw std::runtime_error("User if not a mechanic");
    }

    auto existing = db.getMechanicByUserId(userId);
    if (existing.has_value()){
        throw std::runtime_error("Mechanic profile has already been created");
    }

    MechanicUpdate update;
    update.specialties = profile.specialties;
    update.displayName = profile.displayName;
    update.shopName = profile.shopName;
    update.hourlyRate = profile.hourlyRate;

    if (!db.updateMechanicProfile(userId, update)){
        throw std::runtime_error("Failed to create a mechanic profile");
    }

    auto mechanic = db.getMechanicByUserId(userId);
    if (!mechanic.has_value()){
        throw std::runtime_error("Failed to retreat mechanic's profile");
    }

    return mechanic -> userId;
}

MechanicProfile MechanicService::getMechanicProfile(MechanicId mechanicId){
    auto mechanic = db.getMechanicByUserId(mechanicId);

    if (!mechanic.has_value()){
        throw std::runtime_error("Failed to retrieve the mechanic");
    }

    auto reviews = db.listReviewsForMechanic(mechanicId);

    double avgRating = 0.0;
    if (!reviews.empty()){
        int totalRating = 0;
        for (const auto& review : reviews){
            totalRating +=review.rating;
        }
        avgRating= static_cast<double>(totalRating) / reviews.size();
    }

    MechanicProfile profile;
    profile.mechanicId = mechanic ->userId;
    profile.displayName = mechanic -> displayName;
    profile.userId = mechanic -> userId;
    profile.specialties = mechanic -> specialties;
    profile.hourlyRate= mechanic->hourlyRate;


    return profile;
}

bool MechanicService::updateMechanicProfile(MechanicId mechanicId, MechanicProfileUpdate updates){
    auto mechanic = db.getMechanicByUserId(mechanicId);
    if (!mechanic.has_value()){
        throw std::runtime_error("Mechanic profile was not found");
    }

    return db.updateMechanicProfile(mechanicId, updates);
}

std::vector<AppointmentRequestView> MechanicService::listIncomingRequests(MechanicId mechanicId)
{
    // Get all appointments for this mechanic with "pending" status
    auto appointments = db_.listAppointmentsForMechanic(mechanicId);

    std::vector<AppointmentRequestView> requests;
    for (const auto& appt : appointments) {
        if (appt.status == AppointmentStatus::REQUESTED) {
            AppointmentRequestView view;
            view.appointmentId = appt.appointmentId;
            view.customerId = appt.customerId;

            // Get customer details
            auto customer = db.getUserRecordById(appt.customerId);
            if (customer.has_value()) {
                view.customerName = customer->name;
                view.customerEmail = customer->email;
            }

            // Get vehicle details
            auto vehicle = db.getVehicleById(appt.vehicleId);
            if (vehicle.has_value()) {
                view.vehicleid = vehicle->id;
                view.make = vehicle->make;
                view.model = vehicle->model;
                view.year = vehicle->year;
            }

            // Get symptom form
            if (appt.symptomFormId.has_value()) {
                auto form = db.getSymptomFormById(appt.symptomFormId.value());
                if (form.has_value()) {
                    view.symptoms = form->description;
                    view.urgency = form->severity;
                }
            }

            view.requestedAt = appt.createdAt;
            view.notes = appt.notes;

            requests.push_back(view);
        }
    }

    return requests;
}


bool MechanicService::AcceptAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note){

    auto appt = db.getAppointmentById(appointmentId);

    if (!appt.has_value()){
        throw std::runtime_error("Appointment not found");
    }

    if (!appt -> status != AppointmentStatus::REQUESTED){
        throw std::runtime_error("Appointment was not requested");
    }

    if (slotConflicts(appt->mechanicId, proposedSlot)){
        throw std::runtime_error("Selected time slot conflicts with existing appointments");
    }

    db.beginTransaction();
    try{
        bool success = db.updateAppointmentStatus(AppointmentId, AppointmentStatus::SCHEDULED);
        if (!success){
            db.rollback();
            return false;
        }

        //needs to filled more

    }catch (...){
        db.rollback();
        throw;
    }
}


bool MechanicService::declineAppointment(AppointmentId appointmentId, std::string reason){
    auto appt= db.getAppointmentById(appointmentId);

    if (!appt.has_value()){
        throw std::runtime_error("Appointment was not found");
    }

    if (appt -> status != AppointmentStatus::REQUESTED){
        throw std::runtime_error("Appointment was not requested");
    }

    return db.cancelAppointment(appointmentId, reason);
}

bool MechanicService::RescheduleAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note){
    if (!appt.has_value()){
        throw std::runtime_error("Appointment was not found");
    }

    if (appt -> status != AppointmentStatus::REQUESTED){
        throw std::runtime_error("Appointment was not requested");
    }

    //needs appointmentUpdate structure with shceduled time;

    db.updateAppointmentStatus(appointmentId, AppointmentStatus::SCHEDULED);

    return true;
}


std::vector<AppointmentSummary> MechanicService::listAppointments(MechanicId mechanicId, DataRange dateRanges, VehicleSummary vehicle){

    auto appts = db.listAppointmentsForMechanic(mechanicId);


    std::vector<AppointmentSummary> sums;
    for (const auto& appt:appts){
        // Filter by date range if scheduledTime falls within range
        // TODO: Implement date filtering based on appt.scheduledTime

        AppointmentSummary summary;
        summary.appointmentId = appt.appointmentId;

        auto customer = db.getUserRecordById(appt.customerId);
        if (customer.has_value()){
            summary.vehicleDescription =vehicle.year + " " + vehicle.make + " " + vehicle.model;
        }

        auto vehicle = db.getVehicleById(appt.vehicleId);
        if (!vehicle.has_value()){
            summary.customerName = customer -> name;
        }

        summary.scheduledAt = appt.scheduledAt;
        summary.status = appt.status;

        sums.push_back(summary);
    }
    return sums;
}

AppointmentDetails MechanicService::getAppointmentDetails(AppointmentId appointmentId){
    auto appt= db.getAppointmentById(appointmentId);
    if (!appt.has_value()){
        throw std::runtime_error("Appointment was not found");
    }

    AppointmentDetails details;
    details.appointmentId = appt->appointmentId;
    details.status = appt ->status;
    details.scheduledAt = appt -> scheduledAt;
    details.note = appt->note;
    details.createdAt = appt->createdAt;


    auto customer = db.getUserRecordById(appt->customerId);
    if (customer.has_value()){
        details.customerName = customer -> name;
        details.customerId = customer -> id;
        details.customerPhone = customer -> phone;
    }

    auto vehicle = db.getVehicleById(appt -> vehicleId);
    if (vehicle.has_value()){
        details.vehicle = *vehicle;
    }

    if (appt->symptomForm.has_value()){
        auto form = db.getSymptomFormById(appt ->symptomFormId);
        if (form.has_value()){
            details.symptomForm = *form;
        }
    }

    return details;
}

JobId MechanicService::startJobFromAppointment(MechanicId mechanicId, AppointmentId appointmentId){
    validateMechanicOwnAppointment(mechanicId, appointmentId);

    auto appt = db.getAppointmentById(appointmentId);
    if (!appt.has_value()){
        throw std::runtime_error("Appointment not found");
    }

    if (appt->status != AppointmentStatus::CONFIRMED) {
        throw std::runtime_error("Can only start job from confirmed appointment");
    }

    db.beginTransaction();

    try{
        JobId jobid= db.createJobFromAppointment(appointmentId);

        db.updateAppointmentStatus(appointmentId, AppointmentStatus::SCHEDULED);

        auto stages = buildDefaultStages();;
        if (!stages.empty()){
            db.updateJobStage(jobid, stages[0], 0, "Job Started");
        }

        db.commit();
        return jobid;

    }catch (...){
        db.rollback();
        throw;
    }
}

std::vector<JobCardView> MechanicService::listOpenJobs(MechanicId mechanicId)
{
    auto jobs = db.listOpenJobsForMechanic(mechanicId);

    std::vector<JobCardView> cards;
    for (const auto& job : jobs) {
        JobCardView card;
        card.jobId = job.id;

        // Get customer info
        auto customer = db.getUserRecordById(job.customerId);
        if (customer.has_value()) {
            card.customerName = customer->name;
        }

        // Get vehicle info
        auto vehicle = db.getVehicleById(job.vehicleId);
        if (vehicle.has_value()) {
            card.vehicleDescription = vehicle->year + " " + vehicle->make + " " + vehicle->model;
        }

        card.currentStage = job.currentStage;
        card.percentComplete = job.percentComplete;
        card.startedAt = job.startedAt;
        card.isBlocked = (job.currentStage == "blocked");

        cards.push_back(card);
    }

    return cards;
}

JobDetailsView MechanicService::getJob(JobId jobId)
{
    auto job = db.getJobById(jobId);
    if (!job.has_value()) {
        throw std::runtime_error("Job not found");
    }

    JobDetailsView details;
    details.jobId = job->id;
    details.appointmentId = job->appointmentId;
    details.currentStage = job->currentStage;
    details.percentComplete = job->percentComplete;
    details.startedAt = job->startedAt;
    details.completedAt = job->completedAt;
    details.completionNote = job->completionNote;

    // Get customer details
    auto customer = db.getUserRecordById(job->customerId);
    if (customer.has_value()) {
        details.customerName = customer->name;
        details.customerEmail = customer->email;
    }

    // Get vehicle details
    auto vehicle = db.getVehicleById(job->vehicleId);
    if (vehicle.has_value()) {
        details.vehicle = *vehicle;
    }

    // Get all job stages/history
    details.currentStage = job->currentStage;

    return details;
}

bool MechanicService::updateJobStage(MechanicId mechanicId, JobId jobId, JobStage stage, int percentCompleted, std::string note){
    validateMechanicOwnsJob(mechanicId, jobId);

    bool success = db.updateJobStage(jobId, stage, percentComplete, note);
    if (success) {
        publishJobUpdate(jobId);
        //TODO: this
    }

    return success;
}

bool MechanicService::markJobBlocked(MechanicId mechanicId, JobId jobId, const std::string& reason)
{
    validateMechanicOwnsJob(mechanicId, jobId);

    JobStage blockedStage;
    blockedStage.stageName = "blocked";
    blockedStage.description = reason;

    return db.updateJobStage(jobId, blockedStage, -1, reason);
}

bool MechanicService::markJobComplete(MechanicId mechanicId, JobId jobId, const std::string& completionSummary)
{
    validateMechanicOwnsJob(mechanicId, jobId);

    bool success = db.markJobComplete(jobId, completionSummary);
    if (success) {
        // Update associated appointment to completed
        auto job = db.getJobById(jobId);
        if (job.has_value()) {
            db.updateAppointmentStatus(job->appointmentId, AppointmentStatus::Completed);
        }

        publishJobUpdate(jobId);
    }

    return success;
}

// ---------------- Reviews ----------------

std::vector<ReviewSummary> MechanicService::listMyReviews(MechanicId mechanicId)
{
    auto reviews = db_.listReviewsForMechanic(mechanicId);

    std::vector<ReviewSummary> summaries;
    for (const auto& review : reviews) {
        ReviewSummary summary;
        summary.reviewId = review.reviewId;
        summary.rating = review.rating;
        summary.comment = review.comment;
        summary.createdAt = review.createdAt;

        // Get customer name
        auto customer = db_.getUserRecordById(review.customerId);
        if (customer.has_value()) {
            summary.customerName = customer->name;
        }

        summaries.push_back(summary);
    }

    return summaries;
}

// ---------------- Private Helper Methods ----------------

void MechanicService::validateMechanicOwnsAppointment(MechanicId mechanicId, AppointmentId appointmentId)
{
    auto appt = db_.getAppointmentById(appointmentId);
    if (!appt.has_value()) {
        throw std::runtime_error("Appointment not found");
    }

    if (appt->mechanicId != mechanicId) {
        throw std::runtime_error("Mechanic does not own this appointment");
    }
}

void MechanicService::validateMechanicOwnsJob(MechanicId mechanicId, JobId jobId)
{
    auto job = db_.getJobById(jobId);
    if (!job.has_value()) {
        throw std::runtime_error("Job not found");
    }

    if (job->mechanicId != mechanicId) {
        throw std::runtime_error("Mechanic does not own this job");
    }
}

void MechanicService::publishJobUpdate(JobId jobId)
{
    // TODO: Implement notification system
    // This could publish to a message queue, send email/SMS, or update a real-time feed
    // For now, this is a placeholder
}

bool MechanicService::slotConflicts(MechanicId mechanicId, const TimeSlot& slot)
{
    // Get all confirmed appointments for this mechanic
    auto appointments = db_.listAppointmentsForMechanic(mechanicId);

    for (const auto& appt : appointments) {
        if (appt.status == AppointmentStatus::CONFIRMED ||
            appt.status == AppointmentStatus::IN_PROGRESS) {

            // Check if times overlap
            // TODO: Implement proper time overlap logic based on TimeSlot structure
            // This is a simplified placeholder
            if (appt.scheduledTime == slot.start) {
                return true;
            }
        }
    }

    return false;
}

std::vector<JobStage> MechanicService::buildDefaultStages()
{
    std::vector<JobStage> stages;

    JobStage diagnostic;
    diagnostic.stageName = "diagnostic";
    diagnostic.description = "Initial diagnostic assessment";
    stages.push_back(diagnostic);

    JobStage partsOrdering;
    partsOrdering.stageName = "parts_ordering";
    partsOrdering.description = "Ordering required parts";
    stages.push_back(partsOrdering);

    JobStage repair;
    repair.stageName = "repair";
    repair.description = "Performing repairs";
    stages.push_back(repair);

    JobStage testing;
    testing.stageName = "testing";
    testing.description = "Testing and quality check";
    stages.push_back(testing);

    return stages;
}




