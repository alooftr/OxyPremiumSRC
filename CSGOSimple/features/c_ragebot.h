#pragma once
#include <optional>
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../features/c_animation_system.h"


class c_ragebot
{

	struct aim_info
	{
		aim_info(const Vector position, const float damage, c_animation_system::animation* animation, const bool alt_attack,
			const Vector center, const float radius, const float rs, const int hitbox, const int32_t hitgroup)
			: position(position), damage(damage), animation(animation), alt_attack(alt_attack),
			center(center), radius(radius), rs(rs), hitbox(hitbox), hitgroup(hitgroup) { }

		Vector position{};
		float damage{};
		c_animation_system::animation* animation{};
		bool alt_attack{};
		Vector center{};
		float radius{}, rs{};
		int hitbox{};
		int32_t hitgroup{};
	};


	struct autostop_info
	{
		float call_time;
		bool did_stop;
	};
public:

	static void aim(C_BasePlayer* local, CUserCmd* cmd, bool& send_packet);
	static void autostop(C_BasePlayer* local, CUserCmd* cmd);
	static std::vector<aim_info> select_multipoint(c_animation_system::animation* animation, int box, int32_t group, float scaled_head, float scaled_body);
	static std::optional<aim_info> scan_record_gun(C_BasePlayer* local, c_animation_system::animation* animation,
		std::optional<Vector> pos = std::nullopt);
	static std::optional<aim_info> scan_record_aimbot(C_BasePlayer* local, c_animation_system::animation* animation, std::optional<Vector> pos = std::nullopt);

	static autostop_info& get_autostop_info();

	inline static std::optional<float> last_pitch = std::nullopt;
private:
	static void optimize_multipoint(c_ragebot::aim_info& info);
	static std::optional<aim_info> scan_record(C_BasePlayer* local, c_animation_system::animation* animation);
	static std::optional<aim_info> scan_record_knife(C_BasePlayer* local, c_animation_system::animation* animation);

	static bool is_breaking_lagcomp(c_animation_system::animation* animation);
};

#define ragebot c_ragebot::Get()
