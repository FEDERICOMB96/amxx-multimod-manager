#if defined _mm_cvars_included_
	#endinput
#endif
#define _mm_cvars_included_

#include <amxmodx>
#include <multimod_manager/defines>

/*

 ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗██╗   ██╗ █████╗ ██████╗ ███████╗
██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔════╝██║   ██║██╔══██╗██╔══██╗██╔════╝
██║  ███╗███████║██╔████╔██║█████╗      ██║     ██║   ██║███████║██████╔╝███████╗
██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║     ╚██╗ ██╔╝██╔══██║██╔══██╗╚════██║
╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╗ ╚████╔╝ ██║  ██║██║  ██║███████║
 ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
                                                                                 
*/
cvar_t mp_timelimit;
cvar_t mp_winlimit;
cvar_t mp_maxrounds;

/*

██████╗ ██╗     ██╗   ██╗ ██████╗ ██╗███╗   ██╗     ██████╗██╗   ██╗ █████╗ ██████╗ ███████╗
██╔══██╗██║     ██║   ██║██╔════╝ ██║████╗  ██║    ██╔════╝██║   ██║██╔══██╗██╔══██╗██╔════╝
██████╔╝██║     ██║   ██║██║  ███╗██║██╔██╗ ██║    ██║     ██║   ██║███████║██████╔╝███████╗
██╔═══╝ ██║     ██║   ██║██║   ██║██║██║╚██╗██║    ██║     ╚██╗ ██╔╝██╔══██║██╔══██╗╚════██║
██║     ███████╗╚██████╔╝╚██████╔╝██║██║ ╚████║    ╚██████╗ ╚████╔╝ ██║  ██║██║  ██║███████║
╚═╝     ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝     ╚═════╝  ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
                                                                                            
*/
cvar_t amx_nextmap[64], pamx_nextmap;
cvar_t amx_extendmap_max;
cvar_t amx_extendmap_step;
cvar_t amx_vote_answers;

/* Pointer Cvars */

CvarsInit()
{
	mp_timelimit = get_cvar_pointer("mp_timelimit");
	mp_winlimit = get_cvar_pointer("mp_winlimit");
	mp_maxrounds = get_cvar_pointer("mp_maxrounds");

	bind_pcvar_string((pamx_nextmap = create_cvar("amx_nextmap", "[sin elegir]", FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY)), amx_nextmap, charsmax(amx_nextmap));
	bind_pcvar_num(create_cvar("amx_extendmap_max", "90"), amx_extendmap_max);
	bind_pcvar_num(create_cvar("amx_extendmap_step", "15"), amx_extendmap_step);
	bind_pcvar_num(create_cvar("amx_vote_answers", "1"), amx_vote_answers);	
}