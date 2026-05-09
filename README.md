# T6M Source Restoration

<div align="center">

# 🎮 T6M Restoration Project

### *Preserving a piece of Black Ops II client history*

![Progress](https://img.shields.io/badge/Restoration-42%25-orange?style=for-the-badge)
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

---

# 🛠 Current Restoration Progress

## Overall Progress

```text
█████████████████░░░░░░░░░░░░░░░░░ 42%
```

---

# ⚙ Core Systems Status

| System              | Status          | Notes                             |
| ------------------- | --------------- | --------------------------------- |
| Project Structure   | ✅ Restored      | Original structure mostly rebuilt |
| VS2013 Solution     | ✅ Working       | Opens and compiles correctly      |
| x86 Environment     | ✅ Working       | Legacy toolchain restored         |
| Main Menu Launch    | ✅ Working       | Successfully reaches BO2 menus    |
| Steam Stub          | ⚠ Partial       | Basic interfaces implemented      |
| Dependency Cleanup  | ⚠ In Progress   | Some legacy libraries missing     |
| Legacy SDK Support  | ⚠ In Progress   | Requires old SDK setup            |
| DemonWare Emulation | ❌ Not Started   | No backend implementation yet     |
| Matchmaking         | ❌ Offline       | Disabled                          |
| Party System        | ❌ Offline       | Not functional                    |
| Server Browser      | ❌ Offline       | Not functional                    |
| Authentication      | ❌ Offline       | Placeholder only                  |
| Crash Stabilization | ⚠ In Progress   | Active debugging                  |
| PM_SendMessage      | ⚠ Investigating | Suspected crash source            |

---

# 🧩 Current State

### Working

* Launches into multiplayer main menu
* Loads core engine modules
* Restores portions of original Steam interfaces
* Several linker/compiler issues repaired
* Legacy project structure preserved

### Missing / Incomplete

* DemonWare backend
* Matchmaking systems
* Party handling
* Full Steam emulation
* Network authentication
* Server browser support

---

# 🐞 Current Crash Investigation

Recent dump analysis indicates:

```cpp
Exception Code: 0xC0000005
Access Violation Executing Location: 0x00000000
```

This typically indicates:

* Null function pointer execution
* Missing virtual function initialization
* Unresolved hook targets
* Incomplete networking callbacks
* Invalid Steam export bindings

### Suspected Areas

* `PM_SendMessage`
* Networking callbacks
* DemonWare handlers
* Steam interface hooks
* Uninitialized game pointers

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
