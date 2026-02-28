Kay was here!
Aaron was also here!
Dame was also also here!
Shane was also also here! 
Sviatoslav was also here!

# TorqueDesk: Modern Automotive Repair Management Software

Built for a third-year software engineering course, TorqueDesk is a platform designed to take the headache out of automotive repairs for both the mechanic and the vehicle owner.

## Environment Setup

TorqueDesk uses **CMake (>= 3.20)**, **Ninja**, and **C++20**. Dependencies (nlohmann/json + GoogleTest) are downloaded automatically via CMake **FetchContent** (no manual installs needed beyond basic build tools).

---

## Prerequisites

### macOS

1. Install Xcode Command Line Tools:
  - `xcode-select --install`
2. Install CMake + Ninja (Homebrew):
  - `brew install cmake ninja`

### Linux (native or WSL)

Install a C++ toolchain, CMake, Ninja, and Git using your distro package manager.
Example (Ubuntu/WSL):

- `sudo apt update && sudo apt install -y build-essential cmake ninja-build git`

> Note: First configure/build may take longer because CMake will fetch and build dependencies.

---

## Build (Ninja)

From the repo root:

### Configure

- `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug`

### Build

- `cmake --build build`

---

## Run Unit Tests

After building:

- `ctest --test-dir build --output-on-failure`

If CTest doesn't find tests in your configuration, you can also run the test binary directly:

- `./build/tests/UnitTests` (path may vary by generator/config)

---

## Visual Studio 2022 + WSL workflow (Windows)

If you're using Visual Studio with a WSL toolchain:

- Open the folder as a CMake project.
- Ensure the selected kit/toolchain targets WSL.
- Build with the default CMake targets (Ninja is typical for VS CMake projects).

---

## Extra Working Notes

- The database is always accessed through DatabaseManager, but since SQLite is blocking (can only do one call at a time), every DB call is offloaded to a net::thread_pool via net::co_spawn when working on the server.

## Potential Errors

- db.updateVehicle() and db.deleteVehicle() return bool. If they return false because the ID doesn't exist (vs. a real DB error), the client still gets a 500 Internal Server Error instead of a 404 Not Found. The DB API doesn't distinguish these two cases.
- No validation on VehicleRecord fields after deserialization
After parsing, there is no validation (like if the year is reasonable, vin is non-empty, mileage is non-negative). Garbage data can be written directly to the db.

## Database Schema

This section describes the physical SQLite schema (`DatabaseManager`), not response DTOs.

### Physical tables

- `customers`
  - `id` (PK), `name`, `phone`, `email` (UNIQUE), `password`, `role`, `createdAt`
- `mechanics`
  - `mechanicId` (PK), `userId` (UNIQUE, FK -> `customers.id`), `displayName`, `shopName`, `hourlyRate`, `specialties`
- `vehicles`
  - `id` (PK), `ownerUserId` (FK -> `customers.id`), `vin` (UNIQUE), `make`, `model`, `year`, `mileage`, `createdAt`
- `symptom_forms`
  - `id` (PK), `customerId` (FK -> `customers.id`), `vehicleId` (FK -> `vehicles.id`), `description`, `severity`, `createdAt`
- `mechanic_availability`
  - `id` (PK), `mechanicId` (FK -> `mechanics.mechanicId`), `start`, `end`, UNIQUE(`mechanicId`,`start`,`end`)
- `appointments`
  - `id` (PK), `customerId` (FK -> `customers.id`), `mechanicId` (FK -> `mechanics.mechanicId`), `vehicleId` (FK -> `vehicles.id`), `symptomFormId` (FK -> `symptom_forms.id`), `scheduledAt`, `status`, `note`, `createdAt`
- `jobs`
  - `id` (PK), `appointmentId` (UNIQUE FK -> `appointments.id`), `mechanicId` (FK -> `mechanics.mechanicId`), `customerId` (FK -> `customers.id`), `vehicleId` (FK -> `vehicles.id`), `stage`, `percentComplete`, `lastNote`, `updatedAt`, `startedAt`, `completedAt`, `completionNote`
- `reviews`
  - `id` (PK), `jobId` (UNIQUE FK -> `jobs.id`), `customerId` (FK -> `customers.id`), `mechanicId` (FK -> `mechanics.mechanicId`), `rating`, `comment`, `createdAt`

### DTO/computed fields (not stored as table columns)

- Appointment-facing extras: `customerName`, `customerEmail`, `customerPhone`, `mechanicName`, `vehicleDescription`, `symptoms`
- Job-facing extras: `customerName`, `customerEmail`, `vehicleDescription`, `isBlocked`
- Mechanic-facing extras: `averageRating`, `reviewCount`
- Review-facing extras: `customerName`

### Local DB note

