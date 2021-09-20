﻿#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#define NOMINMAX

#include <Windows.h>
#include <chrono>
#include "Menu.hpp"

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "Guard.h"
#include "ui.hpp"
#include "config.hpp"
#include "features/Notification.h"
#include "render.hpp"
#include "helpers/proto/protobuf.hpp"
#include "../CSGOSimple/pasta.h"
#include <experimental/filesystem>
#include "features/skins.hpp"

namespace fs = std::experimental::filesystem;
std::string imData;
std::string link = "";
IDirect3DTexture9* skinImage = nullptr;

void downloadImage()
{
	imData = Preview::DownloadBytes(link.c_str());
	D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, imData.data(), imData.length(), 512, 384, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &skinImage);
}

const char* const KeyNames[] = {
	"None",
	"Mouse1",
	"Mouse2",
	"Esc",
	"Mouse3",
	"Mouse4",
	"Mouse5",
	"None",
	"Back",
	"Tab",
	"None",
	"None",
	"Backspace",
	"Enter",
	"None",
	"None",
	"Shift",
	"Ctrl",
	"Alt",
	"Pause Break",
	"CapsLock",
	"VK_KANA",
	"Unknown",
	"VK_JUNJA",
	"VK_FINAL",
	"VK_KANJI",
	"Unknown",
	"Esc",
	"VK_CONVERT",
	"VK_NONCONVERT",
	"VK_ACCEPT",
	"VK_MODECHANGE",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"VK_SELECT",
	"VK_PRINT",
	"VK_EXECUTE",
	"Print Screen",
	"Ins",
	"Del",
	"VK_HELP",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"Left Windows",
	"Right Windows",
	"VK_APPS",
	"Unknown",
	"VK_SLEEP",
	"NUMPAD0",
	"NUMPAD1",
	"NUMPAD2",
	"NUMPAD3",
	"NUMPAD4",
	"NUMPAD5",
	"NUMPAD6",
	"NUMPAD7",
	"NUMPAD8",
	"NUMPAD9",
	"Multiply",
	"+",
	"Separator",
	"Subtract",
	"-",
	"/",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"F16",
	"F17",
	"F18",
	"F19",
	"F20",
	"F21",
	"F22",
	"F23",
	"F24",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Num Lock",
	"Scroll lock",
	"VK_OEM_NEC_EQUAL",
	"VK_OEM_FJ_MASSHOU",
	"VK_OEM_FJ_TOUROKU",
	"VK_OEM_FJ_LOYA",
	"VK_OEM_FJ_ROYA",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Left Shift",
	"Right Shift",
	"Left Ctrl",
	"Right Ctrl",
	"Left Alt",
	"Right Alt"
};

#pragma region RenderTabs
template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
	bool values[N] = { false };

	values[activetab] = true;

	for (auto i = 0; i < N; ++i) {
		if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
			activetab = i;
		}
		if (sameline && i < N - 1)
			ImGui::SameLine();
	}
}


static bool IsKeyPressedMap(ImGuiKey key, bool repeat = true)
{
	const int key_index = GImGui->IO.KeyMap[key];
	return (key_index >= 0) ? ImGui::IsKeyPressed(key_index, repeat) : false;
}

bool KeyBind(const char* label, int& k, const ImVec2& size_arg = ImVec2(0, 0))
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const ImRect total_bb(window->DC.CursorPos, frame_bb.Max);

	ImGui::ItemSize(total_bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	const bool focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id);
	//const bool focus_requested_by_code = focus_requested && (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent);
	//const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

	const bool hovered = ImGui::ItemHoverable(frame_bb, id);

	if (hovered) {
		ImGui::SetHoveredID(id);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool user_clicked = hovered && io.MouseClicked[0];

	if (focus_requested || user_clicked) {
		if (g.ActiveId != id) {
			// Start edition
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			k = 0;
		}
		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	}
	else if (io.MouseClicked[0]) {
		// Release focus when we click outside
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	bool value_changed = false;
	int key = k;

	if (g.ActiveId == id) {
		for (auto i = 0; i < 5; i++) {
			if (io.MouseDown[i]) {
				switch (i) {
				case 0:
					key = VK_LBUTTON;
					break;
				case 1:
					key = VK_RBUTTON;
					break;
				case 2:
					key = VK_MBUTTON;
					break;
				case 3:
					key = VK_XBUTTON1;
					break;
				case 4:
					key = VK_XBUTTON2;
					break;
				}
				value_changed = true;
				ImGui::ClearActiveID();
			}
		}
		if (!value_changed) {
			for (auto i = VK_BACK; i <= VK_RMENU; i++) {
				if (io.KeysDown[i]) {
					key = i;
					value_changed = true;
					ImGui::ClearActiveID();
				}
			}
		}

		if (IsKeyPressedMap(ImGuiKey_Escape)) {
			k = 0;
			ImGui::ClearActiveID();
		}
		else {
			k = key;
		}
	}

	// Render
	// Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is Set 'buf' might still be the old value. We Set buf to NULL to prevent accidental usage from now on.

	char buf_display[64] = "None";

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg]), true, style.FrameRounding);

	if (k != 0 && g.ActiveId != id) {
		strcpy_s(buf_display, KeyNames[k]);
	}
	else if (g.ActiveId == id) {
		strcpy_s(buf_display, "<Press a key>");
	}

	const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
	ImVec2 render_pos = frame_bb.Min + style.FramePadding;
	ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, style.ButtonTextAlign, &clip_rect);
	//RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, GetColorU32(ImGuiCol_Text), style.ButtonTextAlign, &clip_rect);
	//draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, NULL, 0.0f, &clip_rect);

	if (label_size.x > 0)
		ImGui::RenderText(ImVec2(total_bb.Max.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}

void Kick(int userid)
{
	char command[1024]; sprintf(command, "callvote kick %d", userid);

	g_EngineClient->ExecuteClientCmd(command);
}

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "features/visuals.hpp"
#include "features/inventory.hpp"

static char* boxes[] = { "Off", "Regular", "Edge", "Rounded" };

namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
	{
		auto clr = ImVec4{
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};

		if (ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
			v->SetColor(clr.x, clr.y, clr.z, clr.w);
			return true;
		}
		return false;
	}
	inline bool ColorEdit3(const char* label, Color* v)
	{
		return ColorEdit4(label, v, false);
	}
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h)
{
	bool values[N] = { false };
	values[activetab] = true;
	for (auto i = 0; i < N; ++i) {
		if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) activetab = i;
		if (i < N - 1) ImGui::SameLine();
	}
}

