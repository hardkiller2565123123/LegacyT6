# T6M Source Restoration

<div align="center">

# 🎮 T6M Restoration Project

### *Preserving a piece of Black Ops II client history*

![Progress](https://img.shields.io/badge/Restoration-55%25-orange?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows-blue?style=for-the-badge)
![Architecture](https://img.shields.io/badge/Build-x86-red?style=for-the-badge)
![Status](https://img.shields.io/badge/Current-Main_Menu_Launching-success?style=for-the-badge)

</div>

---

# 📖 About

This repository focuses on the restoration, preservation, and compatibility repair of the legacy **T6M Black Ops II client source code**.

The goal is to preserve historical BO2 client development while restoring buildability and improving compatibility on modern Windows systems.

This project is intended strictly for:

* Software preservation
* Reverse engineering research
* Educational purposes
* Offline experimentation
* Historical restoration work

# 🛠 Current Restoration Progress

## Overall Progress

```text
████████████████████░░░░░░░░░░░░░ 55%
```

---

# ⚙ Core Systems Status

| System                  | Status          | Notes                                      |
| ----------------------- | --------------- | ------------------------------------------ |
| Project Structure       | ✅ Restored      | Original structure mostly rebuilt          |
| VS2013 Solution         | ✅ Working       | Opens and compiles correctly               |
| x86 Environment         | ✅ Working       | Legacy toolchain restored                  |
| Main Menu Launch        | ✅ Working       | Successfully reaches BO2 menus             |
| Steam Stub              | ⚠ Partial       | Core exports and interfaces restored       |
| PM_SendMessage Export   | ✅ Fixed         | Missing export restored                    |
| Dependency Cleanup      | ⚠ In Progress   | Some legacy libraries still missing        |
| Legacy SDK Support      | ⚠ In Progress   | Requires older SDK setup                   |
| V44 Boot Stability      | ✅ Stable        | Safe boot flow now working                 |
| V44 Console Output      | ✅ Working       | Runtime debug console added                |
| DW Hook Framework       | ⚠ Partial       | Hooks install and initialize               |
| Winsock Import Recovery | ✅ Working       | V44 import addresses recovered via IDA     |
| Auth Packet Handling    | ⚠ Partial       | Packet handling/debugging active           |
| DemonWare Emulation     | ⚠ In Progress   | Basic framework active, backend incomplete |
| Matchmaking             | ❌ Offline       | No functional backend yet                  |
| Party System            | ❌ Offline       | Not functional                             |
| Server Browser          | ❌ Offline       | Not functional                             |
| Authentication          | ⚠ Partial       | Login flow partially intercepted           |
| Crash Stabilization     | ⚠ In Progress   | Active debugging and isolation             |
| Online Menu Connection  | ⚠ Investigating | Infinite “Connecting” loop remains         |

---

# 🧩 Current State

### Working

* Launches directly into multiplayer menus
* Stable V44 boot process
* Core engine modules loading correctly
* Runtime debug console implemented
* Multiple crash sources isolated
* Steam API export restoration working
* V44 Winsock imports recovered and mapped
* Basic DemonWare packet interception operational
* Several linker/compiler/runtime issues repaired
* Legacy project structure preserved

### In Progress

* DemonWare login flow
* Packet response emulation
* Online authentication handling
* DW socket communication debugging
* V44 hook verification/testing

### Missing / Incomplete

* Full DemonWare backend
* Matchmaking systems
* Party handling
* Full Steam emulation
* Network authentication
* Server browser support
* Online profile/stat syncing

---


# 🧱 Build Requirements

## Recommended Setup

| Requirement   | Version       |
| ------------- | ------------- |
| Visual Studio | 2013 Update 5 |
| Platform      | x86           |
| Windows SDK   | 8.1           |
| DirectX SDK   | June 2010     |

---

# 📦 Required Notes

These projects were originally developed around:

* Older MSVC compiler behavior
* Legacy Steam APIs
* Deprecated Windows SDKs
* Pre-modern C++ standards
* Older dependency chains

Modern Visual Studio versions may require:

* Toolset retargeting
* Manual include/lib setup
* Legacy runtime installation
* Disabled warning promotion

Avoid fully modernizing the project unless necessary.

---

# 🔗 Visual Studio 2013 Download

## Official Archive

[https://archive.org/details/en_visual_studio_community_2013_with_update_5_x86_dvd_6816332](https://archive.org/details/en_visual_studio_community_2013_with_update_5_x86_dvd_6816332)

---

# 🙏 Credits

Original source code and projects belong to their respective developers and communities.

Special credit to:

* T6M developers
* FourDeltaOne developers
* Original BO2 reverse engineering communities
* Multiplayer preservation contributors
* Community researchers documenting BO2 internals

---

# ⚠ Disclaimer

This repository does **NOT** include:

* Game assets
* Commercial BO2 files
* Proprietary Activision content
* Copyrighted binaries

Users are responsible for owning legitimate copies of all required software.

This repository exists solely for archival, educational, and restoration purposes.


No copyrighted game assets, proprietary binaries, or commercial content are included.

Users are responsible for owning legitimate copies of any required software or games.
