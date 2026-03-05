//
// Created by svkol on 2026-02-13.
//

#include "MechanicService.h"
#include <stdexcept>

MechanicService::MechanicService(DatabaseManager& db) : db(db){}

MechanicService::~MechanicService() = default;

MechanicId MechanicService::createMechanicProfile(UserId userId, const MechanicCreate& profile){
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

MechanicDTO MechanicService::getMechanicProfile(MechanicId mechanicId){
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

    MechanicDTO profile;
    profile.mechanicId = mechanic ->userId;
    profile.displayName = mechanic -> displayName;
    profile.userId = mechanic -> userId;
    profile.specialties = mechanic -> specialties;
    profile.hourlyRate= mechanic->hourlyRate;
    profile.averageRating = avgRating;
    profile.reviewCount = static_cast<int>(reviews.size());


    return profile;
}

bool MechanicService::updateMechanicProfile(MechanicId mechanicId, MechanicUpdateDTO updates){
    auto mechanic = db.getMechanicByUserId(mechanicId);
    if (!mechanic.has_value()){
        throw std::runtime_error("Mechanic profile was not found");
    }

    MechanicUpdate update;
    update.specialties = updates.specialties;
    update.displayName = updates.displayName;
    update.shopName = updates.shopName;
    update.hourlyRate = updates.hourlyRate;


    return db.updateMechanicProfile(mechanicId, update);
}

std::vector<AppointmentDTO> MechanicService::listIncomingRequests(MechanicId mechanicId)
{
    // Get all appointments for this mechanic with "pending" status
    auto appointments = db.listAppointmentsForMechanic(mechanicId);

    std::vector<AppointmentDTO> requests;
    for (const auto& appt : appointments) {
        if (appt.status == AppointmentStatus::REQUESTED) {
            AppointmentDTO view;
            view.appointmentId = appt.appointmentId;
            view.customerId = appt.customerId;
            view.mechanicId = appt.mechanicId;
            view.status = appt.status;
            view.createdAt = appt.createdAt;
            view.note = appt.note;

            // Get customer details
            auto customer = db.getUserRecordById(appt.customerId);
            if (customer.has_value()) {
                view.customerName = customer->name;
                view.customerEmail = customer->email;
            }

            // Get vehicle details
            auto vehicle = db.getVehicleById(appt.vehicleId);
            if (vehicle.has_value()) {
                view.vehicleId = vehicle->id;
                view.vehicleDescription = std::to_string(vehicle->year) + " " + vehicle->make + " " + vehicle->model;
            }

            // Get symptom form
            if (appt.symptomFormId > 0) {
                try {
                    auto form = db.getSymptomFormById(appt.symptomFormId);
                    view.symptoms = form.description;
                    view.severity = form.severity;
                } catch (...) {}
            }

            requests.push_back(view);
        }
    }

    return requests;
}


bool MechanicService::AcceptAppointment(AppointmentId appointmentId){

    auto appt = db.getAppointmentById(appointmentId);

    if (!appt.has_value()){
        throw std::runtime_error("Appointment not found");
    }

    if (appt->status != AppointmentStatus::REQUESTED){
        throw std::runtime_error("Appointment was not requested");
    }

    db.beginTransaction();
    try {
        bool success = db.updateAppointmentStatus(appointmentId, AppointmentStatus::SCHEDULED);
        if (!success){
            db.rollback();
            return false;
        }

        // needs to be filled more

        db.commit();
    } catch (...) {
        db.rollback();
        throw;
    }
    return true;
}


bool MechanicService::declineAppointment(AppointmentId appointmentId, std::string reason){
    auto appt = db.getAppointmentById(appointmentId);

    if (!appt.has_value()){
        throw std::runtime_error("Appointment was not found");
    }

    if (appt -> status != AppointmentStatus::REQUESTED){
        throw std::runtime_error("Appointment was not requested");
    }

    return db.cancelAppointment(appointmentId, reason);
}

bool MechanicService::RescheduleAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note){
    auto appt = db.getAppointmentById(appointmentId);
    if (!appt.has_value()){
        throw std::runtime_error("Appointment was not found");
    }

    if (appt->status != AppointmentStatus::REQUESTED && appt->status != AppointmentStatus::SCHEDULED){
        throw std::runtime_error("Appointment cannot be rescheduled in its current state");
    }

    // TODO: update scheduledAt with proposedSlot

    db.updateAppointmentStatus(appointmentId, AppointmentStatus::SCHEDULED);

    return true;
}


