#include "profile.hpp"
#include "..//helpers/proto/message.hpp"
#include "..//helpers/proto//writer.hpp"
#include "..//options.hpp"

#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo
#define CAST(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)

std::string Profile::Changer(void* pubDest, uint32_t* pcubMsgSize)
{
	Writer msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, 19);

	//replace commends
	auto _commendation = msg.has(_gc2ch::commendation) ? msg.get(_gc2ch::commendation).String() : std::string("");
	Writer commendation(_commendation, 4);

	if (g_Options.profile.friendly > -1)
		commendation.replace(Field(_pci::cmd_friendly, TYPE_UINT32, (int64_t)g_Options.profile.friendly));

	if (g_Options.profile.teaching > -1)
		commendation.replace(Field(_pci::cmd_teaching, TYPE_UINT32, (int64_t)g_Options.profile.teaching));

	if (g_Options.profile.leader > -1)
		commendation.replace(Field(_pci::cmd_leader, TYPE_UINT32, (int64_t)g_Options.profile.leader));

	msg.replace(Field(_gc2ch::commendation, TYPE_STRING, commendation.serialize()));

	auto _ranking = msg.has(_gc2ch::ranking) ? msg.get(_gc2ch::ranking).String() : std::string("");
	Writer ranking(_ranking, 6);
	Writer rankingwm(_ranking, 7);
	Writer rankingdg(_ranking, 8);

	if (g_Options.profile.rank_mm > 0)
		ranking.replace(Field(_pri::rank_mm, TYPE_UINT32, (int64_t)g_Options.profile.rank_mm));
	if (g_Options.profile.wins_mm > -1)
		ranking.replace(Field(_pri::wins_mm, TYPE_UINT32, (int64_t)g_Options.profile.wins_mm));

	if (g_Options.profile.rank_wm > 0)
		rankingwm.replace(Field(_pri::rank_wm, TYPE_UINT32, (int64_t)g_Options.profile.rank_wm));
	if (g_Options.profile.wins_wm > -1)
		rankingwm.replace(Field(_pri::wins_wm, TYPE_UINT32, (int64_t)g_Options.profile.wins_wm));

	if (g_Options.profile.rank_danger > 0)
		rankingdg.replace(Field(_pri::rank_danger, TYPE_UINT32, (int64_t)g_Options.profile.rank_danger));
	if (g_Options.profile.wins_danger > -1)
		rankingdg.replace(Field(_pri::wins_danger, TYPE_UINT32, (int64_t)g_Options.profile.wins_danger));

	msg.replace(Field(_gc2ch::ranking, TYPE_STRING, ranking.serialize()));
	msg.replace(Field(_gc2ch::ranking, TYPE_STRING, rankingwm.serialize()));
	msg.replace(Field(_gc2ch::ranking, TYPE_STRING, rankingdg.serialize()));

	if (g_Options.profile.player_level > -1)
		msg.replace(Field(_gc2ch::player_level, TYPE_INT32, (int64_t)g_Options.profile.player_level));

	if (g_Options.profile.player_xp > -1)
		msg.replace(Field(_gc2ch::player_xp, TYPE_INT32, (int64_t)g_Options.profile.player_xp));


	return msg.serialize();
}