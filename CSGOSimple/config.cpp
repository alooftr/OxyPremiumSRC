#include "config.hpp"
#include "options.hpp"
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "menu.hpp"
#include "helpers/input.hpp"
#include "features/inventory.hpp"

// For ints, chars, floats, and bools
void CConfig::SetupValue(int& value, int def, std::string category, std::string name) { value = def; ints.push_back(new ConfigValue< int >(category, name, &value, def)); }
void CConfig::SetupValue(char* value, char* def, std::string category, std::string name) { value = def; chars.push_back(new ConfigValue< char >(category, name, value, *def)); }
void CConfig::SetupValue(float& value, float def, std::string category, std::string name) { value = def; floats.push_back(new ConfigValue< float >(category, name, &value, def)); }
void CConfig::SetupValue(bool& value, bool def, std::string category, std::string name) { value = def; bools.push_back(new ConfigValue< bool >(category, name, &value, def)); }
void CConfig::SetupColor(float value[4], std::string name) {
	SetupValue(value[0], value[0], ("colors"), name + "_r");
	SetupValue(value[1], value[1], ("colors"), name + "_g");
	SetupValue(value[2], value[2], ("colors"), name + "_b");
	SetupValue(value[3], value[3], ("colors"), name + "_a");
}

void CConfig::SetupLegit() {
	for (auto& [key, val] : weapon_names) {
		SetupValue(g_Options.aimbot[key].enabled, false, (val), ("enabled"));
		SetupValue(g_Options.use_all_weapons, true, (val), ("auto_set"));
		SetupValue(g_Options.aimbot[key].deathmatch, false, (val), ("deathmatch"));
		SetupValue(g_Options.aimbot[key].autopistol, false, (val), ("autopistol"));
		SetupValue(g_Options.aimbot[key].check_smoke, false, (val), ("smoke_check"));
		SetupValue(g_Options.aimbot[key].check_flash, false, (val), ("flash_check"));
		SetupValue(g_Options.aimbot[key].check_jump, false, (val), ("jump_check"));
		SetupValue(g_Options.aimbot[key].autowall, false, (val), ("autowall"));
		SetupValue(g_Options.aimbot[key].antiaimlock, false, (val), ("antiaimlock"));
		SetupValue(g_Options.aimbot[key].silent, false, (val), ("silent"));
		SetupValue(g_Options.aimbot[key].autofire, false, (val), ("autofire"));
		SetupValue(g_Options.aimbot[key].on_key, true, (val), ("on_key"));
		SetupValue(g_Options.aimbot[key].rcs, false, (val), ("rcs"));
		SetupValue(g_Options.aimbot[key].rcs_fov_enabled, false, (val), ("rcs_fov_enabled"));
		SetupValue(g_Options.aimbot[key].rcs_smooth_enabled, false, (val), ("rcs_smooth_enabled"));
		SetupValue(g_Options.aimbot[key].backtrack.enabled, false, (val), ("backtrack"));
		SetupValue(g_Options.aimbot[key].backtrack.ticks, 6, (val), ("backtrack_ticks"));
		SetupValue(g_Options.aimbot[key].only_in_zoom, false, (val), ("only_in_zoom"));
		SetupValue(g_Options.aimbot[key].humanize, false, (val), ("humanize"));
		SetupValue(g_Options.aimbot[key].curviness, 0.f, (val), ("curviness"));
		SetupValue(g_Options.aimbot[key].autofire_key, 1, (val), ("autofire_key"));
		SetupValue(g_Options.aimbot[key].key, 1, (val), ("key"));
		SetupValue(g_Options.aimbot[key].aim_type, 1, (val), ("aim_type"));
		SetupValue(g_Options.aimbot[key].priority, 0, (val), ("priority"));
		SetupValue(g_Options.aimbot[key].fov_type, 0, (val), ("fov_type"));
		SetupValue(g_Options.aimbot[key].rcs_type, 0, (val), ("rcs_type"));
		SetupValue(g_Options.aimbot[key].smooth_type, 0, (val), ("smooth_type"));
		SetupValue(g_Options.aimbot[key].hitbox, 1, (val), ("hitbox"));
		SetupValue(g_Options.aimbot[key].fov, 0.f, (val), ("fov"));
		SetupValue(g_Options.aimbot[key].silent_fov, 0.f, (val), ("silent_fov"));
		SetupValue(g_Options.aimbot[key].rcs_fov, 0.f, (val), ("rcs_fov"));
		SetupValue(g_Options.aimbot[key].smooth, 1, (val), ("smooth"));
		SetupValue(g_Options.aimbot[key].rcs_smooth, 1, (val), ("rcs_smooth"));
		SetupValue(g_Options.aimbot[key].shot_delay, 0, (val), ("shot_delay"));
		SetupValue(g_Options.aimbot[key].kill_delay, 0, (val), ("kill_delay"));
		SetupValue(g_Options.aimbot[key].rcs_x, 100, (val), ("rcs_x"));
		SetupValue(g_Options.aimbot[key].rcs_y, 100, (val), ("rcs_y"));
		SetupValue(g_Options.aimbot[key].rcs_start, 1, (val), ("rcs_start"));
		SetupValue(g_Options.aimbot[key].min_damage, 1, (val), ("min_damage"));

		SetupValue(g_Options.trigenable, false, (val), ("trigger"));
		SetupValue(g_Options.trigbind, 1, (val), ("trigger_bind"));
		SetupValue(g_Options.onkey, false, (val), ("trigger_onkey"));
		SetupValue(g_Options.trigchecksmoke, false, (val), ("trigger_smoke"));
		SetupValue(g_Options.trigcheckflash, false, (val), ("trigger_flash"));
		SetupValue(g_Options.trighitchance, 0.f, (val), ("trigger_hitchance"));
		SetupValue(g_Options.trigdelay, 0.f, (val), ("trigger_delay"));
		SetupValue(g_Options.trighead, false, (val), ("trigger_head"));
		SetupValue(g_Options.trigchest, false, (val), ("trigger_chest"));
		SetupValue(g_Options.trigstomach, false, (val), ("trigger_stomach"));
		SetupValue(g_Options.trigleftarm, false, (val), ("trigger_la"));
		SetupValue(g_Options.trigrightarm, false, (val), ("trigger_ra"));
		SetupValue(g_Options.trigleftleg, false, (val), ("trigger_ll"));
		SetupValue(g_Options.trigrightleg, false, (val), ("trigger_rl"));
	}
}

