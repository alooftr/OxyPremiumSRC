#pragma once

#include "../valve_sdk/csgostructs.hpp"

#include <list>
#include <mutex>

namespace entities
{
#pragma pack(push, 1)
	struct local_data_t
	{
		Vector local_pos;
		Vector world_pos;
		QAngle angles;
		bool is_scoped;
		bool is_flashed;
		bool is_alive;
		int tick_base;
		int health;
		int armor_value;

		bool has_sniper;

		int hp = 0;
		float bomb_time = 0.f;
		float defuse_time = 0.f;

		bool is_matchmaking = false;

		C_BasePlayer* local;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct player_data_t
	{
		int index = 0;

		std::string name;
		std::string weapon;

		bool is_shooting = false;
		Vector shot_origin;

		bool is_enemy;
		bool is_scoped;
		bool is_dormant;

		bool in_smoke;
		bool is_visible;

		float m_iHealth;
		float m_ArmorValue;
		float m_flShotTime;
		float m_flSimulationTime;

		Vector points[8];

		RECT box;
		Vector eye_pos;
		Vector offset;
		Vector origin;
		Vector world_pos;
		QAngle angles;
		matrix3x4_t* bone_matrix;
		std::array<Vector, 4> hitboxes[HITBOX_MAX];
	};
#pragma pack(pop)

	struct tick_data_t
	{
		int tick_count;

		player_data_t players[65];
	};

	extern std::mutex locker;
	extern std::mutex local_mutex;

	extern local_data_t m_local;
	extern std::list<tick_data_t> m_items;

	void fetch_targets(CUserCmd* cmd);
}