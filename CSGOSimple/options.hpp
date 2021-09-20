#pragma once
#include <set>
#include <string>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <limits>
#include "valve_sdk/Misc/Color.hpp"
#include "item_definitions.hpp"
#define A( s ) #s
#define OPTION(type, var, val) type var = val
#define PRIVATE 1

extern std::map<int, const char*> weapon_names; 

struct aimbot_settings {
	int autofire_key = 0;
	bool enabled = false;
	bool autofire = false;
	bool on_key = true;
	int key = 0;
	bool deathmatch = false;
	bool autopistol = false;
	bool check_smoke = false;
	bool check_flash = false;
	bool check_jump = false;
	bool autowall = false;
	bool silent = false;
	bool antiaimlock = false;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool humanize = false;
	float curviness = false;
	struct {
		bool enabled = false;
		int ticks = 6;
	} backtrack;
	bool only_in_zoom = true;
	int aim_type = 1;
	int smooth_type;
	int priority = 0;
	int fov_type = 0;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	float silent_fov = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;
	int min_damage = 1;
};

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T*() { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};
struct funstr {
	OPTION(float, playback, 0.f);
	OPTION(float, weight, 0.f);
	OPTION(int, order, 0);
	OPTION(int, sequence, 0);
	OPTION(float, cycle, 0.f);
	OPTION(float, weightdelta, 0.f);
	OPTION(float, prevcycle, 0.f);
	OPTION(float, posparam, 0.f);
	OPTION(float, feetrate, 0.f);
	OPTION(bool, funenable, false);
};

struct skinInfo
{
	std::string name;
	std::string cdnName;
	std::vector<std::string> weaponName;
	int rarity;
};

class Options
{
public:
	struct skins_s
	{
		struct sides
		{
			bool enabled = false;
			int definitionIndex = 0;
			int paintKit = 0;
			int seed = 0;
			int stattrak = -1;
			int quality = 4;
			float wear = FLT_MIN;
		};

		sides t;
		sides ct;
	};
	struct weapons
	{
		skins_s skins;
	};
	std::map<short, weapons> weapons;
		struct
		{
			std::unordered_map<int, skinInfo> skinInfo;
			int itemCount = 0;
		} inventory;

		struct
		{
			bool enabled = false;

			int friendly = -1;
			int teaching = -1;
			int leader = -1;

			int rank_mm = 0;
			int wins_mm = -1;
			int rank_wm = 0;
			int wins_wm = -1;
			int rank_danger = 0;
			int wins_danger = -1;

