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

    MechanicId createMechanicProfile(UserId userId, const MechanicProfileCreate& profile) const;
    MechanicProfile getMechanicProfile(MechanicId mechanicId) const;

    bool updateMechanicProfile(MechanicId mechanicId, MechanicProfileUpdate updates);

    std::vector<AppointmentRequestView> listIncomingRequests(MechanicId mechanicId);

    std::vector <AppointmentRequestView> listIncomminRequests(MechanicId mechanicId);
    bool AcceptAppointment (AppointmentId appointmentId, TimeSlot proposedSlot, std::string note);
    bool declineAppointment(AppointmentId appointmentId, std::string reason) const;
    bool RescheduleAppointment(AppointmentId appointmentId, TimeSlot proposedSlot, std::string note);

    std::vector <AppointmentSummary> listAppointments(MechanicId mechanicId, DataRange dateRange);
    AppointmentDetails getAppointmentDetails(AppointmentId appointmentId);
    JobId startJobFromAppointment(MechanicId mechanicId, AppointmentId appointmentId);
    List <JobCardView> listOpenJobs(MechanicId mechanicId);
    JobDetailsView getJob(JobId jobId);

    bool updateJobStage(MechanicId mechanicId, JobId jobId, JobStage stage, int percentCompleted, std::string note);
    bool markJobBlocked(MechanicId, JobId jobId, std::string reasoning);
    bool markJobComplete(MechanicId mechanicId, JobId jobId, std::string completionSum);

    std::vector<ReviewSummary> listMyReviews(MechanicId mechanicId);

    void validateMechanicOwnAppointment(MechanicId mechanicId, AppointmentId appointmentId);
    void validateMechanicOwnsJob(MechanicId mechanicId, JobId jobId);
    void publicJobUpdate(JobId jobId);
    bool slotConflicts(MechanicId mechanicId, TimeSlot timeSlot);
    std::vector <JobStage> buildDefaultStages();

};



#endif //MECHANICSERVICE_H
