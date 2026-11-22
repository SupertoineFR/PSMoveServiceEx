# Building PSMoveServiceEx from source (Windows x64)

## ⚙️ Prerequisite Checklist
* **Git** ≥ 2.23 (needed for sub‑module support)
  5 | * **Visual Studio 2026** (Community or Build Tools) with the **C++ workload** – the helper scripts now reference the 2026 MSBuild and vcvarsall paths.
* **CMake** 4.x that ships with VS2026 (`C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`)
  7 | * **Boost** 1.92 pre‑built for MSVC 14.5 (`C:\local\boost_1_92_0` – ensure `BOOST_ROOT` points here)
* Internet access – OpenCV 3.4.20 is cloned during the first build
* Optional: **Git‑Bash** or **WSL** for the `rm -rf` commands in the scripts


 12 | Toolchain this repo is set up for: **Visual Studio 2026 (v145 / MSVC 14.5x), CMake 4.x, Boost 1.92**.
13 | **Note:** The original `build.ps1` bundled with the repo contained syntax errors
14 | (missing braces, incorrect `foreach` syntax, and an unbalanced block), which caused
15 | PowerShell to abort before any compilation could begin.  The section below
16 | provides a corrected, consolidated helper script that works with the current
17 | toolchain.
 13 | The original upstream scripts targeted VS 2015 / Boost 1.6x / OpenCV 3.1; several fixes were required for the modern toolchain. The helper scripts (`InitialSetup_X64.bat`, `GenerateProjectFiles_X64.bat`, `build.ps1`) have been updated to use VS 2026 paths.

## Prerequisites
 17 | - Visual Studio 2026 (Community or Build Tools) with the C++ workload.
 18 | - Use the CMake that ships with VS:
   `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`
   The `cmake` on the Git Bash/MSYS PATH has no Visual Studio generators — don't use it.
 19 | - Boost 1.92 prebuilt for msvc-14.5 at `C:\local\boost_1_92_0` (libs in `lib64-msvc-14.5`).
- Boost 1.92 prebuilt for msvc-14.5 at `C:\local\boost_1_92_0` (libs in `lib64-msvc-14.5`).
- Git + network access (OpenCV is cloned at build time).
- Submodules checked out: `git submodule update --init --recursive`
  (warning: this also resets the local submodule patches listed below).

 26 | In the commands below, `CMAKE` = the VS cmake.exe path above and
 27 | `MSBUILD` = `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe`.

## 1. Build third-party dependencies (once)

> **NOTE** – Once the `build_thirdparty.bat` finishes, you can skip it on
> subsequent builds (unless you change the Boost or Protobuf sources).
> The dependency libraries live under `thirdparty/<lib>/x64/<Debug|Release>/`. They are
> only rebuilt if you delete the `x64` directory or if the source changes.

 36 | Equivalent of `InitialSetup_X64.bat`, adapted for VS 2026. Run from the repo root.

 38 | **protobuf** (Debug + Release; FindProtobuf expects copies under `x64\<Config>`):
```sh
cd thirdparty/protobuf && rm -rf vsprojects && mkdir vsprojects && cd vsprojects
"$CMAKE" -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -Dprotobuf_DEBUG_POSTFIX="" -Dprotobuf_BUILD_TESTS=OFF ../cmake
for c in Debug Release; do "$CMAKE" --build . --config $c -- -m; mkdir -p x64/$c; cp $c/* x64/$c/; done
```

 46 | **libusb** (Debug + Release):
```sh
 48 | cd thirdparty/libusb/msvc
 49 | for c in Debug Release; do "$MSBUILD" libusb_2015.sln -p:Configuration=$c -p:Platform=x64 \
 50 |   -p:PlatformToolset=v145 -p:WindowsTargetPlatformVersion=10.0 -m; done
```

 54 | **SDL2** (static, Release only):