void CConfig::SetupVisuals() {
	SetupValue(g_Options.esp_enabled, false, ("esp"), ("enabled"));
	SetupValue(g_Options.esp_visible_only, false, ("esp"), ("enabled_visible"));
	SetupValue(g_Options.esp_enemy_only, false, ("esp"), ("enemy_only"));
	SetupValue(g_Options.esp_player_skeleton, false, ("esp"), ("player_skeleton"));
	SetupValue(g_Options.esp_player_skeleton_bt, false, ("esp"), ("player_skeleton_backtrack"));
	SetupValue(g_Options.esp_player_box, 0, ("esp"), ("player_box"));
	SetupValue(g_Options.esp_player_names, false, ("esp"), ("player_names"));
	SetupValue(g_Options.esp_player_health, false, ("esp"), ("player_health"));
	SetupValue(g_Options.esp_player_armour, false, ("esp"), ("player_armour"));
	SetupValue(g_Options.esp_player_weapons, false, ("esp"), ("player_weapons"));
	SetupValue(g_Options.esp_dropped_weapons, false, ("esp"), ("dropped_weapons"));
	SetupValue(g_Options.show_ammo, false, ("esp"), ("ammo"));
	SetupValue(g_Options.esp_flags_armor, false, ("esp"), ("flags_armor"));
	SetupValue(g_Options.esp_flags_c4, false, ("esp"), ("flags_c4"));
	SetupValue(g_Options.esp_flags_kit, false, ("esp"), ("flags_kit"));
	SetupValue(g_Options.esp_flags_defusing, false, ("esp"), ("flags_defusing"));
	SetupValue(g_Options.esp_flags_scoped, false, ("esp"), ("flags_scoped"));
	SetupValue(g_Options.esp_flags_money, false, ("esp"), ("flags_money"));
	SetupValue(g_Options.esp_flags_hostage, false, ("esp"), ("flags_hostage"));
	SetupValue(g_Options.esp_defuse_kit, false, ("esp"), ("defuse_kit"));
	SetupValue(g_Options.esp_planted_c4, false, ("esp"), ("planted_c4"));
	SetupValue(g_Options.esp_items, false, ("esp"), ("items"));
	SetupValue(g_Options.esp_grenade_prediction, false, ("esp"), ("grenade_prediction"));
	SetupValue(g_Options.crosshair, false, ("esp"), ("crosshair"));
	SetupValue(g_Options.nozoom, false, ("esp"), ("nozoom"));
	SetupValue(g_Options.esp_sounds, false, ("esp"), ("sounds"));
	SetupValue(g_Options.esp_sounds_time, 0.5f, ("esp"), ("sounds_time"));
	SetupValue(g_Options.esp_sounds_radius, 15.0f, ("esp"), ("sounds_radius"));
	SetupValue(g_Options.esp_angle_lines, false, ("esp"), ("angle_lines"));
	SetupValue(g_Options.hitsound, false, ("esp"), ("hitsound"));
	SetupValue(g_Options.esp_speclist, false, ("esp"), ("speclist"));

	SetupValue(g_Options.glow_enabled, false, ("glow"), ("enabled"));
	SetupValue(g_Options.glow_visible_only, false, ("glow"), ("visible_only"));
	SetupValue(g_Options.glow_enemy_only, false, ("glow"), ("enemies_only"));
	SetupValue(g_Options.glow_pulsing, false, ("glow"), ("pulsing"));
	SetupValue(g_Options.chams_player_enabled, false, ("chams"), ("player_enabled"));
	SetupValue(g_Options.chams_player_enemies_only, false, ("chams"), ("player_enemies_only"));
	SetupValue(g_Options.chams_player_visibleonly, false, ("chams"), ("player_visibleonly"));
	SetupValue(g_Options.chams_player_wireframe, false, ("chams"), ("player_wireframe"));
	SetupValue(g_Options.chams_player_flat, false, ("chams"), ("player_flat"));
	SetupValue(g_Options.chams_player_fake, false, ("chams"), ("player_fake"));
	SetupValue(g_Options.chams_arms_enabled, false, ("chams"), ("chams_arms"));
	SetupValue(g_Options.chams_arms_mat, false, ("chams"), ("chams_arms_mat"));
	SetupValue(g_Options.chams_arms_wireframe, false, ("chams"), ("chams_arms_wireframe"));
	SetupValue(g_Options.chams_arms_reflect, false, ("chams"), ("chams_arms_reflect"));
	SetupValue(g_Options.chams_sleeves_enabled, false, ("chams"), ("chams_sleeves"));
	SetupValue(g_Options.chams_sleeves_mat, false, ("chams"), ("chams_sleeves_mat"));
	SetupValue(g_Options.chams_sleeves_wireframe, false, ("chams"), ("chams_sleeves_wireframe"));
	SetupValue(g_Options.chams_sleeves_reflect, false, ("chams"), ("chams_sleeves_reflect"));
	SetupValue(g_Options.chams_weapon_enabled, false, ("chams"), ("chams_weapon"));
	SetupValue(g_Options.chams_weapon_mat, false, ("chams"), ("chams_weapon_mat"));
	SetupValue(g_Options.chams_weapon_wireframe, false, ("chams"), ("chams_weapon_wireframe"));
	SetupValue(g_Options.chams_weapon_reflect, false, ("chams"), ("chams_weapon_reflect"));

	SetupValue(g_Options.other_nightmode, false, ("other"), ("nightmode"));
	SetupValue(g_Options.other_nightmode_size, 0, ("other"), ("nightmode_size"));
	SetupValue(g_Options.other_mat_ambient_light_r, false, ("other"), ("mat_ambient_light_r"));
	SetupValue(g_Options.other_mat_ambient_light_g, false, ("other"), ("mat_ambient_light_g"));
	SetupValue(g_Options.other_mat_ambient_light_b, false, ("other"), ("mat_ambient_light_b"));
	SetupValue(g_Options.other_mat_ambient_light_rainbow, false, ("other"), ("mat_ambient_light_rainbow"));

	SetupValue(g_Options.other_drawfov, false, ("other"), ("drawfov"));
	SetupValue(g_Options.other_no_hands, false, ("other"), ("no_hands"));
	SetupValue(g_Options.nosmoke, false, ("other"), ("no_smoke"));
	SetupValue(g_Options.flash, 255.f, ("other"), ("no_flash"));
	SetupValue(g_Options.esp_damageindicator, false, ("other"), ("damage_indicator"));
	SetupValue(g_Options.esp_damageindicator_color, false, ("other"), ("show_hp"));
	SetupValue(g_Options.zeusrange, false, ("other"), ("zeusrange"));
}

