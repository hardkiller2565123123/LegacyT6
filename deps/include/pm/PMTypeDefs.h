// ==========================================================
// PlusMW3 project
// 
// Component: xPM
// Sub-component: libPM
// Purpose: type definitions for libPM
//
// Initial author: NTAuthority
// Started: 2011-06-28
// ==========================================================

#pragma once

// ----------------------------------------------------------
// stdint.h-style integer definitions
// ----------------------------------------------------------
#if _MSC_VER < 1600
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

// ----------------------------------------------------------
// C export definitions
// ----------------------------------------------------------
#ifdef LIBPM_STATIC
#define LIBPM_API
#else
#ifdef LIBPM_EXPORTS
#define LIBPM_API extern "C" __declspec(dllimport)
#else
#define LIBPM_API extern "C" __declspec(dllexport)
#endif
#endif

#define LIBPM_CALL __cdecl

// ----------------------------------------------------------
// Various (base) includes
// ----------------------------------------------------------
#define PMID_GET_USERID(x) (unsigned int)(x & 0xFFFFFFFF)

typedef uint64_t PMID;
