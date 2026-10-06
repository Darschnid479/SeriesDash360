# Xbox 360 port plan

## Runtime model

1. Trigger your existing BadUpdate setup.
2. Enter a homebrew-capable post-exploit environment (for example a setup based on FreeMyXe/XeUnshackle).
3. Start SeriesDash360 from that environment.
4. SeriesDash360 handles UI, library, metadata, file operations and user settings.
5. Launch requests are handed back to the host environment through `IPlatform::launchTitle()`.

BadUpdate itself is non-persistent, so a cold reboot means the exploit must be triggered again before SeriesDash360 can be used.

## Porting layers

### 1. Renderer
Implement a GPU/2D backend for:
- rounded tiles and panels
- text and icons
- PNG/JPEG textures
- cover cache
- 60 FPS focus animations
- 720p and 1080p safe-area scaling

### 2. Input
Map Xbox 360 controls:
- D-pad / left stick: focus
- A: open / launch
- B: back
- X: contextual action
- Y: details
- LB/RB: switch library group
- View/Back: system overlay
- Menu/Start: settings

### 3. Storage
Expose `Usb0:/`, `Hdd1:/` and other valid mounted volumes through `IPlatform::storageRoots()`.

### 4. System info
Bind temperature, free space and network address to `systemStats()` where the host exposes them.

### 5. Launching
Bind `launchTitle()` only to the public/homebrew launch mechanism offered by the already-running host environment. Keep exploit, kernel patch, hypervisor patch and signature/DRM bypass code outside this project.

## Recommended packaging

```
SeriesDash360/
  default.xex        # console build, once the target toolchain is chosen
  config/
  assets/
  cache/covers/
  userdata/
  plugins/
```
