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

 /**
 * Return the next mod index based on JSON position if nextmod is already chosen.
 *
 * @return (int)		>= 0 Index of the next mod.
 *						-1 If nextmod is not chosen yet.
 */
native mm_get_next_mod_id();

/**
 * Retrieves next mod name.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param szOutput		Buffer to copy mod name to.
 * @param iLen			Buffer size.
 *
 * @return (int)		-1 on error.
 *						>= 0 Number of cells written.
 */
native mm_get_next_mod_name(szOutput[], const iLen);