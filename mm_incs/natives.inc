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
 * @return (int)        Index of the current mod.
 *
 * native mm_get_mod_id();
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
 * @param iModId        Mod index.
 * @param szOutput      Buffer to copy mod name to.
 * @param iLen          Buffer size.
 *
 * @return (int)        -1 on error.
 *                      >= 0 Number of cells written.
 *
 * native mm_get_mod_name(const iModId, szOutput[], const iLen);
 */
public _mm_get_mod_name(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1, arg_output, arg_len };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_mod_name' needs %d param(s) (count: %d)", (args_e-1), argc);
		return -1;
	}

	new iModId = get_param(arg_modid);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return -1;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return set_string(arg_output, aMods[ModName], get_param(arg_len));
}

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
 *
 * native mm_get_mod_tag(const iModId, szOutput[], const iLen);
 */
public _mm_get_mod_tag(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1, arg_output, arg_len };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_mod_tag' needs %d param(s) (count: %d)", (args_e-1), argc);
		return -1;
	}

	new iModId = get_param(arg_modid);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return -1;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return set_string(arg_output, aMods[ModTag], get_param(arg_len));
}

/**
 * Return the next mod index based on JSON position if nextmod is already chosen.
 *
 * @return (int)        -1 if nextmod is not chosen yet.
 *                      >= 0 Index of the next mod.
 *
 * native mm_get_next_mod_id();
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
 * @param szOutput      Buffer to copy mod name to.
 * @param iLen          Buffer size.
 *
 * @return (int)        -1 on error.
 *                      >= 0 Number of cells written.
 *
 * native mm_get_next_mod_name(szOutput[], const iLen);
 */
public _mm_get_nextmod_name(plugin_id, argc)
{
	enum _:args_e { arg_output = 1, arg_len };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_next_mod_name' needs %d param(s) (count: %d)", (args_e-1), argc);
		return -1;
	}
	
	if(!g_bSelectedNextMod)
		return set_string(arg_output, "No hay modo elegido aún", get_param(arg_len));
	
	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], g_iNextSelectMod, aMods);
		
	return set_string(arg_output, aMods[ModName], get_param(arg_len));
}

/**
 * Force a votemod.
 *
 * @return (int)        1 on success, 0 otherwise
 *
 * native mm_force_votemod();
 */
public _mm_force_votemod(plugin_id, argc)
{
	if(CanForceVoteNextMod())
	{
		StartVoteNextMod();
		return 1;
	}
	
	return 0;
}