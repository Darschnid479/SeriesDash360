# SeriesDashXEX

SeriesDashXEX is the repository's clean-room, SDK-free XEX2 packer.

It converts an already linked Xbox 360 PowerPC PE into an **unsigned homebrew XEX2** for an Xbox 360 that is already running a compatible patched/homebrew host.

## What it does

- validates `MZ/PE` and Xbox 360 `POWERPCBE (0x01F2)`
- requires Xbox PE subsystem `0x000E`
- maps PE sections to their RVAs
- converts IAT entries to Xbox XEX import form
- creates XEX2 optional headers
- builds import-library tables for `xboxkrnl.exe`, `xam.xex` and `xbdm.xex`
- creates page descriptors and chained SHA-1 page hashes
- creates the XEX2 security-information block
- creates the XEX header digest
- writes an unencrypted/basic-compression XEX2 basefile
- includes a structural `verify` command

## What it deliberately does not do

- Microsoft/retail signing
- retail security bypass
- DRM bypass
- encrypted retail XEX creation
- kernel/hypervisor patching

The RSA signature field is left blank because this output is for an environment that is already configured to run homebrew.

## Usage

```bat
BUILD_XBOX360_NO_SDK.bat path\to\SeriesDash360.exe
```

Or directly:

```text
python tools/SeriesDashXEX/seriesdashxex.py pack input.exe default.xex
python tools/SeriesDashXEX/seriesdashxex.py verify default.xex
```

## Important compiler note

SeriesDashXEX replaces **the XEX packager**, not the compiler/linker.

A compatible Xbox 360 PE still has to be produced by an open toolchain such as OpenXeChain/FreeChainXenon. The current SeriesDash360 native runtime still contains APIs that need additional open-toolchain compatibility work before the complete dashboard can be compiled without the XDK.
