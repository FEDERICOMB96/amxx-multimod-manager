#if defined _multimod_manager_natives_included_
	#endinput
#endif
#define _multimod_manager_natives_included_

/**
 * Multimod Manager version check.
 * Do not modify this!
 */
#define MM_VERSION_MAJOR 2
#define MM_VERSION_MINOR 0
#define MM_VERSION_PATCH 0

/* ===========================================================================
*                 [ MULTIMOD MANAGER NATIVES ]
* ============================================================================ */

/**
 * Return the loaded mods count.
 *
 * @return (int)        Number of loaded mods.
 */
native mm_get_mods_count();

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
 * Return the current mod index based on JSON position.
 *
 * @return (int)        Index of the current mod.
 */
native mm_get_currentmod_id();

/**
 * Return the next mod index based on JSON position if nextmod is already chosen.
 *
 * @return (int)        -1 if nextmod is not chosen yet.
 *                      >= 0 Index of the next mod.
 */
native mm_get_nextmod_id();

/**
 * Force a votemod.
 *
 * @return (bool)       true on success, false otherwise
 */
native bool:mm_force_votemod();

/**
 * Force a map change.
 *
 * @param iModId        Mod index.
 * @param szMap         Map to change to.
 * @param flChangeTime  Time to wait before changing map.
 *                      0.0 to change map immediately.
 *
 * @return (bool)       true on success, false otherwise.
 */
native bool:mm_force_change_map(const iModId, const sMap[], const Float:flChangeTime = 0.0);

/* ===========================================================================
*                 [ MULTIMOD MANAGER FORWARDS ]
* ============================================================================ */

/**
 * Called when a votemod gets started
 *
 * @param bSecondVote   True if this is the second votemod for tiebreaker
 */
forward multimod_start_votemod(const bool:bSecondVote);

/**
 * Called when a votemod gets ended
 *
 * @param bSecondVote   True if this is the second votemod for tiebreaker
 */
forward multimod_end_votemod(const bool:bSecondVote);

/**
 * Called when a votemap gets started
 *
 * @param bSecondVote   True if this is the second votemap for tiebreaker
 */
forward multimod_start_votemap(const bool:bSecondVote);

/**
 * Called when a votemap gets ended
 *
 * @param bSecondVote   True if this is the second votemap for tiebreaker
 */
forward multimod_end_votemap(const bool:bSecondVote);

/**
 * Called when a admin forces a votemod
 *
 * @param iAdminId      Admin index
 *
 * @return              PLUGIN_CONTINUE to let the admin force a votemod
 *                      PLUGIN_HANDLED or higher to stop the admin from forcing a votemod
 */
forward multimod_admin_force_votemod(const iAdminId);

/**
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