void CConfig::SetupMisc() {
	SetupValue(g_Options.misc_bhop, false, ("misc"), ("bhop"));
	SetupValue(g_Options.misc_autostrafer, false, ("misc"), ("autostrafe"));
	//SetupValue(g_Options.misc_autoaccept, false, ("misc"), ("autoaccept"));
	SetupValue(g_Options.misc_watermark, true, ("misc"), ("watermark"));
	SetupValue(g_Options.namestealer, false, ("misc"), ("name_stealer"));
	SetupValue(g_Options.autoblock, false, ("misc"), ("autoblock"));
	SetupValue(g_Options.autoblock_bind, 0, ("misc"), ("autoblock_bind"));
	SetupValue(g_Options.edgejump, false, ("misc"), ("edgejump"));
	SetupValue(g_Options.edgejump_bind, 0, ("misc"), ("edgejump_bind"));
	SetupValue(g_Options.misc_radar, false, ("misc"), ("radar"));
	SetupValue(g_Options.misc_backdrop, true, ("misc"), ("backdrop"));
	SetupValue(g_Options.misc_thirdperson, false, ("misc"), ("thirdperson"));
	SetupValue(g_Options.misc_thirdperson_bind, 0, ("misc"), ("thirdperson_bind"));
	SetupValue(g_Options.misc_rankreveal, false, ("misc"), ("showranks"));
	SetupValue(g_Options.misc_clantag, false, ("misc"), ("clantag"));
	SetupValue(g_Options.misc_clantaga, false, ("misc"), ("clantaga"));
	SetupValue(g_Options.misc_customclan, false, ("misc"), ("customclan"));
	SetupValue(g_Options.customclan, "", ("misc"), ("custom_clan"));
	SetupValue(g_Options.misc_desync, 0, ("misc"), ("desync"));
	SetupValue(g_Options.misc_desync_ad, false, ("misc"), ("desync_ad"));
	SetupValue(g_Options.misc_desync_bind, 0, ("misc"), ("desync_bind"));
	SetupValue(g_Options.resolver, false, ("misc"), ("resolver"));
	SetupValue(g_Options.misc_chat_spammer, 0, ("misc"), ("spammer"));
	SetupValue(g_Options.antiuntrusted, false, ("misc"), ("antiuntrusted"));
	SetupValue(g_Options.slide, false, ("misc"), ("slide"));
	SetupValue(g_Options.kbot, false, ("misc"), ("knife_bot"));
	SetupValue(g_Options.esp_optimise, false, ("misc"), ("optimize"));
	SetupValue( g_Options.fakeduck, false, ( "misc" ), ( "fakeduck_bind" ) );
	SetupValue( g_Options.fakeduck_bind, 0, ( "misc" ), ( "fakeduck_bind" ) );
	SetupValue(g_Options.misc_thirdperson_dist, 50.f, ("misc"), ("thirdperson_dist"));
	SetupValue(g_Options.misc_fov, 90, ("misc"), ("override_fov"));
	SetupValue(g_Options.misc_viewmodel_fov, 68, ("misc"), ("viewmodel_fov"));


	SetupValue(g_Options.fakelag_enabled, false, ("fakelag"), ("enabled"));
	SetupValue(g_Options.fakelag_standing, false, ("fakelag"), ("standing"));
	SetupValue(g_Options.fakelag_moving, false, ("fakelag"), ("moving"));
	SetupValue(g_Options.fakelag_unducking, false, ("fakelag"), ("unducking"));
	SetupValue(g_Options.fakelag_mode, 0, ("fakelag"), ("mode"));
	SetupValue(g_Options.fakelag_factor, 0, ("fakelag"), ("factor"));
}

