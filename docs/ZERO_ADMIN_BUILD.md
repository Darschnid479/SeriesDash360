# Zero-admin SDK-free build

Use:

```bat
BUILD_XBOX360_ZERO_ADMIN.bat
```

This path is intended for locked-down Windows PCs where administrator access and WSL are unavailable.

It keeps everything inside the repository:

- `.tools/msys64` — portable MSYS2 Windows environment
- `.openxechain` — OpenXeChain sources and locally built toolchain
- `build-open` — Xbox PE/XEX output

It does **not** install to Program Files, change the registry, permanently change PATH, use WSL, Docker, or the Microsoft XDK.

The BAT downloads the current portable MSYS2 self-extracting archive from the official MSYS2 GitHub release, installs build packages into that local MSYS2 directory, builds OpenXeChain locally, builds the current SeriesDash360 OpenXeChain target, and runs SeriesDashXEX.

## Current project status

The zero-admin build infrastructure is separate from feature parity. The current `platform/openxechain` target is still the SDK-free bootstrap runtime while the full dashboard feature set is being ported from `platform/xbox360`.
