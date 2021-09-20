#pragma once

#include "../valve_sdk/csgostructs.hpp"

class c_events : public IGameEventListener2
{
public:
	static void hook();

	void FireGameEvent(IGameEvent* event) override;
	int GetEventDebugID() override;

	inline static bool is_active_round = false;
};
