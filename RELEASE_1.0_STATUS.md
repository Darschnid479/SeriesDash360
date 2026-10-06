# SeriesDash360 1.0 release candidate

The native feature set requested for the 1.0 dashboard is now implemented in source on `release/1.0`.

## Implemented native features

- Direct3D9 1280×720 Series-style dashboard
- XInput navigation
- HDD/USB library scanning and XEX launch
- XEX Title ID parsing
- native search keyboard
- persistent favorites
- persistent recently played
- All/Games/Homebrew/Emulators/Apps/Favorites/Recent filters
- controller-driven file manager
- themes and external .theme skins
- local Title Update manager
- read-only achievement browser
- read-only save-game browser
- PNG screenshots
- native .xex plugin loader
- native .sd360 script runtime
- LAN System-Link peer/title discovery
- full local metadata editor
- asynchronous background cover queue
- XboxUnity/XHTTP cover lookup and cache
- disc-to-HDD copy from mounted Dvd:\
- CPU/GPU/eDRAM/board telemetry
- FTP with passive upload/download and file operations

## LiNK note

SeriesDash360 includes LAN/System-Link discovery. It does not bundle an external LiNK relay/tunneling provider or Xbox Live bypass.

## Final binary gate

The remaining blocker is **verification, not feature source**:

1. Build with the Xbox 360 XDK using `BUILD_XBOX360.bat`.
2. Pass the included XEX2/title-module verifier.
3. Produce `build-xbox360/default.xex`.
4. Smoke-test the XEX on an Xbox 360 running the compatible homebrew host.
5. Fix any hardware/XDK-only compile or runtime issues discovered there.

The current authoring environment does not contain the proprietary XDK or `imagexex.exe`, so no hardware-tested XEX is claimed yet.
