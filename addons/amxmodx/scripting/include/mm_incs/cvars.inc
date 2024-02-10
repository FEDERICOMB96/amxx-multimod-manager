#if defined _mm_cvars_included_
	#endinput
#endif
#define _mm_cvars_included_

Cvars_Init()
{
	g_pCvar_mp_timelimit = get_cvar_pointer("mp_timelimit");
	bind_pcvar_num(get_cvar_pointer("mp_winlimit"), g_bCvar_mp_winlimit);
	bind_pcvar_num(get_cvar_pointer("mp_maxrounds"), g_bCvar_mp_maxrounds);

	bind_pcvar_string((g_pCvar_amx_nextmap = create_cvar("amx_nextmap", fmt("%L", LANG_SERVER, "MM_NO_MAP_CHOOSED_CVAR"), FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY)), g_bCvar_amx_nextmap, charsmax(g_bCvar_amx_nextmap));
	bind_pcvar_num(create_cvar("amx_extendmap_max", "90"), g_bCvar_amx_extendmap_max);
	bind_pcvar_num(create_cvar("amx_extendmap_step", "15"), g_bCvar_amx_extendmap_step);
	bind_pcvar_num(create_cvar("amx_vote_answers", "1"), g_bCvar_amx_vote_answers);
	bind_pcvar_num((g_pCvar_amx_last_voting = create_cvar("amx_last_voting", "0")), g_bCvar_amx_last_voting);
	bind_pcvar_num(create_cvar("amx_vote_delay_mm", "5"), g_bCvar_amx_vote_delay_mm);
	bind_pcvar_float((g_pCvar_mp_chattime = get_cvar_pointer("mp_chattime")), g_bCvar_mp_chattime);
	bind_pcvar_num(get_cvar_pointer("mp_timelimit"), g_bCvar_mp_timelimit);
	bind_pcvar_num(create_cvar("amx_multimod_voice", "1"), g_bCvar_amx_multimod_voice);

	set_pcvar_string(g_pCvar_amx_nextmap, fmt("%L", LANG_SERVER, "MM_NO_MAP_CHOOSED_CVAR"));
	set_pcvar_num(g_pCvar_amx_last_voting, 0);

	set_pcvar_string(create_cvar("multimod_manager_version", PLUGIN_VERSION, FCVAR_SERVER | FCVAR_SPONLY), PLUGIN_VERSION);
	set_pcvar_string(create_cvar("multimod_manager_url", PLUGIN_URL, FCVAR_SERVER | FCVAR_SPONLY), PLUGIN_URL);
}