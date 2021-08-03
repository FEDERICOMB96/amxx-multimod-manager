#if defined _mm_rtv_included_
	#endinput
#endif
#define _mm_rtv_included_

#include <amxmodx>
#include <multimod_manager/defines>

new g_IsRtv = 0; // Si es un RockTheVote, no permitir extender el mapa.
new g_VoteRtvResult = 0; // Si es 1, el RockTheVote termino bien.
new g_VoteCountRtv = 0; // "rtv" que han realizado los jugadores
new g_IsVotingRtv = 0; // Se esta votando el rtv?
new g_SystimeRtv = 0;

new g_HasVoteRTV[MAX_USERS];
new g_SVM_Nominate[MAX_USERS];
new g_SVM_MyNominate[MAX_USERS];