#if defined _multimod_manager_natives_included_
	#endinput
#endif
#define _multimod_manager_natives_included_

/* ===========================================================================
*                 [ MULTIMOD MANAGER NATIVES ]
* ============================================================================ */

/**
 * Return the current mod index based on JSON position.
 *
 * @return (int)		Index of the current mod.
 */
native mm_get_mod_id();

/**
 * Retrieves mod name.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param iModId		Mod index.
 * @param szOutput		Buffer to copy mod name to.
 * @param iLen			Buffer size.
 *
 * @return (int)		Number of cells written.
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
 * @return (int)        Number of cells written.
 */
native mm_get_mod_tag(const iModId, szOutput[], const iLen);