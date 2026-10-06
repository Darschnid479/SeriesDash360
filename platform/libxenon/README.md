# SeriesDash360 LibXenon target

This directory is the **LibXenon / XeLL Reloaded** target for SeriesDash360.

It intentionally does not use OpenXeChain or the Microsoft Xbox 360 XDK.

## Output

The build produces:

```
SeriesDash360.elf32
```

This is a LibXenon bare-metal executable intended to be launched by **XeLL Reloaded**. It is **not** a retail-signed XEX and it is not interchangeable with `default.xex`.

## Fast build with Docker

Free60Project documents a prebuilt Docker image named:

```
free60/libxenon:latest
```

From the repository root on Windows, run:

```
BUILD_XBOX360_LIBXENON_DOCKER.bat
```

The script:

1. checks Docker,
2. downloads the prebuilt LibXenon image only when needed,
3. mounts the repository into the container,
4. builds only the SeriesDash360 LibXenon target,
5. copies the final ELF to `build-libxenon`.

Final files:

```
build-libxenon/SeriesDash360.elf32
build-libxenon/xenon.elf
```

The `xenon.elf` copy is provided for convenient FAT32 USB/XeLL use.

## Current port status

The first target is deliberately small and verifiable:

- Xenos video initialization
- framebuffer console
- USB initialization
- Xbox 360 controller polling
- simple Home / Library / System navigation
- A / B navigation
- Y / Guide exit

The feature-rich XDK/XEX dashboard implementation is **not yet 1:1 ported** to LibXenon. APIs and runtime assumptions are different, so those features need to be migrated individually rather than copied blindly.