std::vector<AppointmentDTO> MechanicService::listAppointments(MechanicId mechanicId, DateRange dateRange){

    auto appts = db.listAppointmentsForMechanic(mechanicId);


    std::vector<AppointmentDTO> sums;
    for (const auto& appt : appts){
        // Filter by date range if scheduledTime falls within range
        // TODO: Implement date filtering based on appt.scheduledTime

        AppointmentDTO summary;
        summary.appointmentId = appt.appointmentId;
        summary.mechanicId = appt.mechanicId;
        summary.status = appt.status;
        summary.scheduledAt = appt.scheduledAt;

        auto customer = db.getUserRecordById(appt.customerId);
        if (customer.has_value()){
            summary.customerName = customer->name;
        }

        auto vehicle = db.getVehicleById(appt.vehicleId);
        if (vehicle.has_value()){
            summary.vehicleDescription = std::to_string(vehicle->year) + " " + vehicle->make + " " + vehicle->model;
        }

        sums.push_back(summary);
    }
    return sums;
}

AppointmentDTO MechanicService::getAppointmentDetails(AppointmentId appointmentId){
    auto appt= db.getAppointmentById(appointmentId);
    if (!appt.has_value()){
        throw std::runtime_error("Appointment was not found");
    }

    AppointmentDTO details;
    details.appointmentId = appt->appointmentId;
    details.mechanicId = appt->mechanicId;
    details.status = appt->status;
    details.scheduledAt = appt->scheduledAt;
    details.note = appt->note;
    details.createdAt = appt->createdAt;


    auto customer = db.getUserRecordById(appt->customerId);
    if (customer.has_value()){
        details.customerName = customer->name;
        details.customerId = customer->id;
        details.customerPhone = customer->phone;
        details.customerEmail = customer->email;
    }

    auto vehicle = db.getVehicleById(appt->vehicleId);
    if (vehicle.has_value()){
        details.vehicleId = vehicle->id;
        details.vehicleDescription = std::to_string(vehicle->year) + " " + vehicle->make + " " + vehicle->model;
    }

    if (appt->symptomFormId > 0){
        try {
            auto form = db.getSymptomFormById(appt->symptomFormId);
            details.symptoms = form.description;
            details.severity = form.severity;
        } catch (...) {}
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
            db.updateJobStage(jobid, stages[0], 0);
            db.addJobNote(jobid, "update", "Job started");
        }

        db.commit();
        return jobid;

    }catch (...){
        db.rollback();
        throw;
    }
}

std::vector<JobDTO> MechanicService::listOpenJobs(MechanicId mechanicId)
{
    auto jobs = db.listOpenJobsForMechanic(mechanicId);

    std::vector<JobDTO> cards;
    for (const auto& job : jobs) {
        JobDTO card;
        card.jobId = job.id;
        card.customerId = job.customerId;
        card.mechanicId = job.mechanicId;
        card.appointmentId = job.appointmentId;

        // Get customer info
        auto customer = db.getUserRecordById(job.customerId);
        if (customer.has_value()) {
            card.customerName = customer->name;
        }

        // Get vehicle info
        auto vehicle = db.getVehicleById(job.vehicleId);
        if (vehicle.has_value()) {
            card.vehicleDescription = std::to_string(vehicle->year) + " " + vehicle->make + " " + vehicle->model;
        }

        card.currentStage = job.stage;
        card.percentComplete = job.percentComplete;
        card.startedAt = job.startedAt;
        card.isBlocked = false; // TODO: determine from stage

        cards.push_back(card);
    }

    return cards;
}

JobDTO MechanicService::getJob(JobId jobId)
{
    auto job = db.getJobById(jobId);
    if (!job.has_value()) {
        throw std::runtime_error("Job not found");
    }

    JobDTO details;
    details.jobId = job->id;
    details.appointmentId = job->appointmentId;
    details.mechanicId = job->mechanicId;
    details.customerId = job->customerId;
    details.currentStage = job->stage;
    details.percentComplete = job->percentComplete;
    details.startedAt = job->startedAt;
    details.completedAt = job->completedAt;

    // Load the notes log
    auto noteRecords = db.listJobNotes(job->id);
    for (const auto& nr : noteRecords) {
        JobNoteDTO noteDto;
        noteDto.noteId = nr.id;
        noteDto.type = nr.type;
        noteDto.text = nr.text;
        noteDto.createdAt = nr.createdAt;
        details.notes.push_back(noteDto);
    }

    // Get customer details
    auto customer = db.getUserRecordById(job->customerId);
    if (customer.has_value()) {
        details.customerName = customer->name;
        details.customerEmail = customer->email;
    }

    // Get vehicle details
    auto vehicle = db.getVehicleById(job->vehicleId);
    if (vehicle.has_value()) {
        details.vehicleDescription = std::to_string(vehicle->year) + " " + vehicle->make + " " + vehicle->model;
    }

    return details;
}

