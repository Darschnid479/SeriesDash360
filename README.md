# SeriesDash360

> Xbox Series-inspired dashboard/front-end for Xbox 360 homebrew environments.

![SeriesDash360 home](assets/screenshots/home.svg)

SeriesDash360 is an **unofficial community project** focused on creating a modern, controller-first Xbox 360 dashboard with a Series-style interface and a feature set inspired by dashboards such as Aurora.

## Version

**Current branch:** `release/1.0`  
**Status:** **1.0 release candidate — source complete enough for XDK build testing, but not yet hardware-verified.**

A real `default.xex` must be built with the Xbox 360 XDK/`imagexex` pipeline and smoke-tested on hardware before this branch should be tagged as a final binary release.

---

# Native Xbox 360 features implemented

## Xbox Series-style native UI

- Native Xbox 360 Direct3D 9 renderer
- 1280×720 dashboard layout
- Xbox Series-inspired dark interface
- Cover-art game tiles
- Selected-tile highlight/focus
- Library page
- System-information page
- Quick-action tiles
- Local fallback artwork when a cover is unavailable
- Controller-first navigation

## Xbox 360 controller support

- Native XInput backend
- Xbox 360 controller polling
- D-pad navigation
- **A** — launch selected title
- **Y** — download/refresh cover art
- **X** — copy mounted game disc to HDD
- **START** — open/close system page
- **BACK** — leave SeriesDash360 and return to the host dashboard

## Game and homebrew library

- Recursive game scanning
- Scans common locations including:
  - `Hdd1:\Games`
  - `Hdd1:\Homebrew`
  - `Usb0:\Games`
  - `Usb0:\Homebrew`
  - `Usb1:\Games`
- Automatic discovery of `default.xex`
- XEX2 validation
- XEX Execution ID parsing
- Xbox 360 **Title ID extraction**
- Game title fallback based on folder names
- Direct title launching through `XLaunchNewImage`
- HDD and USB library support

## Cover art and metadata

- Title-ID-based cover identification
- On-console JPEG cover cache
- Cache location:
  - `Hdd1:\SeriesDash360\cache\covers\`
- XboxUnity metadata/cover lookup integration
- XHTTP download backend
- Manual cover refresh with the **Y** button
- Local fallback tile when no valid cover exists
- Portable CoverService implementation retained for desktop/testing builds

> Cover downloading depends on the external metadata provider being reachable and returning compatible metadata.

## Disc → HDD installation

- Detects mounted Xbox disc through `Dvd:\`
- Recursive disc directory traversal
- Recreates directory structure on the destination drive
- Copies exposed files from the mounted disc to HDD
- Dashboard shortcut using the **X** button
- Default current destination:
  - `Hdd1:\Games\DiscImport`
- Portable copy-progress implementation is also included in the desktop/core layer

SeriesDash360 only copies files already exposed by the running console environment. It does not include disc decryption, DRM bypass or security-patch code.

## System telemetry

Native SMC telemetry support for:

- CPU temperature
- GPU temperature
- eDRAM temperature
- motherboard/system temperature

The system page displays the values directly on the dashboard.

## Networking

- Xbox network initialization through XNet
- Winsock initialization
- XHTTP client
- HTTP metadata requests
- Cover-image downloads

## Built-in FTP server

Default FTP port:

`7564`

Implemented commands:

- `USER`
- `PASS`
- `SYST`
- `TYPE`
- `NOOP`
- `PWD`
- `CWD`
- `PASV`
- `LIST`
- `NLST`
- `RETR`
- `STOR`
- `DELE`
- `MKD`
- `RMD`
- `QUIT`

FTP functionality includes:

- passive data connections
- directory listings
- file downloads from the Xbox
- file uploads to the Xbox
- file deletion
- directory creation/removal
- directory navigation

## File-management core

The portable/core implementation also supports:

- directory listing
- copy
- move
- delete
- recursive delete
- create directory
- file sizes
- directory/file sorting

The native Xbox 360 build now also includes a controller-driven file-manager screen with directory navigation, copy, move, delete and folder creation.

## Native search, state and filters

- On-screen Xbox keyboard search through `XShowKeyboardUI`
- persistent favorites in `Hdd1:\SeriesDash360\userdata\state.ini`
- persistent recently-played timestamps
- recent list sorted by last launch
- native filters for:
  - All
  - Games
  - Homebrew
  - Emulators
  - Apps
  - Favorites
  - Recent
- custom title/category metadata overrides

## Native file manager

- controller-driven file browser
- HDD/USB path navigation
- enter folder / go up
- recursive copy
- move
- recursive delete
- create folder
- file-size display
- copy/move destination entry through the Xbox keyboard

## Themes / skins

- built-in Series Dark theme
- Xbox Green
- Series Blue
- OLED
- external `.theme` files from `Hdd1:\SeriesDash360\themes`
- configurable background/panel/accent/text/muted colors
- optional background-image path

## Local Title Update manager

- scans installed local title-update locations
- per-title update listing
- enable/disable through local file state
- rescan from the dashboard

This manager handles updates already present on storage. It is not a piracy-oriented update downloader.

## Achievements and saves

- read-only achievement enumeration for the signed-in profile/title
- achievement label, Gamerscore and unlocked state
- read-only save-game discovery by profile and Title ID
- save filename/path/size display

## UI reference renders

These images reflect the current native 1.0 source layout. They are **reference renders**, not yet HDMI captures from a hardware-tested `default.xex`.

### Native library

![SeriesDash360 1.0 library](assets/screenshots/home.svg)

### Search + favorites

![Native search and favorites](assets/screenshots/library.svg)

### Tools hub

![SeriesDash360 tools](assets/screenshots/tools.svg)

### Native file manager

![Native file manager](assets/screenshots/file-manager.svg)

### Themes / skins

![Themes and skins](assets/screenshots/themes.svg)

### System / telemetry

![Native system dashboard](assets/screenshots/system.svg)

---

# SDK-free XEX2 packer

SeriesDash360 now includes **SeriesDashXEX**, a clean-room XEX2 packer written for this repository. It does not use the Microsoft XDK or `imagexex.exe`.

```bat
BUILD_XBOX360_NO_SDK.bat path\to\SeriesDash360.exe
```

The input must be an Xbox 360 PowerPC big-endian PE (`Machine 0x01F2`, subsystem `0x000E`). SeriesDashXEX maps the PE, converts imports/IAT entries, builds XEX2 page descriptors and SHA-1 chains, creates the security/header structures, and writes an unsigned homebrew `default.xex`.

**Important:** this replaces the proprietary XEX packaging step only. The complete SeriesDash360 runtime still needs to be compiled/linked into an Xbox-compatible PE first. OpenXeChain/FreeChainXenon are the open toolchain direction for that work; the current native runtime still uses APIs that are not all available in the open compatibility libraries yet.

The output is intended for an already patched/homebrew-capable host and does not implement retail signing or DRM/security bypass.

See `tools/SeriesDashXEX/README.md`.

---

# Building the Xbox 360 XEX

The repository contains an XDK CMake build pipeline derived from the MIT-licensed XexForge approach.

Requirements:

- Xbox 360 XDK
- `XEDK` environment variable
- CMake 3.21 or newer
- Ninja
- Windows build environment

Build:

```bat
set XEDK=C:\Program Files (x86)\Microsoft Xbox 360 SDK
BUILD_XBOX360.bat
```

Expected verified output:

```text
build-xbox360\SeriesDash360.xex
build-xbox360\default.xex
```

The build runs a post-build verification step which checks:

- XEX2 magic
- successful `imagexex /DUMP`
- title-module type

A self-hosted GitHub Actions workflow is also included for a Windows runner with the Xbox 360 XDK installed:

`.github/workflows/xbox360-xex.yml`

---

# BadUpdate / FreeMyXe / XeUnshackle use

SeriesDash360 does **not** contain the exploit itself.

The intended flow is:

```text
BadUpdate
   ↓
