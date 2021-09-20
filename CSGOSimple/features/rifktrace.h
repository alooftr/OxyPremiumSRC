#pragma once

#include <optional>
#include <functional>
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"
#include "c_animation_system.h"


#define char_tex_concrete 'C'
#define char_tex_metal 'M'
#define char_tex_dirt 'D'
#define char_tex_vent 'V'
#define char_tex_grate 'G'
#define char_tex_tile 'T'
#define char_tex_slosh 'S'
#define char_tex_wood 'W'
#define char_tex_computer 'P'
#define char_tex_glass 'Y'
#define char_tex_flesh 'F'
#define char_tex_snow 'N'
#define char_tex_plastic 'L'
#define char_tex_cardboard 'U'

class c_trace_system : public Singleton<c_trace_system>
{
public:
	struct wall_pen
	{
		float damage;
		int hitbox;
		int32_t hitgroup;
	};

	std::optional<wall_pen> wall_penetration(Vector src, Vector end,
		c_animation_system::animation* target, C_BasePlayer* override_player = nullptr) const;

	static void run_emulated(c_animation_system::animation* target, std::function<void()> fn);

	static void extrapolate(C_BasePlayer* player, Vector& origin, Vector& velocity, int& flags, bool on_ground);

private:
	static std::optional<wall_pen> fire_bullet(CCSWeaponInfo* data, Vector src,
		Vector pos, CTraceFilter* filter, C_BasePlayer* target = nullptr, bool point = false);

	static bool handle_bullet_penetration(CCSWeaponInfo* weapon_data, trace_t& enter_trace,
		Vector& eye_position, Vector direction, int& penetrate_count,
		float& current_damage, float penetration_power);
	static bool trace_to_exit(trace_t& enter_trace, trace_t& exit_trace, Vector start_position, Vector direction, bool is_local = false);

	static float scale_damage(C_BasePlayer* target, float damage, float weapon_armor_ratio, int hitgroup, bool is_zeus);
};

#define trace_system c_trace_system::Get()