bool MechanicService::updateJobStage(MechanicId mechanicId, JobId jobId, JobStage stage, int percentCompleted, std::string note){
    validateMechanicOwnsJob(mechanicId, jobId);

    bool success = db.updateJobStage(jobId, stage, percentCompleted);
    if (success) {
        if (!note.empty()) {
            db.addJobNote(jobId, "update", note);
        }
        publishJobUpdate(jobId);
    }

    return success;
}

bool MechanicService::markJobBlocked(MechanicId mechanicId, JobId jobId, const std::string& reason)
{
    validateMechanicOwnsJob(mechanicId, jobId);

    // Mark the job as blocked by setting percentComplete to -1
    bool success = db.updateJobStage(jobId, JobStage::REPAIR, -1);
    if (success) {
        db.addJobNote(jobId, "blocked", reason);
    }
    return success;
}

bool MechanicService::markJobComplete(MechanicId mechanicId, JobId jobId, const std::string& completionSummary)
{
    validateMechanicOwnsJob(mechanicId, jobId);

    bool success = db.markJobComplete(jobId);
    if (success) {
        db.addJobNote(jobId, "completion", completionSummary);

        // Update associated appointment to completed
        auto job = db.getJobById(jobId);
        if (job.has_value()) {
            db.updateAppointmentStatus(job->appointmentId, AppointmentStatus::COMPLETED);
        }

        publishJobUpdate(jobId);
    }

    return success;
}

JobNoteId MechanicService::addJobNote(MechanicId mechanicId, JobId jobId, const std::string& text)
{
    validateMechanicOwnsJob(mechanicId, jobId);
    return db.addJobNote(jobId, "update", text);
}

std::vector<JobNoteDTO> MechanicService::listJobNotes(JobId jobId)
{
    auto noteRecords = db.listJobNotes(jobId);
    std::vector<JobNoteDTO> notes;
    for (const auto& nr : noteRecords) {
        JobNoteDTO noteDto;
        noteDto.noteId = nr.id;
        noteDto.type = nr.type;
        noteDto.text = nr.text;
        noteDto.createdAt = nr.createdAt;
        notes.push_back(noteDto);
    }
    return notes;
}

// ---------------- Reviews ----------------

std::vector<ReviewDTO> MechanicService::listMyReviews(MechanicId mechanicId)
{
    auto reviews = db.listReviewsForMechanic(mechanicId);

    std::vector<ReviewDTO> summaries;
    for (const auto& review : reviews) {
        ReviewDTO summary;
        summary.reviewId = review.id;
        summary.mechanicId = review.mechanicId;
        summary.customerId = review.customerId;
        summary.rating = review.rating;
        summary.comment = review.comment;
        summary.createdAt = review.createdAt;

        // Get customer name
        auto customer = db.getUserRecordById(review.customerId);
        if (customer.has_value()) {
            summary.customerName = customer->name;
        }

        summaries.push_back(summary);
    }

    return summaries;
}

// ---------------- Private Helper Methods ----------------

void MechanicService::validateMechanicOwnAppointment(MechanicId mechanicId, AppointmentId appointmentId)
{
    auto appt = db.getAppointmentById(appointmentId);
    if (!appt.has_value()) {
        throw std::runtime_error("Appointment not found");
    }

    if (appt->mechanicId != mechanicId) {
        throw std::runtime_error("Mechanic does not own this appointment");
    }
}

void MechanicService::validateMechanicOwnsJob(MechanicId mechanicId, JobId jobId)
{
    auto job = db.getJobById(jobId);
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

std::vector<JobStage> MechanicService::buildDefaultStages()
{
    // Returns the standard progression of job stages
    return {
        JobStage::DIAGNOSTICS,
        JobStage::PARTS,
        JobStage::REPAIR,
        JobStage::QA,
        JobStage::DONE
    };
}




