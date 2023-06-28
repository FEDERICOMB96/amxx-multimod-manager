#if defined _mm_natives_included_
	#endinput
#endif
#define _mm_natives_included_

/* ===========================================================================
*                 [ NATIVES ]
* ============================================================================ */

 /**
 * Return the current mod index based on JSON position.
 *
 * @return (int)		Index of the current mod.
 */
public _mm_get_mod_id(plugin_id, argc)
{
	return g_iCurrentMod;
}

/**
 * Retrieves mod name.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param iMod			Mod index.
 * @param szOutput		Buffer to copy mod name to.
 * @param iLen			Buffer size.
 *
 * @return (int)		-1 on error.
 *						>= 0 Number of cells written.
 */
public _mm_get_mod_name(plugin_id, argc)
{
	if(argc != 3)
		return 0;

	new iModId = get_param(1);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return -1;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return set_string(2, aMods[ModName], get_param(3));
}

/**
 * Retrieves mod name.
 *
 * @note If the destination buffer is too small to hold the source string, the 
 *       destination will be truncated.
 *
 * @param iMod			Mod index.
 * @param szOutput		Buffer to copy mod name to.
 * @param iLen			Buffer size.
 *
 * @return (int)		-1 on error.
 *						>= 0 Number of cells written.
 */
public _mm_get_mod_tag(plugin_id, argc)
{
	if(argc != 3)
		return 0;

	new iModId = get_param(1);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return -1;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return set_string(2, aMods[ModTag], get_param(3));
}

 /**
 * Return the next mod index based on JSON position if nextmod is already chosen.
 *
 * @return (int)		>= 0 Index of the next mod.
 *						-1 If nextmod is not chosen yet.
 */
public _mm_get_nextmod_id(plugin_id, argc)
{
	if(!g_bSelectedNextMod)
		return -1;

	return g_iNextSelectMod;
}

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
public _mm_get_nextmod_name(plugin_id, argc)
{
	if(argc != 2)
		return 0;
	
	if(!g_bSelectedNextMod)
		return set_string(1, "No hay modo elegido aún", get_param(2));
	
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);
		
	return set_string(1, aMods[ModName], get_param(2));
}