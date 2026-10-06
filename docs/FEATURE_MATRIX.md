# Feature matrix — SeriesDash360 1.0 RC

| Feature | Native Xbox 360 status | Notes |
|---|---|---|
| Series-style Home | ✅ Implemented | Direct3D9 1280×720 |
| Cover library | ✅ Implemented | JPEG cache + fallback tiles |
| Native search | ✅ Implemented | Xbox on-screen keyboard |
| Favorites | ✅ Implemented | Persistent state.ini |
| Recently played | ✅ Implemented | Persistent + sorted by launch time |
| Categories / filters | ✅ Implemented | All, Games, Homebrew, Emulators, Apps, Favorites, Recent |
| Native file manager | ✅ Implemented | Browse/copy/move/delete/mkdir |
| Themes / skins | ✅ Implemented | Built-ins + external .theme files |
| System info | ✅ Implemented | CPU/GPU/eDRAM/board temperatures |
| Launch XEX/homebrew | ✅ Implemented | XLaunchNewImage |
| FTP server | ✅ Implemented | Passive LIST/RETR/STOR and file operations |
| Cover download | ✅ Implemented | XboxUnity/XHTTP + background queue |
| Disc → HDD | ✅ Implemented | Mounted Dvd:\ file tree |
| Title updates | ✅ Implemented | Local installed-update scan and enable/disable |
| Achievements | ✅ Implemented | Read-only XUser enumeration |
| Save browser | ✅ Implemented | Read-only Title ID/profile discovery |
| Screenshot capture | ✅ Implemented | PNG to screenshots folder |
| Native plugin loader | ✅ Implemented | .xex discovery + XexLoadImage |
| Native script runtime | ✅ Implemented | .sd360 command scripts |
| System-Link LAN | ✅ Implemented | UDP peer/title discovery |
| External LiNK relay/tunnel | ➖ Not bundled | Separate external service/protocol |
| Metadata editor | ✅ Implemented | Title/category/dev/genre/year/description |
| Background cover queue | ✅ Implemented | Worker thread + de-duplication |
| Hardware-tested default.xex | ❌ Pending | Requires XDK build + console smoke test |