void RenderEspTab()
{
	static int   visuals_tab = 0;
	static char* visuals_tabs[] = { "ESP", "GLOW", "CHAMS", "OTHER" };

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	render_tabs(visuals_tabs, visuals_tab, (ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().WindowPadding.x * 2.0f) / _countof(visuals_tabs), 25.0f);
	ImGui::PopStyleVar();

	ImGui::BeginGroupBox("##visuals_body");
	{
		//ImGui::Checkbox("Esp Preview", &g_Options.esp_preview);
		if (visuals_tab == 0)
		{
			ImGui::Columns(3, nullptr, true);
			ImGui::BeginChild("##1241223124");
			ImGui::Checkbox("Enabled##1", &g_Options.esp_enabled);
			ImGui::Checkbox("Visible only", &g_Options.esp_visible_only);
			ImGui::Checkbox("Enemy only", &g_Options.esp_enemy_only);
			//ImGui::Checkbox("Extended ESP", &g_Options.esp_farther);
			ImGui::Combo("Box", &g_Options.esp_player_box, boxes, IM_ARRAYSIZE(boxes), 4);
			ImGui::Checkbox("Skeleton", &g_Options.esp_player_skeleton);
			ImGui::Checkbox("Backtrack Skeleton", &g_Options.esp_player_skeleton_bt);
			ImGui::Checkbox("Name", &g_Options.esp_player_names);
			ImGui::Checkbox("Health", &g_Options.esp_player_health);
			ImGui::Checkbox("Armour", &g_Options.esp_player_armour);
			ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
			ImGui::Checkbox("Ammo", &g_Options.show_ammo);
			if (ImGui::BeginCombo("Flags", "..."))
			{
				ImGui::Selectable(("Armor"), &g_Options.esp_flags_armor, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable(("Defusing"), &g_Options.esp_flags_defusing, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable(("Defuse Kit"), &g_Options.esp_flags_kit, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable(("Hostage Carrier"), &g_Options.esp_flags_hostage, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable(("Scoped"), &g_Options.esp_flags_scoped, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable(("Money"), &g_Options.esp_flags_money, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
				ImGui::Selectable(("Bomb"), &g_Options.esp_flags_c4, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);


				ImGui::EndCombo();
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("##124122312sss4");
			ImGui::Checkbox("Dropped Weapons", &g_Options.esp_dropped_weapons);
			ImGui::Checkbox("Crosshair", &g_Options.crosshair);
			ImGui::Checkbox("Remove Scope", &g_Options.nozoom);
			ImGui::Checkbox("Defuse Kit", &g_Options.esp_defuse_kit);
			ImGui::Checkbox("Planted C4", &g_Options.esp_planted_c4);
			ImGui::Checkbox("Item Esp", &g_Options.esp_items);
			if (g_Options.esp_items)
				ImGui::SliderFloat("Distance", &g_Options.esp_distance_dz, 64.0f, 8192.f, "%.1fu");
			ImGui::EndChild();
			ImGui::NextColumn();

			ImGui::PushItemWidth(100);
			ImGui::BeginChild("##12412232124");
			ImGui::ColorEdit3("Visible", g_Options.color_esp_visible);
			ImGui::ColorEdit3("Occluded", g_Options.color_esp_occluded);
			ImGui::ColorEdit4("Skeleton", g_Options.color_esp_skeleton);
			ImGui::EndChild();
			ImGui::PopItemWidth();

			ImGui::Columns(1, nullptr, false);
		}

		else if (visuals_tab == 1)
		{
			ImGui::Columns(2, nullptr, true);
			ImGui::BeginChild("##1241223");
			ImGui::Checkbox("Enabled", &g_Options.glow_enabled);
			ImGui::Checkbox("Visible Only", &g_Options.glow_visible_only);
			ImGui::Checkbox("Pulsing", &g_Options.glow_pulsing);
			ImGui::Checkbox("Enemy only", &g_Options.glow_enemy_only);
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("##124122c3");
			ImGui::PushItemWidth(100);
			ImGui::ColorEdit4("Color", g_Options.color_glow);
			ImGui::PopItemWidth();
			ImGui::EndChild();
			ImGui::Columns(1, nullptr, false);
		}

		else if (visuals_tab == 2)
		{
			ImGui::Columns(2, nullptr, true);
			ImGui::BeginChild("##1241223");
			ImGui::Checkbox("Enabled##2", &g_Options.chams_player_enabled);
			ImGui::Checkbox("Enemy only", &g_Options.chams_player_enemies_only);	
			ImGui::Checkbox("Wireframe", &g_Options.chams_player_wireframe);
			ImGui::Checkbox("Flat", &g_Options.chams_player_flat);
			ImGui::Checkbox("Visible only", &g_Options.chams_player_visibleonly);

			ImGui::ColorEdit4("Visible", g_Options.color_chams_player_visible);
			ImGui::ColorEdit4("Occluded", g_Options.color_chams_player_occluded);
			ImGui::Checkbox("Real Chams", &g_Options.chams_player_fake);
			ImGui::ColorEdit4("Real Chams", g_Options.color_chams_real);
			ImGui::EndChild();

			char* chamstypes[] = { "Material", "Flat", "Glass", "Crystal", "Gold", "Dark Crystal", "Silver" };

			ImGui::NextColumn();
			ImGui::BeginChild("##124122243");
			ImGui::Text("Arms");
			ImGui::Checkbox("Enabled", &g_Options.chams_arms_enabled);
 
			ImGui::Combo("Materials##1", &g_Options.chams_arms_mat, chamstypes,IM_ARRAYSIZE(chamstypes));
			ImGui::Checkbox("Wireframe", &g_Options.chams_arms_wireframe);
			ImGui::Checkbox("Reflective##35215", &g_Options.chams_arms_reflect);
			ImGui::ColorEdit4("Color", g_Options.chams_arms_color);

			ImGui::Separator();

			ImGui::Text("Sleeves");
			ImGui::Checkbox("Enabled##23", &g_Options.chams_sleeves_enabled);

			ImGui::Combo("Materials##441", &g_Options.chams_sleeves_mat, chamstypes, IM_ARRAYSIZE(chamstypes));
			ImGui::Checkbox("Wireframe##777", &g_Options.chams_sleeves_wireframe);
			ImGui::Checkbox("Reflective##35415", &g_Options.chams_sleeves_reflect);
			ImGui::ColorEdit4("Color##2", g_Options.chams_sleeves_color);

			ImGui::Separator();

			ImGui::Text("Weapon");
			ImGui::Checkbox("Enabled##293", &g_Options.chams_weapon_enabled);

			ImGui::Combo("Materials##21", &g_Options.chams_weapon_mat, chamstypes, IM_ARRAYSIZE(chamstypes));
			ImGui::Checkbox("Wireframe##3515", &g_Options.chams_weapon_wireframe);
			ImGui::Checkbox("Reflective##3515", &g_Options.chams_weapon_reflect);
			ImGui::ColorEdit4("Color##92", g_Options.chams_weapon_color);

			ImGui::EndChild();
			ImGui::Columns(1, nullptr, false);
		}
		else if (visuals_tab == 3)
		{
			ImGui::Columns(2, nullptr, true);
			ImGui::BeginChild("##1241ss223");
			ImGui::Checkbox("Draw FOV", &g_Options.other_drawfov);
			ImGui::Checkbox("No hands", &g_Options.other_no_hands);
			ImGui::Checkbox("Grenade Prediction", &g_Options.esp_grenade_prediction);
			ImGui::Checkbox("Angle Lines", &g_Options.esp_angle_lines);

			ImGui::Checkbox("Spectator List", &g_Options.esp_speclist);
			ImGui::Checkbox("Damage Indicator", &g_Options.esp_damageindicator);
			if (g_Options.esp_damageindicator)
				ImGui::Checkbox("Show HP", &g_Options.esp_damageindicator_color);
			ImGui::Checkbox("Hit Sound", &g_Options.hitsound);

			ImGui::Checkbox("Sound ESP", &g_Options.esp_sounds);
			ImGui::SliderFloat("Duration", &g_Options.esp_sounds_time, 0.5f, 3.0f, "%.3f ms");
			ImGui::SliderFloat("Radius", &g_Options.esp_sounds_radius, 15.0f, 150.0f, "%.1f");
			ImGui::SameLine();
			ImGui::ColorEdit4("##Sound color", g_Options.color_esp_sounds, ImGuiColorEditFlags_NoInputs);
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("##1241ssasd223");
			//ImGui::Checkbox("Disable PanoramaBlur", &g_Options.panorama);
			//ImGui::Checkbox("Ping Indicator", &g_Options.ping);
			ImGui::Checkbox("Zeus Range", &g_Options.zeusrange);
		//	ImGui::Checkbox("Knife Range", &g_Options.kniferange);
			//ImGui::Checkbox("Nightmode", &g_Options.other_nightmode);
			//ImGui::SliderFloat("Nightmode", &g_Options.other_nightmode_size, 0, 100, "%.3f");
			ImGui::Checkbox("Rainbow World", &g_Options.other_mat_ambient_light_rainbow);
			ImGui::SliderInt("Viewmodel FOV", &g_Options.misc_viewmodel_fov, 68, 120, "%d");
			ImGui::SliderInt("Override FOV", &g_Options.misc_fov, 90, 150, "%d");
			ImGui::Checkbox("No Smoke", &g_Options.nosmoke);
			ImGui::SliderFloat("Flash Ammount", &g_Options.flash, 0.f, 255.f);
			ImGui::Checkbox("Third Person", &g_Options.misc_thirdperson);
			ImGui::SliderFloat("Distance", &g_Options.misc_thirdperson_dist, 50.f, 500.f);

			KeyBind("Bind", g_Options.misc_thirdperson_bind);

			ImGui::EndChild();
			ImGui::Columns(1, nullptr, false);
		}
	}
	ImGui::EndGroupBox();

}
void do_queue();

void RenderMiscTab()
{
	auto& style = ImGui::GetStyle();
	static const char* Spammers[]{ "None", "Breaker Chat", "Muten" };

	const char* items[] = { "Off", "Minimum", "Maximum" };
	
	const char* radio[] = {
		("Off"),
		("Rock"),
		("Techno"),
		("HvH"),
		("Teatime"),
		("Clubtime"),
		("Housetime"),
		("Iluvradio"),
		("8bit"),
	};
	ImGui::Columns(2, nullptr, true);
	ImGui::BeginChild("##11");

	ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
	ImGui::Checkbox("Auto strafer", &g_Options.misc_autostrafer);
	if(g_Options.misc_autostrafer)
		ImGui::SliderFloat("Retrack Speed", &g_Options.retrack, 1.f, 15.f);

	ImGui::Checkbox("Watermark", &g_Options.misc_watermark);
	ImGui::Checkbox("Backdrop", &g_Options.misc_backdrop);
	ImGui::Checkbox("Rank reveal", &g_Options.misc_rankreveal);
	//ImGui::Checkbox("Autoaccept", &g_Options.misc_autoaccept);
	ImGui::Checkbox("Radar", &g_Options.misc_radar);
	ImGui::Checkbox("Name stealer", &g_Options.namestealer);
	ImGui::Checkbox("Fake Duck", &g_Options.fakeduck);
	if (g_Options.fakeduck)
		KeyBind("Fake Duck bind", g_Options.fakeduck_bind);



	ImGui::Checkbox("Edge Jump", &g_Options.edgejump);
	if (g_Options.edgejump)
		KeyBind("##bind", g_Options.edgejump_bind);

	ImGui::Checkbox("Autoblock", &g_Options.autoblock);
	if (g_Options.autoblock)
		KeyBind("##bind", g_Options.autoblock_bind);



#if PRIVATE != 2
	ImGui::Combo("Desync", &g_Options.misc_desync, items, 3);
	if (g_Options.misc_desync)
	{	
		ImGui::Checkbox("Autodirection", &g_Options.misc_desync_ad);
		KeyBind("Manual", g_Options.misc_desync_bind);
	}


	ImGui::Checkbox("Resolver", &g_Options.resolver);
	ImGui::Checkbox("SlideWalk", &g_Options.slide);

	ImGui::Combo("Spammer", &g_Options.misc_chat_spammer, Spammers, IM_ARRAYSIZE(Spammers));
	ImGui::Checkbox("Disable AntiUntrusted", &g_Options.antiuntrusted);

/*	ImGui::Checkbox("Zoom", &g_Options.zoom);
	if (g_Options.zoom) {
		ImGui::Separator();
		KeyBind("Zoom##zoomkey", g_Options.zoomkey, ImVec2(0, 0));
		}*/
#endif

	ImGui::EndChild();
	ImGui::NextColumn();

	ImGui::BeginChild("##12");

#if PRIVATE != 2
	//ImGui::Checkbox("Auto Reload", &g_Options.autoreload);
	ImGui::Checkbox("Knifebot", &g_Options.kbot);
	ImGui::Checkbox("Optimize", &g_Options.esp_optimise);
	ImGui::Separator();
	ImGui::Checkbox("Clantag", &g_Options.misc_clantag);
	if (g_Options.misc_clantag)
	{
		ImGui::Checkbox("Custom", &g_Options.misc_customclan);
		if (g_Options.misc_customclan) {
			ImGui::Checkbox("Animated", &g_Options.misc_clantaga);
			ImGui::InputText("Custom Clantag", g_Options.customclan, sizeof(g_Options.customclan) - 1);
			if (!g_Options.misc_clantaga)
				if (ImGui::Button("Set Clantag"))
					Utils::SetClantag(g_Options.customclan);
		}
	}
	ImGui::Separator();
	//if (ImGui::Button("Queue"))
	//	do_queue();
	//ImGui::Combo("Radio Channel", &g_Options.radiochannel, radio, IM_ARRAYSIZE(radio));
	//ImGui::SliderInt("Volume", &g_Options.radiovolume, 0, 100);
	//KeyBind("Mute", g_Options.radiomute);

#endif

	ImGui::Checkbox("Fakelag", &g_Options.fakelag_enabled);
	if (g_Options.fakelag_enabled)
	{
		ImGui::Checkbox("Standing", &g_Options.fakelag_standing);
		ImGui::Checkbox("Moving", &g_Options.fakelag_moving);
		ImGui::Checkbox("Unducking", &g_Options.fakelag_unducking);
		ImGui::Combo("Mode", &g_Options.fakelag_mode, std::vector<std::string>{"Factor", "Switch", "Adaptive", "Random", "Legit peek"});
		ImGui::SliderInt("Factor", &g_Options.fakelag_factor, 1, 15);
	}
	ImGui::EndChild();
	ImGui::PopItemWidth();

	ImGui::Columns(1, nullptr, false);
}

Vector RotatePoint(Vector EntityPos, Vector LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck, bool angleInRadians)
{
	float r_1, r_2;
	float x_1, y_1;

	r_1 = -(EntityPos.y - LocalPlayerPos.y);
	r_2 = EntityPos.x - LocalPlayerPos.x;
	float Yaw = angle - 90.0f;

	float yawToRadian = Yaw * (float)(M_PI / 180.0F);
	x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
	y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

	*viewCheck = y_1 < 0;

	x_1 *= zoom;
	y_1 *= zoom;

	int sizX = sizeX / 2;
	int sizY = sizeY / 2;

	x_1 += sizX;
	y_1 += sizY;

	if (x_1 < 5)
		x_1 = 5;

	if (x_1 > sizeX - 5)
		x_1 = sizeX - 5;

	if (y_1 < 5)
		y_1 = 5;

	if (y_1 > sizeY - 5)
		y_1 = sizeY - 5;


	x_1 += posX;
	y_1 += posY;


	return Vector(x_1, y_1, 0);
}





static int weapon_index = 7;
static int weapon_vector_index = 0;
struct WeaponName_t {
	constexpr WeaponName_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};

std::vector< WeaponName_t> WeaponNames =
{
{ WEAPON_AK47, "AK-47" },
{ WEAPON_AUG, "AUG" },
{ WEAPON_AWP, "AWP" },
{ WEAPON_CZ75A, "CZ75 Auto" },
{ WEAPON_DEAGLE, "Desert Eagle" },
{ WEAPON_ELITE, "Dual Berettas" },
{ WEAPON_FAMAS, "FAMAS" },
{ WEAPON_FIVESEVEN, "Five-SeveN" },
{ WEAPON_G3SG1, "G3SG1" },
{ WEAPON_GALILAR, "Galil AR" },
{ WEAPON_GLOCK, "Glock-18" },
{ WEAPON_M249, "M249" },
{ WEAPON_M4A1_SILENCER, "M4A1-S" },
{ WEAPON_M4A1, "M4A4" },
{ WEAPON_MAC10, "MAC-10" },
{ WEAPON_MAG7, "MAG-7" },
{ WEAPON_MP7, "MP7" },
{ WEAPON_MP5, "MP5" },
{ WEAPON_MP9, "MP9" },
{ WEAPON_NEGEV, "Negev" },
{ WEAPON_NOVA, "Nova" },
{ WEAPON_HKP2000, "P2000" },
{ WEAPON_P250, "P250" },
{ WEAPON_P90, "P90" },
{ WEAPON_BIZON, "PP-Bizon" },
{ WEAPON_REVOLVER, "R8 Revolver" },
{ WEAPON_SAWEDOFF, "Sawed-Off" },
{ WEAPON_SCAR20, "SCAR-20" },
{ WEAPON_SSG08, "SSG 08" },
{ WEAPON_SG556, "SG 553" },
{ WEAPON_TEC9, "Tec-9" },
{ WEAPON_UMP45, "UMP-45" },
{ WEAPON_USP_SILENCER, "USP-S" },
{ WEAPON_XM1014, "XM1014" },
};
void RenderCurrentWeaponButton()
{
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)
		return;
	if (!g_Options.use_all_weapons) {
		if (ImGui::Button("Current")) {
			short wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
			auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a) {
				return a.definition_index == wpn_idx;
				});
			if (wpn_it != WeaponNames.end()) {
				weapon_index = wpn_idx;
				weapon_vector_index = std::abs(std::distance(WeaponNames.begin(), wpn_it));
			}
		}
	}
	else
	{
		short wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
		auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a) {
			return a.definition_index == wpn_idx;
			});
		if (wpn_it != WeaponNames.end()) {
			weapon_index = wpn_idx;
			weapon_vector_index = std::abs(std::distance(WeaponNames.begin(), wpn_it));
		}
	}

}
void RenderAimbotTab()
{

	static int   visuals_tab = 0;
	static char* visuals_tabs[] = { "AIM", "TRIGGER" };

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	render_tabs(visuals_tabs, visuals_tab, (ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().WindowPadding.x * 2.0f) / _countof(visuals_tabs), 25.0f);
	ImGui::PopStyleVar();
	if (!visuals_tab)
	{
		ImGui::BeginGroupBox("##visuals_body");
		{
			ImGui::Columns(3, NULL, true);
			auto settings = &g_Options.aimbot[weapon_index];
			ImGui::BeginChild("##aimbot.weapons", ImVec2(0, 0), false);
			{
				ImGui::Text("General");
				ImGui::Separator();
				ImGui::PushItemWidth(-1);
				if (ImGui::Combo(
					"##weapon_aimbot", &weapon_vector_index,
					[](void* data, int32_t idx, const char** out_text) {
						auto vec = reinterpret_cast<std::vector< WeaponName_t >*>(data);
						*out_text = vec->at(idx).name;
						return true;
					}, (void*)(&WeaponNames), WeaponNames.size())) {
					weapon_index = WeaponNames[weapon_vector_index].definition_index;
				}
				//if (!g_Options.use_all_weapons)
				RenderCurrentWeaponButton();
				ImGui::Checkbox("Set Automaticaly", &g_Options.use_all_weapons);
				ImGui::PopItemWidth();
				ImGui::Separator();
				ImGui::PushItemWidth(-1);
				ImGui::Checkbox("Enabled##3333", &settings->enabled);
				ImGui::Checkbox("Friendly fire", &settings->deathmatch);

				if (weapon_index == WEAPON_P250 ||
					weapon_index == WEAPON_USP_SILENCER ||
					weapon_index == WEAPON_GLOCK ||
					weapon_index == WEAPON_FIVESEVEN ||
					weapon_index == WEAPON_TEC9 ||
					weapon_index == WEAPON_DEAGLE ||
					weapon_index == WEAPON_ELITE ||
					weapon_index == WEAPON_HKP2000) {
					ImGui::Checkbox("Autopistol", &settings->autopistol);
				}

				ImGui::Checkbox("Smoke check", &settings->check_smoke);
				ImGui::Checkbox("Flash check ", &settings->check_flash);
				ImGui::Checkbox("Jump check", &settings->check_jump);
				ImGui::Checkbox("Backtrack", &settings->backtrack.enabled);
				ImGui::Checkbox("Autowall", &settings->autowall);
				ImGui::Checkbox("Curve", &settings->humanize);
				ImGui::Checkbox("Silent", &settings->silent);
				ImGui::Checkbox("Anti Aimlock", &settings->antiaimlock);
				if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08 ||
					weapon_index == WEAPON_AUG || weapon_index == WEAPON_SG556) {
					ImGui::Checkbox("Only In Zoom", &settings->only_in_zoom);
				}

				ImGui::Checkbox("On Key", &settings->on_key);

				if (settings->on_key) {
					ImGui::Text("Bind");
					ImGui::SameLine();
					KeyBind("##Key bind", settings->key);
				}

				ImGui::Checkbox("Auto Fire", &settings->autofire);
				if (settings->autofire) {
					ImGui::Text("Bind");
					ImGui::SameLine();
					KeyBind("##Auto Fire bind", settings->autofire_key);
				}

				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
			ImGui::NextColumn();

			ImGui::BeginChild("##aimbot.misc", ImVec2(0, 0), false);
			{
				ImGui::Text("Misc");
				ImGui::Separator();
				ImGui::PushItemWidth(-1);
				static char* priorities[] = {
				"Fov",
				"Health",
				"Damage",
				"Distance"
				};
				static char* aim_types[] = {
				"Hitbox",
				"Nearest"
				};
				static char* fov_types[] = {
				"Static",
				"Dynamic"
				};
				static char* smooth_types[] = {
				"Slow near target",
				"Linear"
				};
				static char* hitbox_list[] = {
				"Head",
				"Neck",
				"Pelvis",
				"Stomach",
				"Lower Chest",
				"Chest",
				"Upper Chest",
				"Left Thigh",
				"Right Thigh",
				};
				ImGui::Text("Aim Type:");
				ImGui::Combo("##aimbot.aim_type", &settings->aim_type, aim_types, IM_ARRAYSIZE(aim_types));
				if (settings->aim_type == 0) {
					ImGui::Text("Hitbox:");
					ImGui::Combo("##aimbot.hitbox", &settings->hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
				}
				ImGui::Text("Priority:");
				ImGui::Combo("##aimbot.priority", &settings->priority, priorities, IM_ARRAYSIZE(priorities));
				ImGui::Text("FOV Type:");
				ImGui::Combo("##aimbot.type", &settings->fov_type, fov_types, IM_ARRAYSIZE(fov_types));
				ImGui::Text("Smooth Type:");
				ImGui::Combo("##aimbot.smooth_type", &settings->smooth_type, smooth_types, IM_ARRAYSIZE(smooth_types));
				ImGui::SliderFloat("##aimbot.fov", &settings->fov, 0, 20, "Fov: %.2f");
				if (settings->silent) {
					ImGui::SliderFloat("##aimbot.silent_fov", &settings->silent_fov, 0, 20, "Silent Fov: %.2f");
				}
				ImGui::SliderFloat("##aimbot.smooth", &settings->smooth, 1, 15, "Smooth: %.2f");
				if (!settings->silent) {
					ImGui::SliderInt("##aimbot.shot_delay", &settings->shot_delay, 0, 100, "Shot Delay: %.0f");
				}
				ImGui::SliderInt("##aimbot.kill_delay", &settings->kill_delay, 0, 1000, "Kill Delay: %.0f");
				if (settings->humanize)
					ImGui::SliderFloat("##aimbot.curve", &settings->curviness, -10.f, 10.f, "Curve: %.0f");
				if (settings->backtrack.enabled) {
					ImGui::SliderInt("##aimbot_backtrack_ticks", &settings->backtrack.ticks, 0, 12, "BackTrack Ticks: %i");
				}
				if (settings->autowall) {
					ImGui::SliderInt("##aimbot.min_damage", &settings->min_damage, 1, 100, "Min Damage: %.0f");
				}
				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("##aimbot.rcs", ImVec2(0, 0), false);
			{
				ImGui::Text("RCS");
				ImGui::Separator();
				ImGui::PushItemWidth(-1);
				ImGui::Checkbox("Enabled##aimbot.rcs", &settings->rcs);
				ImGui::Text("RCS Type:");
				static char* rcs_types[] = {
				"Standalone",
				"Aim"
				};

				ImGui::Combo("##aimbot.rcs_type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));
				ImGui::Checkbox("RCS Custom Fov", &settings->rcs_fov_enabled);
				if (settings->rcs_fov_enabled) {
					ImGui::SliderFloat("##aimbot.rcs_fov", &settings->rcs_fov, 0, 20, "RCS Fov: %.2f");
				}
				ImGui::Checkbox("RCS Custom Smooth", &settings->rcs_smooth_enabled);
				if (settings->rcs_smooth_enabled) {
					ImGui::SliderFloat("##aimbot.rcs_smooth", &settings->rcs_smooth, 1, 15, "RCS Smooth: %.2f");
				}
				ImGui::SliderInt("##aimbot.rcs_x", &settings->rcs_x, 0, 100, "RCS X: %d");
				ImGui::SliderInt("##aimbot.rcs_y", &settings->rcs_y, 0, 100, "RCS Y: %d");
				ImGui::SliderInt("##aimbot.rcs_start", &settings->rcs_start, 1, 30, "RCS Start: %d");
				ImGui::PopItemWidth();
			}
			ImGui::EndChild();
			ImGui::Columns(1, NULL, false);
		}
		ImGui::EndGroupBox();
	}
	else
	{
		ImGui::BeginGroupBox("##trigger");
		{

			ImGui::Columns(2, NULL, true);
			ImGui::BeginChild("##1231231234");
			ImGui::Text("Main");
			ImGui::Separator();
			ImGui::Checkbox("Enabled##444", &g_Options.trigenable);
			ImGui::Checkbox("On Key", &g_Options.onkey);
			if (g_Options.onkey)
				KeyBind("Key##123", g_Options.trigbind);
			ImGui::Separator;
			//ImGui::Checkbox("AutoStop", &g_Options.trigger_autostop);
			ImGui::Checkbox("Check Smoke", &g_Options.trigchecksmoke);
			ImGui::Checkbox("Check Flash", &g_Options.trigcheckflash);
			ImGui::SliderFloat("Hitchance", &g_Options.trighitchance, 0.f, 100.f, "%.1f");
			ImGui::SliderFloat("Delay", &g_Options.trigdelay, 0.f, 150.f, "%.1f");
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("##123123");
			ImGui::Text("Hitgroups");

			ImGui::Separator();
			//ImGui::
			ImGui::Selectable("Head", &g_Options.trighead);
			ImGui::Selectable("Chest", &g_Options.trigchest);
			ImGui::Selectable("Stomach", &g_Options.trigstomach);
			ImGui::Selectable("Left Arm", &g_Options.trigleftarm);
			ImGui::Selectable("Right Arm", &g_Options.trigrightarm);
			ImGui::Selectable("Left Leg", &g_Options.trigleftleg);
			ImGui::Selectable("Right Leg", &g_Options.trigrightleg);
			ImGui::EndChild();
			ImGui::Columns(1, NULL, false);
		}
		ImGui::EndGroupBox();
	}
}

struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};
template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleA("client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}

const char* GetWeaponNameById(int id)
{
	switch (id)
	{
	case 1:
		return "deagle";
	case 2:
		return "elite";
	case 3:
		return "fiveseven";
	case 4:
		return "glock";
	case 7:
		return "ak47";
	case 8:
		return "aug";
	case 9:
		return "awp";
	case 10:
		return "famas";
	case 11:
		return "g3sg1";
	case 13:
		return "galilar";
	case 14:
		return "m249";
	case 60:
		return "m4a1_silencer";
	case 16:
		return "m4a1";
	case 17:
		return "mac10";
	case 19:
		return "p90";
	case 23:
		return "mp5sd";
	case 24:
		return "ump45";
	case 25:
		return "xm1014";
	case 26:
		return "bizon";
	case 27:
		return "mag7";
	case 28:
		return "negev";
	case 29:
		return "sawedoff";
	case 30:
		return "tec9";
	case 32:
		return "hkp2000";
	case 33:
		return "mp7";
	case 34:
		return "mp9";
	case 35:
		return "nova";
	case 36:
		return "p250";
	case 38:
		return "scar20";
	case 39:
		return "sg556";
	case 40:
		return "ssg08";
	case 61:
		return "usp_silencer";
	case 63:
		return "cz75a";
	case 64:
		return "revolver";
	case 508:
		return "knife_m9_bayonet";
	case 500:
		return "bayonet";
	case 505:
		return "knife_flip";
	case 506:
		return "knife_gut";
	case 507:
		return "knife_karambit";
	case 509:
		return "knife_tactical";
	case 512:
		return "knife_falchion";
	case 514:
		return "knife_survival_bowie";
	case 515:
		return "knife_butterfly";
	case 516:
		return "knife_push";

	case 519:
		return "knife_ursus";
	case 520:
		return "knife_gypsy_jackknife";
	case 522:
		return "knife_stiletto";
	case 523:
		return "knife_widowmaker";

	case 5027:
		return "studded_bloodhound_gloves";
	case 5028:
		return "t_gloves";
	case 5029:
		return "ct_gloves";
	case 5030:
		return "sporty_gloves";
	case 5031:
		return "slick_gloves";
	case 5032:
		return "leather_handwraps";
	case 5033:
		return "motorcycle_gloves";
	case 5034:
		return "specialist_gloves";
	case 5035:
		return "studded_hydra_gloves";

	default:
		return "";
	}
}

#include "item_definitions.hpp"
void RenderChangersTab() {
	static int   changers_tab = 0;
	static char* changers_tabs[] = { "PROFILE", "INVENTORY" }; 
	render_tabs(changers_tabs, changers_tab, (ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().WindowPadding.x * 2.0f) / _countof(changers_tabs), 25.0f);
	if (changers_tab == 0)
	{
		const char* ranklist[] =
		{
			"OFF",
			"Silver I",
			"Silver II",
			"Silver III",
			"Silver IV",
			"Silver Elite",
			"Silver Elite Master",

			"Gold Nova I",
			"Gold Nova II",
			"Gold Nova III",
			"Gold Nova Master",
			"Master Guardian I",
			"Master Guardian II",

			"Master Guardian Elite",
			"Distinguished Master Guardian",
			"Legendary Eagle",
			"Legendary Eagle Master",
			"Supreme Master First Class",
			"Global Elite"

		}; 
		const char* ranklist2[] =
		{
			"OFF",
			"Lab Rat I",
			"Lab Rat II",

			"Sprinting Hare I",
			"Sprinting Hare II",

			"Wild Scout I",
			"Wild Scout II",
			"Wild Scout Elite",

			"Hunter Fox I",
			"Hunter Fox II",
			"Hunter Fox III",
			"Hunter Fox Elite",

			"Timber Wolf",
			"Ember Wolf",
			"Wildfire Wolf",
			"The Howling Alpha"

		};

		ImGui::Checkbox("Enabled", &g_Options.profile.enabled);
		ImGui::Checkbox("Prime", &g_Options.profile_prime);

		ImGui::Combo("Rank MM", &g_Options.profile.rank_mm, ranklist, IM_ARRAYSIZE(ranklist));
		ImGui::InputInt("Wins", &g_Options.profile.wins_mm);
		ImGui::Combo("Rank WM", &g_Options.profile.rank_wm, ranklist, IM_ARRAYSIZE(ranklist));
		ImGui::InputInt("Wins", &g_Options.profile.wins_wm);
		ImGui::Combo("Rank Danger", &g_Options.profile.rank_danger, ranklist2, IM_ARRAYSIZE(ranklist2));
		ImGui::InputInt("Wins", &g_Options.profile.wins_danger);
		ImGui::Separator();
		ImGui::SliderInt("Level", &g_Options.profile.player_level, -1, 40);
		ImGui::SliderInt("XP", &g_Options.profile.player_xp, -1, 5000);
		ImGui::Separator();
		ImGui::InputInt("Teacher", &g_Options.profile.teaching);
		ImGui::InputInt("Friendly", &g_Options.profile.friendly);
		ImGui::InputInt("Leader", &g_Options.profile.leader);
		ImGui::Separator();
		if (ImGui::Button("Apply"))
		{
			Protobuf::SendClientHello();
			Protobuf::SendMatchmakingClient2GCHello();
		}
	}
	else if (changers_tab == 1) 
	{
		static wskin weaponSkin;

		if (ImGui::BeginCombo("Weapon", k_inventory_names.at(weaponSkin.wId)))
		{
			for (const auto& weapon : k_inventory_names)
			{
				if (ImGui::Selectable(weapon.second, weaponSkin.wId == weapon.first))
				{
					weaponSkin.wId = weapon.first;
					weaponSkin.paintKit = 0;
					skinImage = nullptr;
				}
			}
			ImGui::EndCombo();
		}

		auto weaponName = weaponnames(weaponSkin.wId);

		if (ImGui::BeginCombo("Paint Kit", weaponSkin.paintKit > 0 ? g_Options.inventory.skinInfo[weaponSkin.paintKit].name.c_str() : ""))
		{
			int lastID = ImGui::GetItemID();

			for (auto skin : g_Options.inventory.skinInfo)
			{
				for (auto names : skin.second.weaponName)
				{
					if (weaponName != names)
						continue;

					ImGui::PushID(lastID++);

					if (ImGui::Selectable(skin.second.name.c_str(), skin.first == weaponSkin.paintKit))
						weaponSkin.paintKit = skin.first;

					ImGui::PopID();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::SliderFloat("Wear##new", &weaponSkin.wear, 0.f, 1.f, "%.5f");
		ImGui::InputInt("Seed##new", &weaponSkin.seed);

		if (ImGui::Button("Add##new"))
		{
			g_InventorySkins.insert({ RandomInt(20000, 200000), weaponSkin });
			g_Options.inventory.itemCount = g_InventorySkins.size();
		}
		ImGui::SameLine();
		if (ImGui::Button("Apply"))
		{
			Protobuf::SendClientHello();
			Protobuf::SendMatchmakingClient2GCHello();
		}

		ImGui::NextColumn();

		static int selectedId = 0;

		if (ImGui::ListBoxHeader("##skins", ImVec2(-1, 0)))
		{
			int lastID = ImGui::GetItemID();
			for (auto weapon : g_InventorySkins)
			{
				if (!weapon.second.wId || !weapon.second.paintKit)
					continue;

				ImGui::PushID(lastID++);

				if (ImGui::Selectable((k_inventory_names.at(weapon.second.wId) + std::string(" | ") + g_Options.inventory.skinInfo[weapon.second.paintKit].name).c_str(), selectedId == weapon.first))
					selectedId = weapon.first;
				ImGui::PopID();
			}

			ImGui::ListBoxFooter();
		}

		if (selectedId != 0)
		{
			ImGui::SliderFloat("Wear##existing", &g_InventorySkins[selectedId].wear, 0.f, 1.f, "%.5f");
			ImGui::InputInt("Seed##existing", &g_InventorySkins[selectedId].seed);

			if (ImGui::Button("Delete##existing", ImVec2(-1, 25)))
			{
				g_InventorySkins.erase(selectedId);
				g_Options.inventory.itemCount = g_InventorySkins.size();
			}
		}
	}
}

void RenderConfigTab()
{
	static std::vector<std::string> configs;

	static auto load_configs = []() {
		std::vector<std::string> items = {};

		std::string path("C:\\Muten");
		if (!fs::is_directory(path))
			fs::create_directories(path);

		for (auto& p : fs::directory_iterator(path))
			items.push_back(p.path().string().substr(path.length() + 1));

		return items;
	};

	static auto is_configs_loaded = false;
	if (!is_configs_loaded) {
		is_configs_loaded = true;
		configs = load_configs();
	}

	static std::string current_config;

	static char config_name[32];

	ImGui::InputText("##config_name", config_name, sizeof(config_name));
	ImGui::SameLine();
	if (ImGui::Button("Create")) {
		current_config = std::string(config_name);

		Config->Save(current_config + ".ini");
		is_configs_loaded = false;
		memset(config_name, 0, 32);
	}



	ImGui::ListBoxHeader("##configs");
	{
		for (auto& config : configs) {
			if (ImGui::Selectable(config.c_str(), config == current_config)) {
				current_config = config;
			}
		}
	}
	ImGui::ListBoxFooter();


	if (!current_config.empty()) {

		if (ImGui::Button("Load"))
			Config->Load(current_config);
		ImGui::SameLine();

		if (ImGui::Button("Save"))
			Config->Save(current_config);
		ImGui::SameLine();

		if (ImGui::Button("Delete") && fs::remove("C:\\Muten\\" + current_config)) {
			current_config.clear();
			is_configs_loaded = false;
		}

		ImGui::SameLine();
	}

	if (ImGui::Button("Refresh"))
		is_configs_loaded = false;
}

static int currentPlayer = -1;
bool changeName(bool reconnect, const char* newName, float delay) noexcept;
std::string Rank(int* id) {
	std::string rank;
	switch (*id) {
	case 1:
		rank = "Silver 1";
		break;
	case 2:
		rank = "Silver 2";
		break;
	case 3:
		rank = "Silver 3";
		break;
	case 4:
		rank = "Silver 4";
		break;
	case 5:
		rank = "Silver 5";
		break;
	case 6:
		rank = "Silver 5";
		break;
	case 7:
		rank = "Gold Nova 1";
		break;
	case 8:
		rank = "Gold Nova 2";
		break;
	case 9:
		rank = "Gold Nova 3";
		break;
	case 10:
		rank = "Gold Nova 4";
		break;
	case 11:
		rank = "MG 1";
		break;
	case 12:
		rank = "MG 2";
		break;
	case 13:
		rank = "MG Elite";
		break;
	case 14:
		rank = "Bigstar";
		break;
	case 15:
		rank = "LE";
		break;
	case 16:
		rank = "LEM";
		break;
	case 17:
		rank = "Supreme";
		break;
	case 18:
		rank = "Global Elite";
		break;
	default:
		rank = "None";
	}
	return rank;
}
void PlayerList() {
	if (!g_EngineClient->IsInGame() || (*g_PlayerResource && !(*g_PlayerResource)->GetConnected(currentPlayer)))
		currentPlayer = -1;
	ImGui::Text("Players List");

	
	if (*g_PlayerResource)
	{
		ImGui::ListBoxHeader("##PLAYERS", ImVec2(-1, (ImGui::GetWindowSize().y - 95)));
		ImGui::Columns(6);

		ImGui::Text("Nickname");
		ImGui::NextColumn();

		ImGui::Text("Team");
		ImGui::NextColumn();

		ImGui::Text("Money");
		ImGui::NextColumn();

		ImGui::Text("Clantag");
		ImGui::NextColumn();

		ImGui::Text("Rank");
		ImGui::NextColumn();

		ImGui::Text("Wins");
		ImGui::NextColumn();

		std::unordered_map<int, std::vector<int>> players = {
			{ TEAM_UNASSIGNED,{} },
			{ TEAM_SPECTATOR,{} },
			{ TEAM_TERRORIST,{} },
			{ TEAM_COUNTER_TERRORIST,{} },
		};

		for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
		{
			auto ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
			if (!ent)
				continue;

			if (ent == g_LocalPlayer)
				continue;

			players[ent->m_iTeamNum()].push_back(i);
		}

		for (int team = TEAM_UNASSIGNED; team <= TEAM_COUNTER_TERRORIST; team++)
		{
			char* teamName = strdup("");
			switch (team)
			{
			case TEAM_SPECTATOR:
				teamName = strdup("SPEC");
				break;
			case TEAM_TERRORIST:
				teamName = strdup("T");
				break;
			case TEAM_COUNTER_TERRORIST:
				teamName = strdup("CT");
				break;
			}

			for (auto it : players[team])
			{
				std::string id = "";
				auto player = C_BasePlayer::GetPlayerByIndex(it);
				if (player)
				{
					if (player->IsAlive())
						id = "Alive";
					else 
						id = "Dead";
				}
				player_info_t entityInformation;
				g_EngineClient->GetPlayerInfo(it, &entityInformation);

				//if (entityInformation.ishltv)
				//	continue;

				ImGui::Separator();

				if (ImGui::Selectable(entityInformation.szName, it == currentPlayer, ImGuiSelectableFlags_SpanAllColumns))
					currentPlayer = it;
				ImGui::NextColumn();

				ImGui::Text("%s", teamName);
				ImGui::NextColumn();
				
				if (player)
					ImGui::Text("%d$", player->m_iAccount());
				ImGui::NextColumn();

				ImGui::Text("%s", (*g_PlayerResource)->GetClan(it));
				ImGui::NextColumn();

				ImGui::Text("%d",*(*g_PlayerResource)->GetCompetitiveRanking(it));
				ImGui::NextColumn();

				ImGui::Text("%d", *(*g_PlayerResource)->GetCompetitiveWins(it));
				ImGui::NextColumn();
			}
		}

		ImGui::ListBoxFooter();

		if (currentPlayer != -1)
		{
			player_info_t entityInformation;
			g_EngineClient->GetPlayerInfo(currentPlayer, &entityInformation);

			ImGui::Columns(2);
			{
			
				/*bool isFriendly = std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid) != Aimbot::friends.end();
				if (ImGui::Checkbox("Friend", &isFriendly))
				{
					if (isFriendly)
						Aimbot::friends.push_back(entityInformation.xuid);
					else
						Aimbot::friends.erase(std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid));
				}

				bool shouldResolve = std::find(Resolver::Players.begin(), Resolver::Players.end(), entityInformation.xuid) != Resolver::Players.end();
				if (ImGui::Checkbox("Resolver", &shouldResolve))
				{
					if (shouldResolve)
						Resolver::Players.push_back(entityInformation.xuid);
					else
						Resolver::Players.erase(std::find(Resolver::Players.begin(), Resolver::Players.end(), entityInformation.xuid));
				}*/
			}
			ImGui::NextColumn();
			{

				if (ImGui::Button("Steal name"))
				{
					std::string name;
					const char* ass = entityInformation.szName;
					name = ass;

					changeName(false, name.append("\x1").c_str(), 1.f);
					
				}
				ImGui::SameLine();
				if (ImGui::Button("Kick"))
				{
					Kick(entityInformation.userId);
				}
				ImGui::SameLine();
				const char* clanTag = (*g_PlayerResource)->GetClan(currentPlayer);
				if (strlen(clanTag) > 0 && ImGui::Button("Steal clan tag"))
				{
					Utils::SetClantag(clanTag);
				}

			}
		}
	}
}

void Menu::RenderPreview()
{
	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	float x = w * 0.5f - 300.0f;
	float y = h * 0.5f - 200.0f;
	ImGui::SetNextWindowPos(ImVec2{ x, y }, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2{ 250, 400 }, ImGuiCond_Once);
	if (ImGui::Begin("##ep", &_visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
	{
		ImDrawList* draw = ImGui::GetWindowDrawList();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::BeginGroupBox("Esp Preview");
		{
			int alpha = 0;
			static const char* name = "";
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::SameLine();
			if (g_Options.esp_player_names)
				name = "name";
			ImGui::Text(name);
			ImGui::Spacing();
			if (g_Options.esp_player_health)
				alpha ? 255 : 0;
			
			ImGui::SameLine();
			if (g_Options.esp_player_box) {
				ImVec2 pos1 = ImGui::GetCursorScreenPos();
				draw->AddRect(ImVec2(pos1.x, pos1.y), ImVec2(pos1.x + 100, pos1.y + 175), ImColor(255, 255, 255, 255), 0.0f, 15, 1.f);
			}
			if (g_Options.esp_player_weapons) {
				ImGui::SameLine();
				ImGui::Text("weapon");
			}
			if (g_Options.esp_player_armour) {
				ImVec2 pos2 = ImGui::GetCursorScreenPos();
				draw->AddRect(ImVec2(pos2.x, pos2.y), ImVec2(pos2.x - 2, pos2.y + 175), ImColor(0, 0, 255, 255), 0.0f, 15.f, 1.f);
			}
		}
		ImGui::EndGroupBox();
		ImGui::PopStyleColor();
	}
	ImGui::End();
}
void Menu::Initialize()
{
	CreateStyle();

	_visible = false;
}

void Menu::Shutdown()
{
	_visible = false;
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
	ImGui_ImplDX9_CreateDeviceObjects();
}
#include "BASS/bass.h"
#include "sounds/sounds.h"
void RenderRadio() {
	if (strlen(BASS::bass_metadata) > 0 && g_Options.radiochannel) {

		ImGui::Begin("Now playing", nullptr, ImVec2(0, 0), 0.4F, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text(radio_muted ? "MUTED" : BASS::bass_metadata);
		ImGui::End();
	}
}

ImVec2 pos;
ImDrawList* draw;
void decorations()
{


	pos = ImGui::GetWindowPos();
	draw = ImGui::GetWindowDrawList();

	

	draw->AddRectFilled(ImVec2(pos), ImVec2(pos.x + 765, pos.y + 495), ImColor(29, 29, 29));
	draw->AddRectFilled(ImVec2(pos.x, pos.y + 47), ImVec2(pos.x + 765, pos.y + 93), ImColor(45, 45, 45));
	draw->AddRectFilled(ImVec2(pos), ImVec2(pos.x + 765, pos.y + 44), ImColor(253, 166, 8));

	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image(loggo, ImVec2(49, 50));
}

extern ImDrawList* draw;

bool ImGui::tab(const char* label, bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	static int y = 46;
	static float x = 102.3f;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(ImVec2(x, y), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

	if (hovered || held)
		ImGui::SetMouseCursor(7);

	if (selected)
	{
		draw->AddRectFilled(ImVec2(bb.Min), ImVec2(bb.Max), ImColor(35, 35, 35));
		draw->AddRectFilledMultiColor(ImVec2(bb.Min), ImVec2(bb.Max.x, bb.Min.y + 20), ImColor(29, 29, 29), ImColor(29, 29, 29), ImColor(35, 35, 35), ImColor(35, 35, 35));
	}

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f));
	ImGui::RenderText(ImVec2(bb.Min.x + 0 + (size.x / 2 - label_size.x / 2), bb.Min.y + (size.y / 2 - label_size.y / 2)), label);
	ImGui::PopStyleColor();

	return pressed;
}
static int selected_tab = 0;
ImFont* icons = nullptr;


float dpi_scale = 1.f;

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;

	if (g_Options.esp_speclist)
		Visuals::Get().RenderSpectatorList();
	if (g_Options.esp_preview)
		Menu::RenderPreview();

	//RenderRadio();

	if (!_visible)
		return;

	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	float x = w * 0.5f - 300.0f;
	float y = h * 0.5f - 200.0f;

	ImGui::SetNextWindowPos(ImVec2{ x, y }, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2{ 765, 495 }, ImGuiCond_Once);


	if (ImGui::Begin("Muten | YouGame.Biz", &_visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration)) {



		static int x = 765, y = 495;
		pos = ImGui::GetWindowPos();
		draw = ImGui::GetWindowDrawList();

		ImGui::SetWindowSize(ImVec2(ImFloor(x * dpi_scale), ImFloor(y * dpi_scale)));

		decorations();

		ImGui::PushFont(icons);

		ImGui::SetCursorPos(ImVec2(0, 47));
		ImGui::BeginGroup();

		if (ImGui::tab("A", 0 == selected_tab))
			selected_tab = 0;
		ImGui::SameLine();

		if (ImGui::tab("B", 1 == selected_tab))
			selected_tab = 1;
		ImGui::SameLine();

		if (ImGui::tab("C", 2 == selected_tab))
			selected_tab = 2;
		ImGui::SameLine();

		if (ImGui::tab("D", 3 == selected_tab))
			selected_tab = 3;
		ImGui::SameLine();

		if (ImGui::tab("F", 4 == selected_tab))
			selected_tab = 4;
		ImGui::SameLine();

		if (ImGui::tab("G", 5 == selected_tab))
			selected_tab = 5;
		ImGui::SameLine();

		if (ImGui::tab("H", 6 == selected_tab))
			selected_tab = 6;

		ImGui::EndGroup();

		ImGui::PopFont();

		/*ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushFont(g_pDefaultFont);
		render_tabs(tabs, selected_tab, (ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().WindowPadding.x * 2.0f) / _countof(tabs), 50.0f);
		ImGui::PopFont();
		ImGui::PopStyleVar();*/

		ImGui::BeginGroupBox("##body");
		{
			if (selected_tab == 0)	RenderAimbotTab();
			else if (selected_tab == 1)	RenderEspTab();
			else if (selected_tab == 2)	RenderMiscTab();
			else if (selected_tab == 3)	RenderChangersTab();
			else if (selected_tab == 4)	RenderConfigTab();
			else if (selected_tab == 5)	PlayerList();

		}
		ImGui::EndGroupBox();

		ImGui::End();
	}
}

void Visuals::RenderSpectatorList() {
	int specs = 0;
	std::string spect = "";

	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
		int localIndex = g_EngineClient->GetLocalPlayer();
		C_BasePlayer* pLocalEntity = C_BasePlayer::GetPlayerByIndex(localIndex);
		if (pLocalEntity) {
			for (int i = 1; i < g_EngineClient->GetMaxClients(); i++) {
				C_BasePlayer* pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
				if (!pBaseEntity)										     continue;
				if (pBaseEntity->m_iHealth() > 0)							 continue;
				if (pBaseEntity == pLocalEntity)							 continue;
				if (pBaseEntity->IsDormant())								 continue;
				if (!pLocalEntity->IsAlive()) {
					if (pBaseEntity->m_hObserverTarget() != pLocalEntity->m_hObserverTarget())
						continue;
				}
				else {
					if (pBaseEntity->m_hObserverTarget() != pLocalEntity)
						continue;
				}

				player_info_t pInfo;
				g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
				if (pInfo.ishltv) continue;

				spect += pInfo.szName;
				spect += "\n";
				specs++;
			}
		}
	}
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
	auto& styles = ImGui::GetStyle();
	styles.WindowRounding = 0.f;
	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
		if (ImGui::Begin("Spectator List", nullptr, ImVec2(0, 0), 0.4F, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
			if (specs > 0) spect += "\n";

			ImVec2 size = ImGui::CalcTextSize(spect.c_str());
			ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
			ImGui::Text(spect.c_str());
		}
	}
	styles.WindowRounding = 5.f;
	ImGui::End();
	ImGui::PopStyleVar();
}

void Menu::Toggle()
{
	_visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig font_config;
	font_config.OversampleH = 1; //or 2 is the same
	font_config.OversampleV = 1;
	font_config.PixelSnapH = 1;
	icons = io.Fonts->AddFontFromMemoryTTF((void*)icon, sizeof(icon), 25.f, &font_config);
	ImGuiStyle * style = &ImGui::GetStyle();


	ImGui::StyleColorsDark();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	ImGui::GetStyle() = _style;
}

