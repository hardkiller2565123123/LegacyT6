# T6M Source Restoration

<div align="center">

# 🎮 T6M Restoration Project

### *Preserving a piece of Black Ops II client history*

![Progress](https://img.shields.io/badge/Restoration-65%25-orange?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows-blue?style=for-the-badge)
![Architecture](https://img.shields.io/badge/Build-x86-red?style=for-the-badge)
![Status](https://img.shields.io/badge/Current-MP_ZM_Menus-success?style=for-the-badge)

</div>

---

## 📖 About

This repository focuses on the restoration, preservation, and compatibility repair of the legacy **T6M Black Ops II client source code**.

The goal is to preserve historical BO2 client development while restoring buildability, runtime stability, and offline menu functionality on modern Windows systems.

This project is intended strictly for:

- Software preservation
- Reverse engineering research
- Educational purposes
- Offline experimentation
- Historical restoration work

---

## 🛠 Current Restoration Progress

```text
██████████████████████████░░░░░░░░░ 65%
```

---

## ⚙ Core Systems Status

| System | Status | Notes |
|---|---|---|
| Project Structure | ✅ Restored | Original structure mostly rebuilt |
| VS2013 Solution | ✅ Working | Opens and compiles correctly |
| x86 Environment | ✅ Working | Legacy toolchain restored |
| Multiplayer Main Menu | ✅ Working | MP V44 reaches menu safely |
| Zombies Main Menu | ✅ Working | ZM V42 reaches menu safely |
| Zombies System Link Menu | ✅ Working | System Link menu enabled through dvar forcing |
| MP System Link Research | ⚠ In Progress | V44 dvar pointer work started |
| Steam Stub | ⚠ Partial | Core exports and interfaces restored |
| PM_SendMessage Export | ✅ Fixed | Missing export restored |
| V44 Boot Stability | ✅ Stable | Safe boot flow now working |
| V42 ZM Boot Stability | ✅ Stable | Safe boot flow now working |
| Runtime Debug Console | ✅ Working | Colored labels and cleaner output added |
| Debug Log Filtering | ✅ Working | Duplicate spam suppression added |
| GSC Stub Generation | ⚠ Partial | Missing ZM score functions being stubbed |
| DW Hook Framework | ⚠ Partial | Hooks install and initialize |
| Winsock Import Recovery | ✅ Working | V44 import addresses recovered via IDA |
| Auth Packet Handling | ⚠ Partial | Packet handling/debugging active |
| DemonWare Emulation | ⚠ In Progress | Basic framework active, backend incomplete |
| Matchmaking | ❌ Offline | No functional backend yet |
| Party System | ❌ Offline | Not functional |
| Server Browser | ❌ Offline | Not functional |
| Crash Stabilization | ⚠ In Progress | Active debugging and isolation |

---

# 🧩 Current State

## ✅ Working

- Multiplayer V44 launches safely
- Zombies V42 launches safely
- Zombies System Link menu is visible
- Dvar pointer forcing works after frontend initialization
- Runtime debug console works
- Cleaner colored console logging added
- Duplicate log spam reduced
- Steam API export restoration working
- PM_SendMessage export restored
- V44 Winsock imports recovered and mapped
- Basic DemonWare packet interception operational
- Multiple startup crash sources isolated
- Legacy project structure preserved

---

## ⚠ In Progress

- MP V44 System Link enablement
- Zombies script error cleanup
- GSC stub loading paths
- DemonWare login flow
- Packet response emulation
- Online authentication handling
- DW socket communication debugging
- V44 hook verification/testing

---

## ❌ Missing / Incomplete

- Full DemonWare backend
- Matchmaking systems
- Party handling
- Full Steam emulation
- Network authentication
- Server browser support
- Online profile/stat syncing

---

# 🧱 Build Requirements

| Requirement | Version |
|---|---|
| Visual Studio | 2013 Update 5 |
| Platform | x86 |
| Windows SDK | 8.1 |
| DirectX SDK | June 2010 |

---

# 📦 Required Notes

These projects were originally developed around:

- Older MSVC compiler behavior
- Legacy Steam APIs
- Deprecated Windows SDKs
- Pre-modern C++ standards
- Older dependency chains

Modern Visual Studio versions may require:

- Toolset retargeting
- Manual include/lib setup
- Legacy runtime installation
- Disabled warning promotion

Avoid fully modernizing the project unless necessary.

---

# 🔗 Visual Studio 2013 Download

Visual Studio Community 2013 with Update 5 Archive

---

# 🙏 Credits

Original source code and projects belong to their respective developers and communities.

Special credit to:

- T6M developers
- FourDeltaOne developers
- Original BO2 reverse engineering communities
- Multiplayer preservation contributors
- Community researchers documenting BO2 internals

---

# ⚠ Disclaimer

This repository does NOT include:

- Game assets
- Commercial BO2 files
- Proprietary Activision content
- Copyrighted binaries

Users are responsible for owning legitimate copies of all required software.

This repository exists solely for archival, educational, and restoration purposes.