```sh
cd thirdparty/SDL2 && rm -rf build && mkdir build && cd build
"$CMAKE" .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DDIRECTX=OFF -DSDL_STATIC=ON -DFORCE_STATIC_VCRT=ON \
  "-DEXTRA_CFLAGS=-MT -Z7 -DSDL_MAIN_HANDLED -DWIN32 -DNDEBUG -D_CRT_SECURE_NO_WARNINGS -DHAVE_LIBC -D_USE_MATH_DEFINES"
 60 | "$CMAKE" --build . --config Release --target SDL2main -- -m
"$CMAKE" --build . --config Release --target SDL2main -- -m
```

## 2. Configure and build PSMoveService

### Build the existing VS 2026 configuration from PowerShell

From the repository root, build only the service target with:

```powershell
$cmake = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake --build build-vs2026 --config Release --target PSMoveService --parallel 8
```

The executable is written to `build-vs2026\src\psmoveservice\Release\PSMoveService.exe`.
To build every configured project instead, omit `--target PSMoveService`.

## Consolidated `build.ps1` helper

You can replace the two separate PowerShell snippets with a single script that
optionally builds the third‑party dependencies, configures the main project, and
 83 | ```powershell
 84 | # build.ps1 (consolidated helper)
 85 | # -------------------------------
 86 | # This script replaces the older, broken build.ps1 that had syntax errors
 87 | # (missing braces around foreach loops, incorrect foreach syntax, etc.).
 88 | # The current version performs three optional steps:
 89 | #   1️⃣  Build third‑party dependencies (protobuf, libusb, SDL2, OpenCV)
 90 | #   2️⃣  Configure the main project with CMake
 91 | #   3️⃣  Build the requested target(s)
 92 | # -----------------------------------------------------------------
 93 | param(
 94 |     [switch]$Deps,
 95 |     [string]$Target = "PSMoveService",   # default to just the service
 96 |     [int]$Jobs = 8
 97 | )
 98 | 
 99 | $cmake = "C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe"
100 | 
101 | # 1️⃣  Build third‑party deps if requested
102 | if ($Deps) {
103 |     # Each helper script (e.g. build_thirdparty.bat) expects to be run from the repo root.
104 |     & .\build_thirdparty.bat
105 | }
106 | 
107 | # 2️⃣  Configure the main project (creates build/)
108 | & $cmake -S . -B build -G "Visual Studio 18 2026" -A x64 \
109 |     -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DPSM_BUILD_TESTS=OFF \
110 |     -DBOOST_ROOT="C:/local/boost_1_92_0" \
111 |     -DBOOST_LIBRARYDIR="C:/local/boost_1_92_0/lib64-msvc-14.5" \
112 |     -DPROTOBUF_SRC_ROOT_FOLDER=thirdparty/protobuf
113 | 
114 | # 3️⃣  Build the requested target(s)
115 | & $cmake --build build --config Release --target $Target -- -m -v:m -j$Jobs
116 | ```
# Build
& $cmake --build build --config Release --target $Target -- -m -v:m -j$Jobs
```

Run it with:

```powershell
powershell -ExecutionPolicy Bypass -File build.ps1 -Deps   # include deps
powershell -ExecutionPolicy Bypass -File build.ps1 -Target PSMoveServiceAdmin   # build a different target
```

The initial configure command below uses `build`; the PowerShell command above uses the separate `build-vs2026` directory so it can coexist with an older or differently configured `build` directory.

```sh
"$CMAKE" -S . -B build -G "Visual Studio 18 2026" -A x64 \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DPSM_BUILD_TESTS=OFF \
  -DBOOST_ROOT="C:/local/boost_1_92_0" \
  -DBOOST_LIBRARYDIR="C:/local/boost_1_92_0/lib64-msvc-14.5" \
  -DPROTOBUF_SRC_ROOT_FOLDER=thirdparty/protobuf
