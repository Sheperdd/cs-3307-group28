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
