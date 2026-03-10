/**
 * @file MechanicService.h
 * @brief Business logic for mechanic operations — profile, appointments,
 *        job lifecycle, notes, and reviews.
 */

#ifndef MECHANICSERVICE_H
#define MECHANICSERVICE_H

#include <string>
#include <vector>
#include"DTO.h"
#include "Records.h"
#include "DatabaseManager.h"



/// @brief Service handling all mechanic-facing business logic.
class MechanicService {

private:
    DatabaseManager& db; ///< shared DB instance

public:
    explicit MechanicService(DatabaseManager& db);
    ~MechanicService();

    // ---- Profile ----

    /// @brief Create a new mechanic profile linked to an existing user.
    MechanicId createMechanicProfile(UserId userId, const MechanicCreate& profile);
    /// @brief Fetch mechanic profile by user ID.
    MechanicDTO getMechanicProfile(MechanicId mechanicId);
    /// @brief Update editable mechanic profile fields.
    bool updateMechanicProfile(MechanicId mechanicId, MechanicUpdateDTO updates);

    // ---- Appointments ----

    /// @brief List pending appointment requests for a mechanic.
    std::vector<AppointmentDTO> listIncomingRequests(MechanicId mechanicId);
    /// @brief Accept a pending appointment request.
    bool AcceptAppointment(AppointmentId appointmentId);
    /// @brief Decline an appointment with a note.
    bool declineAppointment(AppointmentId appointmentId, std::string note);
    /// @brief Propose a new time slot for an appointment.
    bool RescheduleAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note);
    /// @brief List appointments within a date range.
    std::vector<AppointmentDTO> listAppointments(MechanicId mechanicId, DateRange dateRange);
    /// @brief Get full details for a single appointment.
    AppointmentDTO getAppointmentDetails(AppointmentId appointmentId);

    // ---- Jobs ----

    /// @brief Create a new job from a confirmed appointment.
    JobId startJobFromAppointment(MechanicId mechanicId, AppointmentId appointmentId);
    /// @brief List all open (non-complete) jobs for a mechanic.
    std::vector<JobDTO> listOpenJobs(MechanicId mechanicId);
    /// @brief Get full job details including notes.
    JobDTO getJob(JobId jobId);
    /// @brief Advance a job to a new stage with progress and optional note.
    bool updateJobStage(MechanicId mechanicId, JobId jobId, JobStage stage, int percentCompleted, std::string note);
    /// @brief Flag a job as blocked with reasoning.
    bool markJobBlocked(MechanicId mechanicId, JobId jobId, const std::string& reasoning);
    /// @brief Mark a job as complete with a summary note.
    bool markJobComplete(MechanicId mechanicId, JobId jobId, const std::string& completionSum);

    // ---- Job Notes ----

    /// @brief Add a note to a job's activity log.
    JobNoteId addJobNote(MechanicId mechanicId, JobId jobId, const std::string& text);
    /// @brief List all notes for a job.
    std::vector<JobNoteDTO> listJobNotes(JobId jobId);

    // ---- Reviews ----

    /// @brief List all reviews received by this mechanic.
    std::vector<ReviewDTO> listMyReviews(MechanicId mechanicId);

    // ---- Internal Helpers ----

    /// @brief Throw if mechanic doesn't own the appointment.
    void validateMechanicOwnAppointment(MechanicId mechanicId, AppointmentId appointmentId);
    /// @brief Throw if mechanic doesn't own the job.
    void validateMechanicOwnsJob(MechanicId mechanicId, JobId jobId);
    /// @brief Publish a job update notification (stub).
    void publishJobUpdate(JobId jobId);
    /// @brief Return the default job stage progression.
    std::vector<JobStage> buildDefaultStages();
    /// @brief Search mechanics by filter criteria.
    std::vector<MechanicDTO> searchMechanics(const MechanicSearchFilter &filters); 

};



#endif //MECHANICSERVICE_H