"$CMAKE" --build build --config Release -- -m -v:m > build_release.log 2>&1
```
Alternatively: `SetBuildVars_x64.bat` + `GenerateProjectFiles_X64.bat` (both updated for this toolchain), then build `build\PSMoveService.sln`.

The first build clones and compiles OpenCV 3.4.20 via ExternalProject into `deps/` (slow; run in background). Installed libs land in `deps/local/staticlib`.

Outputs:
- `build/src/psmoveservice/Release/PSMoveService.exe`, `PSMoveServiceAdmin.exe`
- `build/src/psmoveconfigtool/Release/PSMoveConfigTool.exe`
- `build/src/psmoveclient/Release/PSMoveClient_CAPI.dll`

Find errors quickly: `grep -E "error [A-Z]+[0-9]+" build_release.log | sort -u`

## Troubleshooting
 140 | | Symptom | Likely Cause | Fix |
 141 | |---------|--------------|-----|
 142 | | `CMake Error: The following variables are required but not set: BOOST_ROOT` | Wrong Boost path | Update `BOOST_ROOT` in the `build.ps1` script or set the environment variable `BOOST_ROOT`.
 143 | | `error C1083: cannot open include file: 'boost/...` | Wrong library directory | Verify `BOOST_LIBRARYDIR` points to `C:\local\boost_1_92_0\lib64-msvc-14.5`.
 144 | | `error LNK2005: duplicate symbol` | Mixing debug/release libs | Ensure you build all components in the same configuration (Debug or Release).
 145 | | `error 125: The process cannot access the file because it is being used by another process` | Previous build still running | Close the VS instance or kill the lingering `msbuild.exe` process.
| `error 125: The process cannot access the file because it is being used by another process` | Previous build still running | Close the VS instance or kill the lingering `msbuild.exe` process.

## Compatibility notes / gotchas

- **OpenCV is pinned to 3.4.20** (`PSM_OPENCV_GIT_TAG` / `PSM_OPENCV_LIB_SUFFIX` in `cmake/ThirdParty.cmake`). 3.1.0 can't configure under CMake 4; 4.x removed the C API (`IplImage`, `CV_CAP_PROP_*`) the code still uses. `dnn`/protobuf/ITT/quirc are disabled. Changing the version requires updating the lib suffix.
- If OpenCV options change, delete `deps/opencv/src/opencv-build`, `deps/opencv/src/opencv-stamp` and `deps/local` to force a clean rebuild.
- **Boost ≥ 1.87** removed deprecated Asio (`io_service`, `resolver::iterator`, `resolver::query`, `mutable_buffers_1`) — use `io_context`, `results_type::iterator`, `mutable_buffer`. Boost.System is header-only (no `system` component in `find_package`). Boost.Filesystem v4: `branch_path()` → `parent_path()`.
- `cmake/Version.cmake` parses `PSM_RELEASE_VERSION_{MAJOR,MINOR,BUILD,REVISION}` from `src/psmoveprotocol/ProtocolVersion.h`.
- `src/tests` is stale (references removed Kalman filter sources): keep `PSM_BUILD_TESTS=OFF`.
- Modern MSVC no longer pulls in `<chrono>`/`<string>` transitively — include them explicitly.
- `ImGui::ButtonChecked` is provided by `src/psmoveconfigtool/ImGuiButtonChecked.h` (not in the imgui submodule).

### Local submodule patches (not tracked by the main repo)

These live inside submodules and are lost on `git submodule update`; re-apply if needed:
- `thirdparty/protobuf/src/google/protobuf/stubs/hash.h`: MSVC `hash_compare` branch limited to `_MSC_VER < 1600`.
- `thirdparty/protobuf/src/google/protobuf/compiler/java/java_file.cc`: `FieldDescriptorCompare::operator()` made `const`.
- `thirdparty/PS3EYEDriver/src/ps3eye.{h,cpp}`: `bool getFrame(uint8_t* frame, bool wait = true)` (non-blocking when `wait=false`, 1 s bounded wait otherwise).
- `thirdparty/libstem_gamepad/source/gamepad/*`: `Gamepad_init(bool xinputOnly)` (skips DirectInput enumeration when true and XInput is available).
- `thirdparty/Boost.Application/include/boost/application/signal_binder.hpp` and `example/setup/windows/setup/service_setup.hpp`: `io_context` / `parent_path` ports.