FreeMyXe / XeUnshackle / compatible patched host
   ↓
SeriesDash360 default.xex
   ↓
Games / homebrew / dashboard tools
```

SeriesDash360 handles the dashboard/front-end layer after a compatible homebrew-capable Xbox 360 environment already exists.

---

# Repository structure

```text
SeriesDash360/
├─ .github/workflows/
│  └─ xbox360-xex.yml
├─ assets/screenshots/
├─ cmake/
│  ├─ XdkXenon.toolchain.cmake
│  ├─ XdkXex.cmake
│  └─ verify-xex.cmake
├─ config/
├─ docs/
├─ include/sd360/
│  ├─ App.hpp
│  ├─ CoverService.hpp
│  ├─ DiscImporter.hpp
│  ├─ FileManager.hpp
│  ├─ GameLibrary.hpp
│  ├─ Platform.hpp
│  ├─ PluginApi.hpp
│  ├─ Settings.hpp
│  └─ XexMetadata.hpp
├─ platform/
│  ├─ desktop/
│  └─ xbox360/
│     ├─ NativeMain.cpp
│     ├─ NativeNetwork.cpp
│     ├─ NativeRenderer.cpp
│     ├─ NativeRuntime.cpp
│     └─ Application.xml
├─ preview/
├─ src/
├─ BUILD_XBOX360.bat
├─ CMakeLists.txt
├─ CMakePresets.json
├─ RELEASE_1.0_STATUS.md
└─ README.md
```

---

# What still has to happen before final 1.0

The source currently represents a **1.0 release candidate**, not a verified final binary release.

Before a final `v1.0.0` tag should be created:

1. Build `default.xex` using the Xbox 360 XDK.
2. Pass the included XEX2/title-module verifier.
3. Boot it on a BadUpdate/FreeMyXe or XeUnshackle Xbox 360.
4. Verify controller navigation.
5. Verify HDD/USB scanning.
6. Verify title launching.
7. Verify temperature readings.
8. Verify disc copying.
9. Verify cover downloads.
10. Verify FTP upload/download.
11. Fix any hardware-only issues found during testing.
12. Decide whether the remaining Aurora-style items above are required for 1.0 or are moved to 1.1/2.0.

---

# Safety / scope

SeriesDash360 does **not** include:

- BadUpdate exploit code
- signature bypass implementation
- DRM bypass implementation
- hypervisor/kernel patch implementation
- Xbox Live bypasses
- piracy-oriented content downloaders

It is a dashboard/front-end for a console that is already running a compatible homebrew-capable environment.

---

# Disclaimer

SeriesDash360 is not affiliated with, endorsed by, or sponsored by Microsoft, Xbox, Aurora/Team Phoenix, XboxUnity, or any console manufacturer/dashboard project. Xbox and related marks are property of their respective owners.
