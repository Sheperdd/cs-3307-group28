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

The following packages must be installed before building. Everything listed under **Auto-fetched** below is downloaded automatically by CMake — no manual install needed for those.

### Auto-fetched via CMake FetchContent (no action required)

| Library | Version | Purpose |
| ------- | ------- | ------- |
| [SQLiteCpp](https://github.com/SRombauts/SQLiteCpp) | 3.3.3 | C++ SQLite wrapper |
| [nlohmann/json](https://github.com/nlohmann/json) | v3.12.0 | JSON serialization |
| [GoogleTest](https://github.com/google/googletest) | v1.17.0 | Unit testing |
| [jwt-cpp](https://github.com/Thalhammer/jwt-cpp) | v0.7.0 | JWT generation/validation |

### Must be installed manually

| Package | Component | Used by |
| ------- | --------- | ------- |
| C++20 toolchain (GCC ≥ 11 or Clang ≥ 13) | — | Everything |
| CMake ≥ 3.20 | — | Build system |
| Ninja | — | Build generator |
| Git | — | FetchContent |
| OpenSSL (dev headers + libs) | libssl / libcrypto | jwt-cpp |
| Qt 5 | Core, Widgets, Network | Client (`TorqueClient`) |
| Boost | system | Server (`TorqueServer`) |

---

### macOS

1. Install Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```
2. Install all required packages via Homebrew:
   ```bash
   brew install cmake ninja openssl qt@5 boost
   ```
3. If CMake cannot find Qt5, add it to your path:
   ```bash
   export PATH="$(brew --prefix qt@5)/bin:$PATH"
   ```

### Linux (native or WSL)

Install all required packages via your distro package manager.

**Ubuntu / Debian / WSL:**
```bash
sudo apt update && sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  libssl-dev \
  qtbase5-dev \
  libboost-system-dev
```

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

After building, you can run all tests at once via CTest:

```bash
ctest --test-dir build --output-on-failure
```

Or run each test suite individually:

| Suite | Source file | Command |
| ----- | ----------- | ------- |
| Engine tests | `test_engine.cpp` | `./build/tests/UnitTests` |
| Customer service tests | `test_customer_service.cpp` | `./build/tests/CustomerServiceTests` |
| Mechanic service tests | `test_mechanic_service.cpp` | `./build/tests/MechanicServiceTests` |
| Auth service tests | `test_auth_service.cpp` | `./build/tests/AuthServiceTests` |

---

## Extra Working Notes

- The database is always accessed through DatabaseManager, but since SQLite is blocking (can only do one call at a time), every DB call is offloaded to a net::thread_pool via net::co_spawn when working on the server.

---

## Database Schema

This section describes the physical SQLite schema (`DatabaseManager`), not response DTOs.

### Physical tables

- `customers`
  - `id` (PK), `name`, `phone`, `email` (UNIQUE), `password`, `role`, `createdAt`
- `mechanics`
  - `id` (PK), `userId` (UNIQUE, FK -> `customers.id`), `displayName`, `shopName`, `hourlyRate`, `specialties`
- `vehicles`
  - `id` (PK), `ownerUserId` (FK -> `customers.id`), `vin` (UNIQUE), `make`, `model`, `year`, `mileage`, `createdAt`
- `symptom_forms`
  - `id` (PK), `customerId` (FK -> `customers.id`), `vehicleId` (FK -> `vehicles.id`), `description`, `severity`, `createdAt`
- `mechanic_availability` (not really used)
  - `id` (PK), `mechanicId` (FK -> `mechanics.id`), `start`, `end`, UNIQUE(`mechanicId`,`start`,`end`)
- `appointments`
  - `id` (PK), `customerId` (FK -> `customers.id`), `mechanicId` (FK -> `mechanics.id`), `vehicleId` (FK -> `vehicles.id`), `symptomFormId` (FK -> `symptom_forms.id`), `scheduledAt`, `status`, `note`, `createdAt`
- `jobs`
  - `id` (PK), `appointmentId` (UNIQUE FK -> `appointments.id`), `mechanicId` (FK -> `mechanics.id`), `customerId` (FK -> `customers.id`), `vehicleId` (FK -> `vehicles.id`), `stage`, `percentComplete`, `lastNote`, `updatedAt`, `startedAt`, `completedAt`, `completionNote`
- `reviews`
  - `id` (PK), `jobId` (UNIQUE FK -> `jobs.id`), `customerId` (FK -> `customers.id`), `mechanicId` (FK -> `mechanics.id`), `rating`, `comment`, `createdAt`

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

| HTTP Method | Endpoint                       | Status | Tested |
| ----------- | ------------------------------ | ------ | ------ |
| **POST**    | `/auth/register`               | Not implemented (`501`) | null |
| **POST**    | `/auth/login`                  | Not implemented (`501`) | null |
| **GET**     | `/users/{id}`                  | Implemented | null |
| **PATCH**   | `/users/{id}`                  | Implemented | null |
| **DELETE**  | `/users/{id}`                  | Implemented | null |
| **PATCH**   | `/users/{id}/password`         | Implemented | null |
| **GET**     | `/users/{userId}/vehicles`     | Implemented | null |
| **POST**    | `/users/{userId}/vehicles`     | Implemented | null |
| **GET**     | `/users/{userId}/symptoms`     | Implemented | null |
| **GET**     | `/users/{userId}/appointments` | Implemented | null |
| **GET**     | `/users/{userId}/reviews`      | Implemented | null |


---

### 2. Vehicles (`Vehicles.cpp`)

*CRUD for vehicles and any `/vehicles/{id}/…` nested routes.*


| HTTP Method | Endpoint                         | Status | Tested |
| ----------- | -------------------------------- | ------ | ------ |
| **GET**     | `/vehicles/{id}`                 | Implemented | null |
| **PATCH**   | `/vehicles/{id}`                 | Implemented | null |
| **DELETE**  | `/vehicles/{id}`                 | Implemented | null |
| **POST**    | `/vehicles/{vehicleId}/symptoms` | Implemented | null |


---

### 3. Symptom Forms (`Symptoms.cpp`)

*Direct symptom-form access (`/symptoms/…` routes only).*


| HTTP Method | Endpoint         | Status | Tested |
| ----------- | ---------------- | ------ | ------ |
| **GET**     | `/symptoms/{id}` | Implemented | null |
| **PATCH**   | `/symptoms/{id}` | Implemented | null |
| **DELETE**  | `/symptoms/{id}` | Implemented | null |


---

### 4. Mechanics (`Mechanics.cpp`)

*Search, profile management, and any `/mechanics/{id}/…` nested routes.*


| HTTP Method | Endpoint                       | Status | Tested |
| ----------- | ------------------------------ | ------ | ------ |
| **GET**     | `/mechanics`                   | Implemented | null |
| **GET**     | `/mechanics/{id}`              | Implemented | null |
| **PATCH**   | `/mechanics/{id}`              | Implemented | null |
| **GET**     | `/mechanics/{id}/jobs`         | Implemented | null |
| **GET**     | `/mechanics/{id}/appointments` | Implemented | null |
| **GET**     | `/mechanics/{id}/reviews`      | Implemented | null |


---

### 5. Appointments (`Appointments.cpp`)

*Scheduling logic (`/appointments/…` routes only).*


| HTTP Method | Endpoint                    | Status | Tested |
| ----------- | --------------------------- | ------ | ------ |
| **POST**    | `/appointments`             | Implemented | null |
| **GET**     | `/appointments/{id}`        | Implemented | null |
| **PATCH**   | `/appointments/{id}/status` | Implemented | null |
| **POST**    | `/appointments/{id}/job`    | Implemented | null |


---

### 6. Jobs (`Jobs.cpp`)

*Active work tracking (`/jobs/…` routes only).*


| HTTP Method | Endpoint              | Status | Tested |
| ----------- | --------------------- | ------ | ------ |
| **GET**     | `/jobs/{id}`          | Implemented | null |
| **PUT**     | `/jobs/{id}/stage`    | Implemented | null |
| **POST**    | `/jobs/{id}/complete` | Implemented | null |
| **GET**     | `/jobs/{id}/notes`    | Implemented | null |
| **POST**    | `/jobs/{id}/notes`    | Implemented | null |


---

### 7. Reviews (`Reviews.cpp`)

*Direct review access (`/reviews/…` routes only).*


| HTTP Method | Endpoint        | Status | Tested |
| ----------- | --------------- | ------ | ------ |
| **POST**    | `/reviews`      | Implemented | null |
| **DELETE**  | `/reviews/{id}` | Implemented | null |


