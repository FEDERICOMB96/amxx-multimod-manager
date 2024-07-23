#if defined _mm_version_included_
	#endinput
#endif
#define _mm_version_included_

// Do NOT modify this file!

#define MM_VERSION_MAJOR           2
#define MM_VERSION_MINOR           3
#define MM_VERSION_MAINTENANCE     0

#define PLUGIN_VERSION        fmt("v%d.%d.%d.(%s %s)", MM_VERSION_MAJOR, MM_VERSION_MINOR, MM_VERSION_MAINTENANCE, __DATE__, __TIME__)