- The project currently initializes schema at startup with `CREATE TABLE IF NOT EXISTS`.
- Existing local DB files may not pick up newer constraints/relations automatically.
- During development, resetting/recreating `torquedesk.db` is expected when schema changes.

## Endpoints

> **Design rule:** each handler file only owns routes whose first path
> segment matches the key registered in `HttpSession::register_handlers()`.
> For example, `GET /users/{id}/vehicles` starts with `users`, so it is
> routed to `CustomersHandler` (Customers.cpp), **not** Vehicles.cpp.

### 1. Users & Auth (`Customers.cpp`)

*Registration/login, user profile routes, and `/users/{id}/...` nested routes.*

| HTTP Method | Endpoint                       | Status |
| ----------- | ------------------------------ | ------ |
| **POST**    | `/auth/register`               | Not implemented (`501`) |
| **POST**    | `/auth/login`                  | Not implemented (`501`) |
| **GET**     | `/users/{id}`                  | Wired, but depends on WIP service method |
| **PATCH**   | `/users/{id}`                  | Implemented (`updateUserRecord`) |
| **DELETE**  | `/users/{id}`                  | Wired, but depends on WIP service method |
| **PATCH**   | `/users/{id}/password`         | Wired, but depends on WIP service method |
| **GET**     | `/users/{userId}/vehicles`     | Implemented |
| **POST**    | `/users/{userId}/vehicles`     | Implemented |
| **GET**     | `/users/{userId}/symptoms`     | Wired, but depends on WIP service method |
| **GET**     | `/users/{userId}/appointments` | Wired, but depends on WIP service method |
| **GET**     | `/users/{userId}/reviews`      | Wired, but depends on WIP service method |


---

### 2. Vehicles (`Vehicles.cpp`)

*CRUD for vehicles and any `/vehicles/{id}/…` nested routes.*


| HTTP Method | Endpoint                         | Status |
| ----------- | -------------------------------- | ------ |
| **GET**     | `/vehicles/{id}`                 | Implemented |
| **PATCH**   | `/vehicles/{id}`                 | Implemented |
| **DELETE**  | `/vehicles/{id}`                 | Implemented |
| **POST**    | `/vehicles/{vehicleId}/symptoms` | Not implemented (`501`) |


---

### 3. Symptom Forms (`Symptoms.cpp`)

*Direct symptom-form access (`/symptoms/…` routes only).*


| HTTP Method | Endpoint         | Status |
| ----------- | ---------------- | ------ |
| **GET**     | `/symptoms/{id}` | Not implemented (`501`) |
| **PUT**     | `/symptoms/{id}` | Not implemented (`501`) |
| **DELETE**  | `/symptoms/{id}` | Not implemented (`501`) |


---

### 4. Mechanics (`Mechanics.cpp`)

*Search, profile management, and any `/mechanics/{id}/…` nested routes.*


| HTTP Method | Endpoint                       | Status |
| ----------- | ------------------------------ | ------ |
| **GET**     | `/mechanics`                   | Not implemented (`501`) |
| **GET**     | `/mechanics/{id}`              | Not implemented (`501`) |
| **PUT**     | `/mechanics/{id}`              | Not implemented (`501`) |
| **GET**     | `/mechanics/{id}/schedule`     | Not implemented (`501`) |
| **PUT**     | `/mechanics/{id}/schedule`     | Not implemented (`501`) |
| **GET**     | `/mechanics/{id}/jobs`         | Not implemented (`501`) |
| **GET**     | `/mechanics/{id}/appointments` | Not implemented (`501`) |
| **GET**     | `/mechanics/{id}/reviews`      | Not implemented (`501`) |


---

### 5. Appointments (`Appointments.cpp`)

*Scheduling logic (`/appointments/…` routes only).*


| HTTP Method | Endpoint                    | Status |
| ----------- | --------------------------- | ------ |
| **POST**    | `/appointments`             | Implemented |
| **GET**     | `/appointments/{id}`        | Implemented |
| **PATCH**   | `/appointments/{id}/status` | Not implemented (`501`) |
| **POST**    | `/appointments/{id}/job`    | Not implemented (`501`) |


---

### 6. Jobs (`Jobs.cpp`)

*Active work tracking (`/jobs/…` routes only).*


| HTTP Method | Endpoint              | Status |
| ----------- | --------------------- | ------ |
| **GET**     | `/jobs/{id}`          | Not implemented (`501`) |
| **PUT**     | `/jobs/{id}/stage`    | Not implemented (`501`) |
| **POST**    | `/jobs/{id}/complete` | Not implemented (`501`) |


---

### 7. Reviews (`Reviews.cpp`)

*Direct review access (`/reviews/…` routes only).*


| HTTP Method | Endpoint        | Status |
| ----------- | --------------- | ------ |
| **POST**    | `/reviews`      | Not implemented (`501`) |
| **DELETE**  | `/reviews/{id}` | Not implemented (`501`) |


