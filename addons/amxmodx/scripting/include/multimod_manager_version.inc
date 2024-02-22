#if defined _multimod_manager_version_included_
	#endinput
#endif
#define _multimod_manager_version_included_

// Do NOT modify this file!

/**
 * AMX Mod X version check
 */
#if defined AMXX_VERSION_NUM
	#if AMXX_VERSION_NUM < 200
		#error "[API] Multimod Manager CS requires AMX Mod X 1.10 or higher. Download from: https://www.amxmodx.org/downloads-new.php?branch=master"
	#endif
#else
	#error "[API] Multimod Manager CS: please include multimod_manager_natives.inc after including amxmodx.inc."
#endif

#define MM_VERSION_MAJOR      2
#define MM_VERSION_MINOR      2
#define MM_VERSION_COMMIT     207

#define MM_NATIVES_API_VER    fmt("v%d.%d.%d", MM_VERSION_MAJOR, MM_VERSION_MINOR, MM_VERSION_COMMIT)