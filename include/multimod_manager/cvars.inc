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
cvar_t mm_timelimit;

/*

██████╗ ██╗     ██╗   ██╗ ██████╗ ██╗███╗   ██╗     ██████╗██╗   ██╗ █████╗ ██████╗ ███████╗
██╔══██╗██║     ██║   ██║██╔════╝ ██║████╗  ██║    ██╔════╝██║   ██║██╔══██╗██╔══██╗██╔════╝
██████╔╝██║     ██║   ██║██║  ███╗██║██╔██╗ ██║    ██║     ██║   ██║███████║██████╔╝███████╗
██╔═══╝ ██║     ██║   ██║██║   ██║██║██║╚██╗██║    ██║     ╚██╗ ██╔╝██╔══██║██╔══██╗╚════██║
██║     ███████╗╚██████╔╝╚██████╔╝██║██║ ╚████║    ╚██████╗ ╚████╔╝ ██║  ██║██║  ██║███████║
╚═╝     ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝╚═╝  ╚═══╝     ╚═════╝  ╚═══╝  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝
                                                                                            
*/
cvar_t mm_nextmod[64];
cvar_t mm_nextmap[64];
cvar_t mm_extendmap_max;
cvar_t mm_extendmap_step;
cvar_t mm_vote_answers;

CvarsInit()
{
	bind_pcvar_num(get_cvar_pointer("mp_timelimit"), mm_timelimit);

	bind_pcvar_string(create_cvar("amx_nextmod", "[sin elegir]", FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY), mm_nextmod, charsmax(mm_nextmod));
	bind_pcvar_string(create_cvar("amx_nextmap", "[sin elegir]", FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY), mm_nextmap, charsmax(mm_nextmap));
	bind_pcvar_num(create_cvar("amx_extendmap_max", "90"), mm_extendmap_max);
	bind_pcvar_num(create_cvar("amx_extendmap_step", "15"), mm_extendmap_step);
	bind_pcvar_num(create_cvar("amx_vote_answers", "1"), mm_vote_answers);
	
}