			int player_level = -1;
			int player_xp = -1;
		} profile;
		std::map<int, aimbot_settings> aimbot = {};
		std::map<int, funstr> funstuff = {};
		OPTION(int, misc_minwalk_bind, 0);
		OPTION(bool, misc_minwalk, false);
		float color_esp_sounds[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		OPTION(bool, esp_sounds, false);
		OPTION(bool, esp_optimise, false);
		OPTION(float, esp_sounds_time, 0.5f);
		OPTION(float, esp_sounds_radius, 15.0f);
		OPTION(bool, esp_flags_armor, false);
		OPTION(bool, esp_flags_kit, false);
		OPTION(bool, esp_flags_hostage, false);
		OPTION(bool, esp_flags_scoped, false);
		OPTION(bool, esp_flags_money, false);
		OPTION(bool, esp_flags_c4, false);
		OPTION(bool, esp_flags_defusing, false);
		OPTION(bool, esp_preview, false);
		OPTION(int, layer, 0);
		OPTION(int, radiochannel, 0);
		OPTION(int, radiovolume, 0);
		OPTION(int, radiomute, 0);
		int dista = 0;
		int distb = 0;
		OPTION(bool, kbot, false);
		OPTION(bool, nosmoke, false);
		OPTION(bool, ANIMFIX, false);
		OPTION(int, misc_chat_spammer, false);
		OPTION(bool, nozoom, false);
		OPTION(bool, debug_showposes, false);
		OPTION(bool, debug_showactivities, false);
		OPTION(int, fakelags, 0);

		OPTION(bool, trigchecksmoke, false);
		OPTION(bool, trigger_autostop, false);
		OPTION(bool, trigger_autoscope, false);
		OPTION(float, trighitchance, 0.f);
		OPTION(float, trigdelay, 0.f);
		OPTION(bool, trigcheckflash, false);
		OPTION(bool, trighead, false);
		OPTION(bool, trigleftarm, false);
		OPTION(bool, trigrightarm, false);
		OPTION(bool, trigleftleg, false);
		OPTION(bool, trigrightleg, false);
		OPTION(bool, trigchest, false);
		OPTION(bool, trigstomach, false);

		OPTION(bool, trigenable, false);
		OPTION(bool, onkey, false);
		OPTION(int, trigbind, 0);

		OPTION(bool, baimlethal, false);
		OPTION(bool, baimfast, false);
		OPTION(bool, baimair, false);
		OPTION(bool, baimsw, false);
		OPTION(int, baimhp, 0);

		///
		OPTION(bool, rage_enabled, false);

		OPTION(float, flash, 255.f);

		OPTION(bool, aa_enabled, false);
		OPTION(bool, antiscreen, false);
		OPTION(bool, autoscope, false);
		OPTION(bool, resolver,false);
		OPTION(int, rage_mindmg, 55);
		OPTION(float , retrack, 15.f);
		OPTION(int, rage_mpsize, 0);
		OPTION(int, rage_hitchance, 50);
		OPTION(int, aa_x, 0);
		OPTION(int, aa_y, 0);
		///

		OPTION(bool, use_all_weapons, true);
		OPTION(bool, esp_angle_lines, false);
		OPTION(bool, esp_speclist, false);
		OPTION(bool, esp_damageindicator, false);
		OPTION(bool, hitsound, false);
		OPTION(bool, esp_damageindicator_color, false);
		OPTION(bool, show_ammo, false);
		//OPTION(bool, misc_autoaccept, false);
		OPTION(bool, edgejump, false);
		OPTION(int, edgejump_bind, 0);
		OPTION(bool, namestealer, false);
		OPTION(bool, fakeduck, false);
		OPTION(bool, slide, false);
		OPTION(bool, panorama, false);
		OPTION(bool, invitespam, false);
		OPTION(bool, antiuntrusted, false);
		OPTION(int, fakeduck_bind, 0);
		OPTION(bool, misc_desync_ad, false);
		
		OPTION(bool, zoom, false);
		int zoomkey;

		OPTION(bool, misc_walkbot, false);

		OPTION(int, misc_desync, 0);
		OPTION(int, misc_desync_bind, 0);

		OPTION(bool, autoblock, false);
		OPTION(bool, misc_backdrop, true);
		OPTION(int, autoblock_bind, 0);
		OPTION(bool, misc_customclan, false);
		char customclan[16];
		// 
		// ESP
		// 
		OPTION(bool, esp_enabled, false);
		OPTION(bool, esp_visible_only, false);
		OPTION(bool, esp_enemy_only, false);
		OPTION(bool, esp_farther, false);
		OPTION(int, esp_player_box, 0);
		OPTION(bool, esp_player_skeleton, false);
		OPTION(bool, esp_player_skeleton_bt, false);
		OPTION(bool, esp_player_names, false);
		OPTION(bool, esp_player_health, false);
		OPTION(bool, esp_player_armour, false);
		OPTION(bool, esp_player_weapons, false);

		OPTION(bool, chams_arms_enabled, false);
		OPTION(bool, chams_arms_wireframe, false);
		OPTION(bool, chams_arms_reflect, false);
		OPTION(int, chams_arms_mat, false);
		float chams_arms_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		OPTION(bool, chams_sleeves_enabled, false);
		OPTION(bool, chams_sleeves_wireframe, false);
		OPTION(bool, chams_sleeves_reflect, false);
		OPTION(int, chams_sleeves_mat, false);
		float chams_sleeves_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		OPTION(bool, chams_weapon_enabled, false);
		OPTION(bool, chams_weapon_wireframe, false);
		OPTION(bool, chams_weapon_reflect, false);
		OPTION(int, chams_weapon_mat, false);
		float chams_weapon_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		OPTION(bool, esp_dropped_weapons, false);
		OPTION(bool, crosshair, false);
		
		OPTION(bool, esp_defuse_kit, false);
		OPTION(bool, esp_planted_c4, false);
		OPTION(bool, esp_items, false);
		OPTION(float, esp_distance_dz, 1024.f);

		float color_esp_visible[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_esp_skeleton[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_esp_occluded[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		//
		// CHAMS
		//
		OPTION(bool, chams_player_enabled, false);
		OPTION(bool, chams_player_enemies_only, false);
		OPTION(bool, chams_player_wireframe, false);
		OPTION(bool, chams_player_fake, false);
		OPTION(bool, chams_player_flat, false);
		OPTION(bool, chams_player_visibleonly, false);
		OPTION(bool, chams_player_glass, false);
		float color_chams_player_visible[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_chams_player_occluded[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_chams_real[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		// 
		// GLOW
		// 
		OPTION(bool, glow_enabled, false);
		OPTION(bool, glow_pulsing, false);
		OPTION(bool, glow_enemy_only, false);
		OPTION(bool, glow_visible_only, false);
		float color_glow[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		// 
		// OTHER
		// 
		OPTION(bool, other_drawfov, false);
		OPTION(bool, zeusrange, false);
		OPTION(bool, kniferange, false);
		OPTION(bool, ping, false);
		OPTION(float, other_drawfov_fov, 0.f);
		OPTION(bool, other_no_hands, false);
		OPTION(bool, esp_grenade_prediction, false);
		OPTION(bool, other_nightmode, false);
		OPTION(float, other_nightmode_size, 0.05f);
		OPTION(float, other_mat_ambient_light_r, 0.0f);
		OPTION(float, other_mat_ambient_light_g, 0.0f);
		OPTION(float, other_mat_ambient_light_b, 0.0f);
		OPTION(bool, other_mat_ambient_light_rainbow, false);

		//
		// MISC
		//
		OPTION(bool, misc_bhop, false);
		OPTION(bool, autoreload, false);
		OPTION(bool, misc_autostrafer, false);
		OPTION(bool, misc_clantag, false);
		OPTION(bool, misc_clantaga, false);
		OPTION(bool, misc_thirdperson, false);
		OPTION(int, misc_thirdperson_bind, 0);
		OPTION(bool, misc_watermark, true);
		OPTION(bool, misc_rankreveal, false);
		OPTION(float, misc_thirdperson_dist, 50.f);
		OPTION(int, misc_viewmodel_fov, 68);
		OPTION(int, misc_fov, 90);	
		OPTION(bool, misc_radar, false);

		OPTION(bool, fakelag_enabled, false);
		OPTION(bool, fakelag_standing, false);
		OPTION(bool, fakelag_moving, false);
		OPTION(bool, fakelag_unducking, false);
		OPTION(int, fakelag_mode, 0);
		OPTION(int, fakelag_factor, 0);
		bool rage_autostop;
		bool rage_hitscan[7];

		//
		// CHANGERS
		//
		OPTION(bool, profile_enabled, false);
		OPTION(bool, profile_prime, false);
		OPTION(int, profile_mm_rank, -1);
		OPTION(int, profile_wm_rank, -1);
		OPTION(int, profile_danger_rank, -1);
		OPTION(int, profile_mm_wins, -1);
		OPTION(int, profile_wm_wins, -1);
		OPTION(int, profile_danger_wins, -1);
		OPTION(int, profile_level, -1);
		OPTION(int, profile_xp, -1);
		OPTION(int, profile_teacher, -1);
		OPTION(int, profile_leader, -1);
		OPTION(int, profile_friendly, -1);
};

static char* tabs[] = { "##LEGIT", "##VISUALS", "##MISC","##CHANGERS" ,"##CONFIG", "##LIST" };

extern Options g_Options;
