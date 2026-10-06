# OpenXeChain target

This folder is the **SDK-free compatibility target** for SeriesDash360.

The normal native runtime under `platform/xbox360` was written against XDK-style APIs. This target proves and maintains a fully open build chain:

```text
SeriesDash360 source
      ↓
OpenXeChain clang / lld
      ↓
Xbox POWERPCBE PE
      ↓
SeriesDashXEX
      ↓
unsigned homebrew default.xex
```

## Current compatibility runtime

The current open target verifies these real Xbox OS paths without Microsoft SDK files:

- `xam.xex` imports through xecorelib
- `xboxkrnl.exe` imports through xecorelib
- controller polling through `XamInputGetState`
- SMC CPU/GPU/eDRAM/board temperature request
- Xbox notification UI
- return/terminate through `XamLoaderTerminateTitle`
- Xbox PE linking through OpenXeChain
- XEX2 creation through SeriesDashXEX

This is currently a **compatibility/bootstrap runtime**, not yet the full Direct3D Series-style dashboard. The full native source remains in `platform/xbox360` while those XDK-facing APIs are being replaced with open equivalents.

Run `BUILD_XBOX360_NO_SDK.bat` from Windows. No prebuilt PE argument is required.
