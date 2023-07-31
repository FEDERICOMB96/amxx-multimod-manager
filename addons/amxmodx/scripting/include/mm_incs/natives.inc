#if defined _mm_natives_included_
	#endinput
#endif
#define _mm_natives_included_

/* ===========================================================================
*                 [ NATIVES ]
* ============================================================================ */

/**
 * Return the loaded mods count.
 *
 * @return (int)        Number of loaded mods.
 *
 * native mm_get_mods_count();
 */
public _mm_get_mods_count(plugin_id, argc)
{
	return ArraySize(g_GlobalConfigs[Mods]);
}

/**
 * Return if the mod is enabled or not.
 *
 * @param iModId        Mod index.
 *
 * @return (bool)       true if the mod is enabled, false otherwise.
 *
 * native bool:mm_is_mod_enabled(const iModId);
 */
public bool:_mm_is_mod_enabled(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1 };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_is_mod_enabled' needs %d param(s) (count: %d)", (args_e-1), argc);
		return false;
	}

	new iModId = get_param(arg_modid);

	if(iModId < 0 || iModId >= ArraySize(g_GlobalConfigs[Mods]))
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return false;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return aMods[Enabled];
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
 * Return the mod changemap type.
 *
 * @note See 'ChangeMap_e' enum for more information.
 *
 * @param iModId        Mod index.
 *
 * @return (int)        0 on change map timeleft.
 *                      1 on change map end of round.
 *                      2 on change map one more round.
 *
 * native ChangeMap_e:mm_get_mod_changemap_type(const iModId);
 */
public ChangeMap_e:_mm_get_mod_changemap_type(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1 };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_mod_changemap_type' needs %d param(s) (count: %d)", (args_e-1), argc);
		return CHANGEMAP_TIMELEFT;
	}

	new iModId = get_param(arg_modid);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return CHANGEMAP_TIMELEFT;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return aMods[ChangeMapType];
}

/**
 * Return the maps list of the mod.
 *
 * @param iModId        Mod index.
 *
 * @return (Array)      Maps list array handle, which must be freed via ArrayDestroy()
 *
 * native Array:mm_get_mod_maps(const iModId);
 */
public Array:_mm_get_mod_maps(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1 };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_mod_maps' needs %d param(s) (count: %d)", (args_e-1), argc);
		return Invalid_Array;
	}

	new iModId = get_param(arg_modid);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return Invalid_Array;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return ArrayClone(aMods[Maps]);
}

/**
 * Return the cvars list of the mod.
 *
 * @param iModId        Mod index.
 *
 * @return (Array)      Cvars list array handle, which must be freed via ArrayDestroy()
 *
 * native Array:mm_get_mod_cvars(const iModId);
 */
public Array:_mm_get_mod_cvars(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1 };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_mod_cvars' needs %d param(s) (count: %d)", (args_e-1), argc);
		return Invalid_Array;
	}

	new iModId = get_param(arg_modid);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return Invalid_Array;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return ArrayClone(aMods[Cvars]);
}

/**
 * Return the plugin list of the mod.
 *
 * @param iModId        Mod index.
 *
 * @return (Array)      Plugins list array handle, which must be freed via ArrayDestroy()
 *
 * native Array:mm_get_mod_plugins(const iModId);
 */
public Array:_mm_get_mod_plugins(plugin_id, argc)
{
	enum _:args_e { arg_modid = 1 };

	if(argc != (args_e-1))
	{
		abort(AMX_ERR_NATIVE, "'mm_get_mod_plugins' needs %d param(s) (count: %d)", (args_e-1), argc);
		return Invalid_Array;
	}

	new iModId = get_param(arg_modid);
	new iArraySizeMods = ArraySize(g_GlobalConfigs[Mods]);

	if(iModId < 0 || iModId >= iArraySizeMods)
	{
		abort(AMX_ERR_NATIVE, "Invalid array size (%d)", iModId);
		return Invalid_Array;
	}

	new aMods[ArrayMods_e];
	ArrayGetArray(g_GlobalConfigs[Mods], iModId, aMods);

	return ArrayClone(aMods[Plugins]);
}

/**
 * Return the current mod index based on JSON position.
 *
 * @return (int)        Index of the current mod.
 *
 * native mm_get_currentmod_id();
 */
public _mm_get_currentmod_id(plugin_id, argc)
{
	return g_iCurrentMod;
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