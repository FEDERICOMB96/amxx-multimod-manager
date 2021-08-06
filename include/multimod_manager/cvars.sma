#if defined _mm_cvars_included_
	#endinput
#endif
#define _mm_cvars_included_

#include "./include/multimod_manager/defines.sma"

/*

 ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗██╗   ██╗ █████╗ ██████╗ ███████╗
██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔════╝██║   ██║██╔══██╗██╔══██╗██╔════╝
██║  ███╗███████║██╔████╔██║█████╗      ██║     ██║   ██║███████║██████╔╝███████╗
██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║     ╚██╗ ██╔╝██╔══██║██╔══██╗╚════██║
╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╗ ╚████╔╝ ██║  ██║██║  ██║███████║
 ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
                                                                                 
*/
cvar_t g_pCvar_mp_timelimit;
cvar_t g_bCvar_mp_winlimit;
cvar_t g_bCvar_mp_maxrounds;

/*

██████╗ ██╗     ██╗   ██╗ ██████╗ ██╗███╗   ██╗     ██████╗██╗   ██╗ █████╗ ██████╗ ███████╗
██╔══██╗██║     ██║   ██║██╔════╝ ██║████╗  ██║    ██╔════╝██║   ██║██╔══██╗██╔══██╗██╔════╝
██████╔╝██║     ██║   ██║██║  ███╗██║██╔██╗ ██║    ██║     ██║   ██║███████║██████╔╝███████╗
██╔═══╝ ██║     ██║   ██║██║   ██║██║██║╚██╗██║    ██║     ╚██╗ ██╔╝██╔══██║██╔══██╗╚════██║
██║     ███████╗╚██████╔╝╚██████╔╝██║██║ ╚████║    ╚██████╗ ╚████╔╝ ██║  ██║██║  ██║███████║
╚═╝     ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝     ╚═════╝  ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
                                                                                            
*/
cvar_t g_bCvar_amx_nextmap[64], g_pCvar_amx_nextmap;
cvar_t g_bCvar_amx_extendmap_max;
cvar_t g_bCvar_amx_extendmap_step;
cvar_t g_bCvar_amx_vote_answers;

/* Pointer Cvars */

CvarsInit()
{
	g_pCvar_mp_timelimit = get_cvar_pointer("mp_timelimit");
	bind_pcvar_num(get_cvar_pointer("mp_winlimit"), g_bCvar_mp_winlimit);
	bind_pcvar_num(get_cvar_pointer("mp_maxrounds"), g_bCvar_mp_maxrounds);

	bind_pcvar_string((g_pCvar_amx_nextmap = create_cvar("amx_nextmap", "[sin elegir]", FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY)), g_bCvar_amx_nextmap, charsmax(g_bCvar_amx_nextmap));
	bind_pcvar_num(create_cvar("amx_extendmap_max", "90"), g_bCvar_amx_extendmap_max);
	bind_pcvar_num(create_cvar("amx_extendmap_step", "15"), g_bCvar_amx_extendmap_step);
	bind_pcvar_num(create_cvar("amx_vote_answers", "1"), g_bCvar_amx_vote_answers);	
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang3082\\ f0\\ fs16 \n\\ par }
*/
