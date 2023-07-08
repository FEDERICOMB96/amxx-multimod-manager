#if defined _multimod_manager_natives_included_
	#endinput
#endif
#define _multimod_manager_natives_included_

/* ===========================================================================
*                 [ MULTIMOD MANAGER NATIVES ]
* ============================================================================ */

/**
 * Multimod Manager version check.
 * Do not modify this!
 */
#define MM_VERSION_MAJOR 1
#define MM_VERSION_MINOR 0
#define MM_VERSION_PATCH 160

/**
 * Return the current mod index based on JSON position.
 *
 * @return (int)        Index of the current mod.
 */
native mm_get_mod_id();

/**
 * Retrieves mod name.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param iModId        Mod index.
 * @param szOutput      Buffer to copy mod name to.
 * @param iLen          Buffer size.
 *
 * @return (int)        -1 on error.
 *                      >= 0 Number of cells written.
 */
native mm_get_mod_name(const iModId, szOutput[], const iLen);

/**
 * Retrieves mod tag.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param iModId        Mod index.
 * @param szOutput      Buffer to copy mod tag to.
 * @param iLen          Buffer size.
 *
 * @return (int)        -1 on error.
 *                      >= 0 Number of cells written.
 */
native mm_get_mod_tag(const iModId, szOutput[], const iLen);

/**
 * Return the next mod index based on JSON position if nextmod is already chosen.
 *
 * @return (int)        -1 if nextmod is not chosen yet.
 *                      >= 0 Index of the next mod.
 */
native mm_get_nextmod_id();

/**
 * Retrieves next mod name.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param szOutput      Buffer to copy mod name to.
 * @param iLen          Buffer size.
 *
 * @return (int)        -1 on error.
 *                      >= 0 Number of cells written.
 */
native mm_get_nextmod_name(szOutput[], const iLen);

/**
 * Force a votemod.
 *
 * @return (int)        1 on success, 0 otherwise
 */
native mm_force_votemod();

/*
* This is the callback from the plugin core that gives major/minor versions for verifying compatibility for Multimod Manager versions.
* If an AMXX plugin gets a failure, then you do need to upgrade to the latest version of the Multimod Manager or update the files included for AMXX plugins.
* Do not modify this!
*
* Ported function from ReAPI include.
*/
forward __multimod_version_check(const majorVersion, const minorVersion);
public __multimod_version_check(const majorVersion, const minorVersion)
{
	if(majorVersion != MM_VERSION_MAJOR)
	{
		new szTemp[512];
		formatex(szTemp, charsmax(szTemp), "[MULTIMOD]: Api major version mismatch; expected %d, real %d", MM_VERSION_MAJOR, majorVersion);
		set_fail_state(szTemp);
		return;
	}

	if(minorVersion < MM_VERSION_MINOR)
	{
		new szTemp[512];
		formatex(szTemp, charsmax(szTemp), "[MULTIMOD]: Api minor version mismatch; expected at least %d, real %d", MM_VERSION_MINOR, minorVersion);
		set_fail_state(szTemp);
		return;
	}
}