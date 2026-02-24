//
// Created by svkol on 2026-02-13.
//

#ifndef MECHANICSERVICE_H
#define MECHANICSERVICE_H

#include <string>
#include <vector>
#include"DTO.h"
#include "Records.h"
#include "DatabaseManager.h"



class MechanicService {

private:
    DatabaseManager& db;

public:
    explicit MechanicService(DatabaseManager& db);
    ~MechanicService();

    MechanicId createMechanicProfile(UserId userId, const MechanicCreate& profile);
    MechanicDTO getMechanicProfile(MechanicId mechanicId);

    bool updateMechanicProfile(MechanicId mechanicId, MechanicUpdate updates);
    std::vector<AppointmentDTO> listIncomingRequests(MechanicId mechanicId);
    bool AcceptAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note);
    bool declineAppointment(AppointmentId appointmentId, std::string reason);
    bool RescheduleAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note);

    std::vector<AppointmentDTO> listAppointments(MechanicId mechanicId, DateRange dateRange);
    AppointmentDTO getAppointmentDetails(AppointmentId appointmentId);
    JobId startJobFromAppointment(MechanicId mechanicId, AppointmentId appointmentId);
    std::vector<JobDTO> listOpenJobs(MechanicId mechanicId);
    JobDTO getJob(JobId jobId);

    bool updateJobStage(MechanicId mechanicId, JobId jobId, JobStage stage, int percentCompleted, std::string note);
    bool markJobBlocked(MechanicId mechanicId, JobId jobId, std::string reasoning);
    bool markJobComplete(MechanicId mechanicId, JobId jobId, std::string completionSum);

    std::vector<ReviewDTO> listMyReviews(MechanicId mechanicId);

    void validateMechanicOwnAppointment(MechanicId mechanicId, AppointmentId appointmentId);
    void validateMechanicOwnsJob(MechanicId mechanicId, JobId jobId);
    void publishJobUpdate(JobId jobId);
    bool slotConflicts(MechanicId mechanicId, const TimeSlot& timeSlot);
    std::vector<JobStage> buildDefaultStages();

};



#endif //MECHANICSERVICE_H
