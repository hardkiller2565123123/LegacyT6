# BO2 Client Source Restoration

## Credits

Original source code, research, and tooling belong to their respective original developers and communities.

This repository exists for preservation, restoration, compatibility repair, and historical research purposes only.

Special credit to:

- T6M developers
- FourDeltaOne developers
- Original BO2 reverse engineering community
- Legacy multiplayer preservation contributors
- Community researchers who documented DemonWare, Steam, and BO2 internals

---

# T6M Restoration Status

### Overall Restoration Progress

```txt
[███████████████████░░░░░░░░░░░░░░░░░░░░░] 42%
```

### Core Systems

| Component                     | Status |
|------------------------------|--------|
| Project Structure            | Restored |
| VS2013 Solution Loading      | Restored |
| x86 Build Environment        | Restored |
| Main Menu Launch             | Working |
| Basic Steam Stub             | Partial |
| Dependency Cleanup           | In Progress |
| Legacy SDK Compatibility     | In Progress |
| DemonWare Emulation          | Not Started |
| Matchmaking                  | Offline |
| Server Browser               | Offline |
| Party System                 | Offline |
| Authentication               | Offline |
| Crash Stabilization          | In Progress |
| PM_SendMessage Repair        | In Progress |

---

# Current State

The current T6M restoration successfully:

- Builds under legacy Visual Studio environments
- Launches into the BO2 multiplayer main menu
- Loads major engine modules
- Restores portions of legacy Steam interfaces
- Repairs several broken project references and dependencies

The project is **not yet fully playable online**.

Major networking systems such as DemonWare emulation, matchmaking, and party handling are still incomplete or stubbed.

---

# Known Issues

## Current Crash

Recent dump analysis shows:

- `0xC0000005` Access Violation
- Null execution at:

```cpp
0x00000000
```

This indicates a missing or unresolved function pointer during runtime execution.

### Suspected Areas

- `PM_SendMessage`
- Uninitialized networking callbacks
- Missing DemonWare handlers
- Invalid Steam interface exports
- Incomplete hook initialization
- Null virtual function calls

---

# Restoration Goals

- Restore original build compatibility
- Preserve original project structure
- Repair broken linker/compiler issues
- Restore missing Steam exports
- Improve runtime stability
- Repair legacy networking systems
- Implement safer initialization checks
- Rebuild missing DemonWare systems
- Document offsets, hooks, and crash causes
- Preserve historical BO2 client development

---

# Build Requirements

## Recommended Environment

- Visual Studio 2013 Update 5
- Windows SDK 8.1
- DirectX SDK (June 2010)
- x86 Configuration

### Archive Link

https://archive.org/details/en_visual_studio_community_2013_with_update_5_x86_dvd_6816332

---

# Important Notes

These projects were originally designed around:

- Older MSVC compiler behavior
- Legacy Windows SDKs
- Deprecated Steam APIs
- Older dependency chains
- Pre-modern C++ standards

Modern Visual Studio versions may require:

- Retargeting toolsets
- Manual include/lib configuration
- Disabled warning promotion
- Legacy runtime installation

Avoid fully modernizing the codebase unless necessary for stability.

---

# Repository Purpose

This repository focuses on:

- Software preservation
- Historical research
- Reverse engineering education
- Offline experimentation
- Restoration of abandoned source code

This is **not** a commercial project.

---

# Disclaimer

This repository is intended strictly for:

- Archival purposes
- Educational research
- Offline testing
- Software restoration

No copyrighted game assets, proprietary binaries, or commercial content are included.

Users are responsible for owning legitimate copies of any required software or games.