void CConfig::SetupChangers() {
	SetupValue(g_Options.profile.enabled, false, ("profile"), ("enabled"));
	SetupValue(g_Options.profile.player_level, 0, ("profile"), ("level"));
	SetupValue(g_Options.profile.player_xp, 0, ("profile"), ("xp"));
	SetupValue(g_Options.profile_prime, false, ("profile"), ("prime"));
	SetupValue(g_Options.profile.rank_mm, 0, ("profile"), ("rank_mm"));
	SetupValue(g_Options.profile.rank_mm, 0, ("profile"), ("rank_wm"));
	SetupValue(g_Options.profile.rank_danger, 0, ("profile"), ("rank_danger"));
	SetupValue(g_Options.profile.wins_mm, 0, ("profile"), ("wins_mm"));
	SetupValue(g_Options.profile.wins_mm, 0, ("profile"), ("wins_wm"));
	SetupValue(g_Options.profile.wins_danger, 0, ("profile"), ("wins_danger"));
	SetupValue(g_Options.profile.teaching, 0, ("profile"), ("teaching"));
	SetupValue(g_Options.profile.leader, 0, ("profile"), ("leader"));
	SetupValue(g_Options.profile.friendly, 0, ("profile"), ("friendly"));
}

void CConfig::SetupColors() {
	SetupColor(g_Options.color_esp_visible, "color_esp_enemy_visible");
	SetupColor(g_Options.color_esp_occluded, "color_esp_enemy_occluded");
	SetupColor(g_Options.color_glow, "color_glow");
	SetupColor(g_Options.color_chams_player_visible, "color_chams_player_enemy_visible");
	SetupColor(g_Options.color_chams_player_occluded, "color_chams_player_enemy_occluded");
	SetupColor(g_Options.chams_arms_color, "color_arms_chams");
	SetupColor(g_Options.chams_weapon_color, "color_weapon_chams");
	SetupColor(g_Options.chams_sleeves_color, "color_sleeves_chams");
	SetupColor(g_Options.color_chams_real, "color_chams_real");
	SetupColor(g_Options.color_esp_sounds, "color_esp_sounds");
	SetupColor(g_Options.color_esp_skeleton, "color_esp_skeleton");
}

