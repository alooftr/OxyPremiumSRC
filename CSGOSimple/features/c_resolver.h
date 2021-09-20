#pragma once

#include "../options.hpp"
#include "autowall.hpp"

namespace resolver
{
	struct shot
	{
	//	c_animation_system::animation record{};
		Vector start{}, end{};
		uint32_t hitgroup{};
		int hitbox{};
		float time{}, damage{};
		bool confirmed{}, impacted{}, skip{}, manual{};

		struct server_info_t
		{
			std::vector<Vector> impacts{};
			uint32_t hitgroup{}, damage{}, index{};
		} server_info;
	};
}

class c_resolver
{
public:
	static void resolve(C_BasePlayer* player);
	static void register_shot(resolver::shot&& s);
	static void on_player_hurt(IGameEvent* event);
	static void on_bullet_impact(IGameEvent* event);
	static void on_weapon_fire(IGameEvent* event);
	static void on_render_start();
	static void Listener();
private:
	static void resolve_shot(resolver::shot& shot);

	//inline static std::deque<resolver::shot> shots = {};
};
