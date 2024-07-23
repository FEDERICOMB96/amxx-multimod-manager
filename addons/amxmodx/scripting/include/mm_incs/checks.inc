#if defined _mm_checks_included_
	#endinput
#endif
#define _mm_checks_included_

// Do NOT modify this file!

/**
 * AMX Mod X version check
 */
#if AMXX_VERSION_NUM < 200
	#error "Multimod Manager CS requires AMX Mod X 1.10 or higher. Download from: https://www.amxmodx.org/downloads-new.php?branch=master"
#endif

/**
 * ReAPI version check
 */
#if REAPI_VERSION < 524300
	#error "Multimod Manager CS requires ReAPI 5.24.300 or higher. Download from: https://github.com/s1lentq/reapi/releases/latest"
#endif