void CConfig::Setup() {
	CConfig::SetupLegit();
	CConfig::SetupVisuals();
	CConfig::SetupMisc();
	CConfig::SetupChangers();
	CConfig::SetupColors();
}

#include "helpers/utils.hpp"
void CConfig::Save(const std::string& name) {
	if (name.empty())
		return;

	CreateDirectoryA(u8"C:\\Muten\\", NULL);
	std::string file = u8"C:\\Muten\\" + name;

	for (auto value : ints) {
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	}

	for (auto value : floats) WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	for (auto value : bools) WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());

	WritePrivateProfileStringA("InventoryChanger", "Count", std::to_string(g_Options.inventory.itemCount).c_str(), file.c_str());

	int i = 0;
	for (auto weapon : g_InventorySkins)
	{
		if (!weapon.second.wId || !weapon.second.paintKit)
			continue;

		std::string itemid = "inventory" + std::to_string(i) + "_id";
		std::string paintkitstr = "inventory" + std::to_string(i) + "_paintkit";
		std::string wearkitstr = "inventory" + std::to_string(i) + "_wear";
		std::string seedkitstr = "inventory" + std::to_string(i) + "_seed";
		std::string inusetstr = "inventory" + std::to_string(i) + "_inuset";
		std::string inusectstr = "inventory" + std::to_string(i) + "_inusect";

		WritePrivateProfileStringA("InventoryChanger", itemid.c_str(), std::to_string(weapon.second.wId).c_str(), file.c_str());
		WritePrivateProfileStringA("InventoryChanger", paintkitstr.c_str(), std::to_string(weapon.second.paintKit).c_str(), file.c_str());
		WritePrivateProfileStringA("InventoryChanger", wearkitstr.c_str(), std::to_string(weapon.second.wear).c_str(), file.c_str());
		WritePrivateProfileStringA("InventoryChanger", seedkitstr.c_str(), std::to_string(weapon.second.seed).c_str(), file.c_str());
		WritePrivateProfileStringA("InventoryChanger", inusetstr.c_str(), weapon.second.in_use_t ? "true" : "false", file.c_str());
		WritePrivateProfileStringA("InventoryChanger", inusectstr.c_str(), weapon.second.in_use_ct ? "true" : "false", file.c_str());
		i++;
	}
}

