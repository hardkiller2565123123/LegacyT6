# BO2 Client Source Restoration

Restoration and build-fix repository for archived Black Ops 2 / T6 client source codes.

This project focuses on getting older BO2 client sources compiling and running again on modern systems while preserving the original codebases as much as possible.

## Included Sources

* PlusOpsSource
* T6M Source
* FourDeltaOne.SourceCode
* RedactedSteamBase

## Purpose

Many older BO2/T6 client sources no longer compile correctly due to:

* Missing dependencies
* Broken project settings
* Old Visual Studio toolchains
* Missing SDKs
* Runtime crashes
* Removed libraries
* Modern compiler incompatibilities

This repository documents fixes, dependency setup, crash debugging, and restoration work required to make these projects buildable again.

## Requirements

### Main Requirement

Visual Studio Community 2013 Update 5:

https://archive.org/details/en_visual_studio_community_2013_with_update_5_x86_dvd_6816332

### Recommended Components

* Windows 10 SDK
* Windows 11 SDK
* MSVC v142 Build Tools
* MSVC v143 Build Tools
* C++ MFC
* C++ ATL
* .NET Framework 4.7.2 Tools
* .NET Framework 4.8 Tools

## Current Project Status

| Project                 | Status                                 |
| ----------------------- | -------------------------------------- |
| PlusOpsSource           | Builds partially, pm_sendMessage issue |
| T6M Source              | Builds but crashes                     |
| FourDeltaOne.SourceCode | Pending cleanup                        |
| RedactedSteamBase       | Pending cleanup                        |

## Goals

* Restore original project build environments
* Fix compile/linker errors
* Repair broken dependencies
* Document known issues
* Improve compatibility on modern Windows
* Preserve old BO2 client source history

## Notes

Most projects are designed around older MSVC behavior and legacy SDKs.

Do not fully retarget or modernize projects unless necessary.

Some projects may require:

* x86 build configuration
* Legacy DirectX SDKs
* Old Steam/API libraries
* Manual include/lib path setup

## Disclaimer

This repository is intended for archival, educational, offline research, and software restoration purposes only.

No game assets or copyrighted files are included.
