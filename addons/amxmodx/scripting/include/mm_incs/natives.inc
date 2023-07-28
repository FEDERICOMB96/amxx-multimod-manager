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
 * native mm_get_nextmod_id();
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
 * native mm_get_nextmod_name(szOutput[], const iLen);
 */
public _mm_get_nextmod_name(plugin_id, argc)
{
	enum _:args_e { arg_output = 1, arg_len };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_nextmod_name' needs %d param(s) (count: %d)", (args_e-1), argc);
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
 * @return (bool)       true on success, false otherwise
 *
 * native bool:mm_force_votemod();
 */
public bool:_mm_force_votemod(plugin_id, argc)
{
	if(CanForceVoteNextMod())
	{
		StartVoteNextMod();
		return true;
	}
	
	return false;
}

/**
 * Force a map change.
 *
 * @param iModId        Mod index.
 * @param szMap         Map to change to.
 * @param flChangeTime  Time to wait before changing map.
 *                      0.0 to change map immediately.
 *
 * @return (bool)       true on success, false otherwise.
 *
 * native bool:mm_force_change_map(const iModId, const sMap[], const Float:flChangeTime = 0.0);
 */
public _mm_force_change_map(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1, arg_map, arg_changetime };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_force_change_map' needs %d param(s) (count: %d)", (args_e-1), argc);
		return false;
	}

	new iModId = get_param(arg_modid);

	if(iModId < 0 || iModId >= ArraySize(g_GlobalConfigs[Mods]))
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return false;
	}
	
	new szMap[MAX_MAPNAME_LENGTH];
	get_string(arg_map, szMap, charsmax(szMap));

	if(!IsValidMap(szMap) || !IsValidMapForMod(iModId, szMap))
	{
		abort(AMX_ERR_NATIVE, "Invalid map name (%s)", szMap);
		return false;
	}

	new Float:flChangeTime = floatmax(0.0, get_param_f(arg_changetime));

	if(flChangeTime)
	{
		remove_task(TASK_FORCE_CHANGE_MAP);
		set_task(flChangeTime, "OnTask_ForceChangeMap", TASK_FORCE_CHANGE_MAP);

		return true;
	}

	OnCSGameRules_GoToIntermission();
	return true;
}