void CConfig::Load(const std::string& name) {
	if (name.empty())
		return;

	CreateDirectoryA(u8"C:\\Muten\\", NULL);
	std::string file = u8"C:\\Muten\\" + name;

	char value_l[32] = { '\0' };
	for (auto value : ints) {
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "0", value_l, 32, file.c_str()); *value->value = atoi(value_l);
	}

	for (auto value : floats) {
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "0.0f", value_l, 32, file.c_str()); *value->value = atof(value_l);
	}

	for (auto value : bools) {
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "false", value_l, 32, file.c_str()); *value->value = !strcmp(value_l, "true");
	}
	g_InventorySkins.clear();

	GetPrivateProfileStringA("InventoryChanger", "Count", "", value_l, 32, file.c_str());
	g_Options.inventory.itemCount = atoi(value_l);

	for (int i = 0; i < g_Options.inventory.itemCount; i++)
	{
		std::string itemid = "inventory" + std::to_string(i) + "_id";
		std::string paintkitstr = "inventory" + std::to_string(i) + "_paintkit";
		std::string wearkitstr = "inventory" + std::to_string(i) + "_wear";
		std::string seedkitstr = "inventory" + std::to_string(i) + "_seed";
		std::string inusetstr = "inventory" + std::to_string(i) + "_inuset";
		std::string inusectstr = "inventory" + std::to_string(i) + "_inusect";

		wskin skinInfo;

		GetPrivateProfileStringA("InventoryChanger", itemid.c_str(), "", value_l, 32, file.c_str());
		skinInfo.wId = atoi(value_l);
		GetPrivateProfileStringA("InventoryChanger", paintkitstr.c_str(), "", value_l, 32, file.c_str());
		skinInfo.paintKit = atoi(value_l);
		GetPrivateProfileStringA("InventoryChanger", wearkitstr.c_str(), "", value_l, 32, file.c_str());
		skinInfo.wear = atof(value_l);
		GetPrivateProfileStringA("InventoryChanger", seedkitstr.c_str(), "", value_l, 32, file.c_str());
		skinInfo.seed = atoi(value_l);
		GetPrivateProfileStringA("InventoryChanger", inusetstr.c_str(), "", value_l, 32, file.c_str());
		skinInfo.in_use_t = !strcmp(value_l, "true");
		GetPrivateProfileStringA("InventoryChanger", inusectstr.c_str(), "", value_l, 32, file.c_str());
		skinInfo.in_use_ct = !strcmp(value_l, "true");

		g_InventorySkins.insert({ RandomInt(20000, 200000), skinInfo });
	}

	Protobuf::SendClientHello();
	Protobuf::SendMatchmakingClient2GCHello();

	g_ClientState->ForceFullUpdate();
}

CConfig* Config = new CConfig();
