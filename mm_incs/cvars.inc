#if defined _mm_cvars_included_
	#endinput
#endif
#define _mm_cvars_included_

Cvars_Init()
{
	g_pCvar_mp_timelimit = get_cvar_pointer("mp_timelimit");
	bind_pcvar_num(get_cvar_pointer("mp_winlimit"), g_bCvar_mp_winlimit);
	bind_pcvar_num(get_cvar_pointer("mp_maxrounds"), g_bCvar_mp_maxrounds);

	bind_pcvar_string((g_pCvar_amx_nextmap = create_cvar("amx_nextmap", "[sin elegir]", FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY)), g_bCvar_amx_nextmap, charsmax(g_bCvar_amx_nextmap));
	bind_pcvar_num(create_cvar("amx_extendmap_max", "90"), g_bCvar_amx_extendmap_max);
	bind_pcvar_num(create_cvar("amx_extendmap_step", "15"), g_bCvar_amx_extendmap_step);
	bind_pcvar_num(create_cvar("amx_vote_answers", "1"), g_bCvar_amx_vote_answers);
	bind_pcvar_num((g_pCvar_amx_last_voting = create_cvar("amx_last_voting", "0")), g_bCvar_amx_last_voting);
	bind_pcvar_num(create_cvar("amx_vote_delay_mm", "5"), g_bCvar_amx_vote_delay_mm);

	set_pcvar_string(g_pCvar_amx_nextmap, "[sin elegir]");
}