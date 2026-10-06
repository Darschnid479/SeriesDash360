# SeriesDash360 1.0 release candidate

This branch contains the native Xbox 360 1.0 runtime and the reproducible XEX build path.

## Implemented in the native runtime

- Xbox 360 Direct3D9 1280x720 Series-style UI
- XInput navigation and launch controls
- recursive HDD/USB game scanning for `default.xex`
- XEX execution-ID / Title ID parsing
- launch through `XLaunchNewImage`
- SMC CPU/GPU/eDRAM/board temperatures
- mounted `Dvd:\` file-tree import to HDD
- XboxUnity cover-art lookup + on-console JPEG cache
- XHTTP networking
- FTP control + passive data channel
- FTP LIST/NLST/RETR/STOR/DELE/MKD/RMD/CWD/PWD
- system page and library page
- local cover fallback
- desktop portable core retained for testing

## XEX build

On a Windows machine with the Xbox 360 XDK installed:

1. Set `XEDK` to the XDK root.
2. Install CMake >= 3.21 and Ninja.
3. Run `BUILD_XBOX360.bat`.
4. The build must finish with the XEX2/title-module verifier passing.
5. Output: `build-xbox360/default.xex`.

A binary must not be published as a 1.0 release until this exact build has passed and the XEX has been smoke-tested on hardware.

## Current verification limitation

The ChatGPT build container used to author this branch does not contain the proprietary Xbox 360 XDK or `imagexex.exe`. Therefore the source and build pipeline are present, but no claim is made that a hardware-tested `default.xex` was produced in this environment.
