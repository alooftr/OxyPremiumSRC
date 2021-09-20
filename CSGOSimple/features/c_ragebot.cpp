#include "c_ragebot.h"
#include "../options.hpp"
//#include "rifktrace.h"
static constexpr auto total_seeds = 255;
#include "rifktrace.h"
#include "autowall.hpp"

std::optional<c_ragebot::aim_info> c_ragebot::scan_record(C_BasePlayer* local, c_animation_system::animation* animation)
{
	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return std::nullopt;

	const auto info = weapon->GetCSWeaponData();

	if (!info)
		return std::nullopt;

	const auto is_zeus = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER;
	const auto is_knife = !is_zeus && info->iWeaponType == WEAPONTYPE_KNIFE;


	//return scan_record_aimbot(local, animation);
	if(!is_knife)
		return scan_record_gun(local, animation);
}

inline static const int hitboxes_aiming[] = {

	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
};

inline static const int hitboxes_baim[] = {
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
};

std::optional<Options> get_weapon_conf()
{
	const auto local = g_LocalPlayer;

	if (!local)
		return std::nullopt;

	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return std::nullopt;

	const auto info = weapon->GetCSWeaponData();

	if (!info)
		return std::nullopt;

	return g_Options;
	/*	
	if (info->get_weapon_id() == weapon_g3sg1 || info->get_weapon_id() == weapon_scar20)
		return config.rage.auto_snipe;

	if (info->get_weapon_id() == weapon_ssg08)
		return config.rage.scout;

	if (info->get_weapon_id() == weapon_awp)
		return config.rage.awp;

	if (info->get_weapon_id() == weapon_deagle || info->get_weapon_id() == weapon_revolver)
		return config.rage.heavy;

	if (info->get_weapon_id() == weapon_taser)
		return config.rage.pistol;

	if (info->WeaponType == weapontype_pistol)
		return config.rage.pistol;

	return config.rage.misc;*/
}

bool can_hit_hitbox(const Vector start, const Vector end, matrix3x4_t* bones, studiohdr_t* hdr, int box)
{
	const auto studio_box = hdr->get_hitbox(static_cast<uint32_t>(box), 0);

	if (!studio_box)
		return false;

	Vector min, max;

	const auto is_capsule = studio_box->m_flRadius != -1.f;

	if (is_capsule)
	{
		Math::VectorTransform(studio_box->bbmin, bones[studio_box->bone], min);
		Math::VectorTransform(studio_box->bbmax, bones[studio_box->bone], max);
		const auto dist = Math::segment_to_segment(start, end, min, max);

		if (dist < studio_box->m_flRadius)
			return true;
	}

	if (!is_capsule)
	{
		Math::VectorTransform(Math::vector_rotate(studio_box->bbmin, studio_box->rotation), bones[studio_box->bone], min);
		Math::VectorTransform(Math::vector_rotate(studio_box->bbmax, studio_box->rotation), bones[studio_box->bone], max);

		Math::vector_i_transform(start, bones[studio_box->bone], min);
		Math::vector_i_rotate(end, bones[studio_box->bone], max);

		if (Math::intersect_line_with_bb(min, max, studio_box->bbmin, studio_box->bbmax))
			return true;
	}

	return false;
}
static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};

void build_seed_table()
{
	if (!precomputed_seeds.empty())
		return;

	for (auto i = 0; i < total_seeds; i++) {
		RandomSeed(i + 1);

		const auto pi_seed = Math::RandomFloat(0.f, (M_PI * 2));

		precomputed_seeds.emplace_back(Math::RandomFloat(0.f, 1.f),
			sin(pi_seed), cos(pi_seed));
	}
}


bool can_hit(c_animation_system::animation* animation, const Vector position, const float chance, int box)
{
	


	auto local = g_LocalPlayer;
	// generate look-up-table to enhance performance.
	build_seed_table();

	const auto weapon = local->m_hActiveWeapon();

	if (!weapon)
		return false;

	const auto info = weapon->GetCSWeaponData();

	if (!info)
		return false;

	const auto studio_model = g_MdlInfo->GetStudiomodel(animation->player->GetModel());

	if (!studio_model)
		return false;

	// performance optimization.
	if ((local->GetEyePos() - position).Length() > info->flRange)
		return false;

	

	// setup calculation parameters.
	const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };

	const auto sniper = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20
		|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_G3SG1 || weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08;
	const auto crouched = local->m_fFlags() & IN_DUCK;

	weapon->UpdateAccuracyPenalty();

	// calculate inaccuracy.
	const auto weapon_inaccuracy = weapon->GetInaccuracy();
	const auto weapon_spread = weapon->GetSpread();


	//if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
	//	return weapon_inaccuracy < (crouched ? .0020f : .0055f);

	// no need for hitchance, if we can't increase it anyway.
	/*if (crouched)
	{
		if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->flInaccuracyCrouchAlt : info->flInaccuracyCrouch))
			return true;
	}
	else
	{
		if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->flInaccuracyStandAlt : info->flInaccuracyStand))
			return true;
	}*/

	// calculate start and angle.
	const auto start = local->GetEyePos();
	const auto aim_angle = Math::CalcAngle(start, position);
	Vector forward, right, up;
	Math::AngleVectors(aim_angle, forward, right, up);

	// keep track of all traces that hit the enemy.
	auto current = 0;

	// setup calculation parameters.
	Vector total_spread, end;
	QAngle spread_angle;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	const auto get_bullet_location = [&](int seed) {
		RandomSeed(seed + 1);
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * M_PI);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * M_PI);
		if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			a = 1.f - a * a;
			c = 1.f - c * c;
		}
		const float generated_spread = a * weapon_spread;
		const float generated_cone = c * weapon_inaccuracy;

		const Vector spread = Vector(
			std::cos(b) * generated_spread + std::cos(d) * generated_cone,
			std::sin(b) * generated_spread + std::sin(d) * generated_cone,
			0.f
		);

		return Vector(forward + right * -spread.x + up * -spread.y).Normalized();
	};

	// use look-up-table to find average hit probability.
	for (auto i = 0u; i < total_seeds; i++)  // NOLINT(modernize-loop-convert)
	{
		// get seed.
		seed = &precomputed_seeds[i];

		// calculate spread.
		inaccuracy = std::get<0>(*seed) * weapon_inaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).Normalized();



		// calculate angle with spread applied.
		Math::VectorAngles(total_spread, spread_angle);

		// calculate end point of trace.
		Math::AngleVectors(spread_angle, end);
		end = start + end.Normalized() * info->flRange;

		// did we hit the hitbox?
		if (can_hit_hitbox(start, end, animation->bones, studio_model, box))
			current++;

		// abort if hitchance is already sufficent.
		if (static_cast<float>(current) / static_cast<float>(total_seeds) >= chance)
			return true;

		// abort if we can no longer reach hitchance.
		if (static_cast<float>(current + total_seeds - i) / static_cast<float>(total_seeds) < chance)
			return false;
	}

	return static_cast<float>(current) / static_cast<float>(total_seeds) >= chance;
}

std::vector<c_ragebot::aim_info> c_ragebot::select_multipoint(c_animation_system::animation* animation, int box, int32_t group, float scaled_head, float scaled_body)
{
	std::vector<aim_info> points;

	if (box == HITBOX_NECK)
		return points;

	float scale = scaled_body;
	if (box == HITBOX_HEAD || box == HITBOX_NECK)
		scale = scaled_head;

	const auto model = animation->player->GetModel();

	if (!model)
		return points;

	const auto studio_model = g_MdlInfo->GetStudiomodel(model);

	if (!studio_model)
		return points;

	auto local = g_LocalPlayer;

	if (!local)
		return points;

	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return points;

	const auto anim = animation_system.get_animation_info(animation->player);

	if (!anim)
		return points;

	const auto hitbox = studio_model->get_hitbox(static_cast<uint32_t>(box), 0);

	if (!hitbox)
		return points;

	const auto is_zeus = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER;

	if (is_zeus)
		return points;

	auto& mat = animation->bones[hitbox->bone];

	Vector min, max;
	Math::VectorTransform(hitbox->bbmax, mat, max);
	Math::VectorTransform(hitbox->bbmin, mat, min);

	const auto center = (min + max) * 0.5f;
	const auto cur_angles = Math::CalcAngle(center, local->GetEyePos());

	Vector forward;
	Math::AngleVectors(cur_angles, forward);

	auto rs = hitbox->m_flRadius * scale;

	rs *= 1.f - std::min(anim->missed_due_to_spread, 4) * .05f;

	if (rs < .2f)
		return points;

	const auto right = forward.Cross(Vector(0.f, 0.f, 1.f)) * rs;
	const auto left = Vector(-right.x, -right.y, right.z);
	const auto top = Vector(0.f, 0.f, 1.f) * rs;

	const auto delta = (max - min).Normalized();
	QAngle angle;
	Math::VectorAngles(delta, angle);
	angle -= cur_angles;
	Math::Normalize3(angle);

	const auto is_horizontal = angle.pitch < 45.f && angle.pitch > -45.f;
	const auto is_flipped = angle.yaw < 0.f;

	if (box == HITBOX_HEAD || (box != HITBOX_LEFT_FOOT && box != HITBOX_RIGHT_FOOT))
	{
		points.emplace_back(max + top, 0.f, animation, false, center, hitbox->m_flRadius, rs, box, group);
		points.emplace_back(min - top, 0.f, animation, false, center, hitbox->m_flRadius, rs, box, group);
	}

	points.emplace_back(max - (is_horizontal ? Vector() - top : left), 0.f, animation, false, center, hitbox->m_flRadius, rs, box, group);
	points.emplace_back(max - (is_horizontal ? is_flipped ? left : right : right), 0.f, animation, false, center, hitbox->m_flRadius, rs, box, group);

	if (box != HITBOX_LEFT_FOOT && box != HITBOX_RIGHT_FOOT)
	{
		points.emplace_back(min - (is_horizontal ? top : left),
			0.f, animation, false, center, hitbox->m_flRadius, rs, box, group);
		points.emplace_back(min + (is_horizontal ? is_flipped ? left : right : left),
			0.f, animation, false, center, hitbox->m_flRadius, rs, box, group);
	}

	return points;
}


std::optional<c_ragebot::aim_info> c_ragebot::scan_record_gun(C_BasePlayer* local, c_animation_system::animation* animation, std::optional<Vector> pos)
{
	const auto weapon_cfg = get_weapon_conf();

	if (!animation || !animation->player || !weapon_cfg.has_value())
		return std::nullopt;

	const auto info = animation_system.get_animation_info(animation->player);

	if (!info)
		return std::nullopt;

	const auto cfg = weapon_cfg.value();

	auto should_baim = false;
	auto center = animation->player->get_hitbox_position(HITBOX_PELVIS, animation->bones);
	//auto center = animation->player->GetHitboxPos(HITBOX_PELVIS);
	if (center.has_value())
	{
		const auto center_wall = g_AutoWall.CanHitDetail(center.value());

		if (center_wall.has_value() && center_wall.value().hitbox == HITBOX_PELVIS
			&& center_wall.value().damage - 3.f > animation->player->m_iHealth())
			should_baim = true;
	}

	aim_info best_match = { Vector(), -FLT_MAX, nullptr, false, Vector(), 0.f, 0.f, HITBOX_HEAD, 0 };

	int health = animation->player->m_iHealth();

	const auto scan_box = [&](int hitbox)
	{
		auto box = animation->player->get_hitbox_position(hitbox, const_cast<matrix3x4_t*>(animation->bones));
		//auto box = animation->player->GetHitboxPos(hitbox);
		if (!box.has_value())
			return;

		auto points = pos.has_value() ?
			std::vector<aim_info>() :
			select_multipoint(animation, hitbox, HITGROUP_HEAD, g_Options.rage_mpsizehead / 100.f, g_Options.rage_mpsize / 100.f);
		points.emplace_back(box.value(), 0.f, animation, false, box.value(), 0.f, 0.f, hitbox, HITGROUP_HEAD);
		const auto low_hitchance = pos.has_value() || !can_hit(animation, box.value(), g_Options.rage_hitchance / 100.f, hitbox);

		for (auto& point : points)
		{
			if (point.rs > 0.f && low_hitchance)
				continue;

			const auto wall = g_AutoWall.CanHitDetail(point.position);

			if (!wall.has_value())
				continue;

			if (hitbox == HITBOX_HEAD && hitbox != wall.value().hitbox)
				continue;

			point.hitgroup = wall.value().hitgroup;

			point.damage = wall.value().damage;
			if ((point.damage >= g_Options.rage_mindmg || point.damage >= health) && point.damage > best_match.damage)
				best_match = point;
		}
	};

	if (should_baim)
		for (const auto& hitbox : hitboxes_baim)
			scan_box(hitbox);
	else
		for (const auto& hitbox : hitboxes_aiming)
			scan_box(hitbox);

	if (best_match.damage >= g_Options.rage_mindmg || best_match.damage >= animation->player->m_iHealth())
		return best_match;

	return std::nullopt;
}
#include "../rifkpredict.h"


bool c_ragebot::is_breaking_lagcomp(c_animation_system::animation* animation)
{
	static constexpr auto teleport_dist = 64 * 64;

	const auto info = animation_system.get_animation_info(animation->player);

	if (!info || info->frames.size() < 2)
		return false;

	if (info->frames[0].dormant)
		return false;

	auto prev_org = info->frames[0].origin;
	auto skip_first = true;

	// walk context looking for any invalidating event
	for (auto& record : info->frames)
	{
		if (skip_first)
		{
			skip_first = false;
			continue;
		}

		if (record.dormant)
			break;

		auto delta = record.origin - prev_org;
		if (delta.Length2DSqr() > teleport_dist)
		{
			// lost track, too much difference
			return true;
		}

		// did we find a context smaller than target time?
		if (record.sim_time <= animation->sim_time)
			break; // hurra, stop

		prev_org = record.origin;
	}

	return false;
}

#include "c_resolver.h"


void c_ragebot::optimize_multipoint(c_ragebot::aim_info& info)
{
	static constexpr auto steps = 6;

	auto local = g_LocalPlayer;

	if (!local || info.rs == 0.f || info.radius == 0.f)
		return;

	auto original_position = info.position;
	auto optimal_position = original_position;
	auto optimal_damage = info.damage;

	for (auto i = 0; i <= steps; i++)
	{
		const auto scale = info.rs / info.radius;

		if (scale <= .2f)
			break;

		const auto target_scale = (scale - .2f) / steps * (steps - i);
		const auto direction = (info.position - info.center) / info.rs;

		original_position = info.center + direction * info.radius * target_scale;

		const auto wall = g_AutoWall.CanHitDetail(original_position);

		if (wall.has_value())
		{
			if (info.damage > info.animation->player->m_iHealth()
				&& wall.value().damage < info.animation->player->m_iHealth())
				continue;

			if (wall.value().damage < info.damage)
				continue;

			optimal_position = original_position;
			optimal_damage = wall.value().damage;
		}
	}

	info.position = optimal_position;
	info.damage = optimal_damage;
}

float get_standing_accuracy(C_BaseCombatWeapon* weapon) 
{
	const auto max_speed = weapon->get_zoom_level() > 0 ? weapon->GetCSWeaponData()->flMaxPlayerSpeedAlt : weapon->GetCSWeaponData()->flMaxPlayerSpeed;
	g_Options.distb = max_speed;
	return max_speed / 3.3333333333333333f;
}

c_ragebot::autostop_info& c_ragebot::get_autostop_info()
{
	static c_ragebot::autostop_info stop{ -FLT_MAX, false };

	return stop;
}

void c_ragebot::autostop(C_BasePlayer* local, CUserCmd* cmd)
{
	if (cmd->buttons & IN_JUMP)
		return;

	static const auto nospread = g_CVar->FindVar(("weapon_accuracy_nospread"));

	auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (nospread->GetInt() || !(local->m_fFlags() & FL_ONGROUND) ||
		(weapon && weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER) && (local->m_fFlags() & FL_ONGROUND))
		return;

	const auto wpn_info = weapon->GetCSWeaponData();

	if (!wpn_info)
		return;

	auto& info = get_autostop_info();

	if (info.call_time == g_GlobalVars->curtime)
	{
		info.did_stop = true;
		return;
	}

	info.did_stop = false;
	info.call_time = g_GlobalVars->curtime;

	//g_Options.dista = prediction_system.unpredicted_velocity.Length2D();

	if (local->m_vecVelocity().Length2D() <= get_standing_accuracy(weapon))
		return;
	else
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;

	//	prediction_system.repredict(local, cmd);

		if (GetAsyncKeyState(VK_SHIFT))
		{
			info.did_stop = true;
			return;
		}

		if (local->m_vecVelocity().Length2D() <= get_standing_accuracy(weapon))
			return;
	}

	QAngle dir;
	Math::VectorAngles(g_LocalPlayer->m_vecVelocity(), dir);
	QAngle angles;
	g_EngineClient->GetViewAngles(&angles);
	dir.yaw = angles.yaw - dir.yaw;

	Vector move;
	Math::AngleVectors(dir, move);

	if (g_LocalPlayer->m_vecVelocity().Length2D() > .1f)
		move *= -450.f / std::max(std::abs(move.x), std::abs(move.y));

	cmd->forwardmove = move.x;
	cmd->sidemove = move.y;

	const auto backup = cmd->viewangles;
	cmd->viewangles = angles;
	//prediction_system.repredict(local, cmd);
	cmd->viewangles = backup;

	

	if (local->m_vecVelocity().Length2D() > g_LocalPlayer->m_vecVelocity().Length2D())
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
	}
	//prediction_system.repredict(local, cmd);
}
void MinWalk(CUserCmd* get_cmd, float get_speed);
bool HitChance(C_BasePlayer* pEnt, C_BaseCombatWeapon* pWeapon, QAngle Angle, int chance, CUserCmd* cmd);
void c_ragebot::aim(C_BasePlayer* local, CUserCmd* cmd, bool& send_packet)
{
	last_pitch = std::nullopt;

	auto weapon = local->m_hActiveWeapon();
	if (!weapon)
		return;

	auto wpn_info = weapon->GetCSWeaponData();
	if (!wpn_info)
		return;

	if (!weapon->CanFire())
		return;

	auto weapon_cfg = get_weapon_conf();

	if (!weapon_cfg.has_value())
		return;

	std::vector<aim_info> hitpoints = {};

	for (int i = 1; i < 65; i++)
	{
		auto player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!player || player->IsDormant())
			continue;
		if ((player->m_iTeamNum() == local->m_iTeamNum()) || !player->IsAlive() || player->m_bGunGameImmunity())
			continue;

		const auto latest = animation_system.get_latest_animation(player);

		if (!latest.has_value())
			continue;

		const auto oldest = animation_system.get_oldest_animation(player);

		const auto rtt = 2.f * g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		const auto breaking_lagcomp = latest.value()->lag && latest.value()->lag <= 16 && is_breaking_lagcomp(latest.value());
		const auto can_delay_shot = (latest.value()->lag > TIME_TO_TICKS(rtt) + g_GlobalVars->interval_per_tick);
		const auto delay_shot = (TIME_TO_TICKS(rtt) + TIME_TO_TICKS(g_GlobalVars->curtime - latest.value()->sim_time)
			+ g_GlobalVars->interval_per_tick >= latest.value()->lag);

		if (breaking_lagcomp && delay_shot && can_delay_shot)
			return;
			
		//if (breaking_lagcomp)
		//{
		//	c_trace_system::extrapolate(player, player->get_origin(), player->get_velocity(), player->get_flags(), player->get_flags() & c_base_player::on_ground);
		//}

		std::optional<aim_info> target;

		const auto alternative = scan_record(local, latest.value());

		if (!target.has_value() || (alternative.has_value()
			&& target.value().damage < alternative.value().damage))
			target = alternative;

		// are there two distinct records?
		// and if there are, is the last one moving?
		if (oldest.has_value() && latest.value() != oldest.value()
			&& oldest.value()->velocity.Length2D() >= .1f)
		{

			const auto alternative = scan_record(local, oldest.value());

			// is there no other record?
			if ((alternative.has_value() && !target.has_value())
				// is the current one standing?
				|| (target.has_value() && target.value().animation->velocity.Length2D() < .1f)
				// can we do more damage by backtracking?
				|| (alternative.has_value() && target.has_value()
					&& alternative.value().damage > target.value().damage))
				target = alternative;
		}

		// is he standing and crouched?
		if (target.has_value() && target.value().animation->velocity.Length2D() < .1f && fabsf(latest.value()->duck) > .001f)
		{
			// let's see if he was standing a moment ago...
			const auto uncrouched = animation_system.get_uncrouched_animation(player);

			if (uncrouched.has_value())
			{
				const auto alternative = scan_record(local, uncrouched.value());

				if (alternative.has_value() &&
					(!target.has_value() || alternative.value().damage > target.value().damage))
					target = alternative;
			}
		}

		if (target.has_value())
			hitpoints.push_back(target.value());
	}

	aim_info best_match = { Vector(), -FLT_MAX, nullptr, false, Vector(), 0.f, 0.f, HITBOX_HEAD, 0 };

	// find best target spot of all valid spots.
	for (auto& hitpoint : hitpoints)
		if (hitpoint.damage > best_match.damage)
			best_match = hitpoint;

	// stop if no target found.
	if (best_match.damage < 0.f)
		return;


	if (weapon->get_zoom_level() == 0 && g_Options.autoscope)
		cmd->buttons |= IN_ZOOM;

	// run autostop.
	autostop(local, cmd);
	//MinWalk(cmd, 30);

	// scope the weapon.

	
	// calculate angle.
	auto angle = Math::CalcAngle(local->GetEyePos(), best_match.position);

	// store pitch for eye correction.
	last_pitch = angle.pitch;

	// optimize multipoint and select final aimpoint.
	//optimize_multipoint(best_match);

	if (!can_hit(best_match.animation, best_match.position, weapon_cfg.value().rage_hitchance / 100.f, best_match.hitbox))
		return;

	// store shot info for resolver.
	if (!best_match.alt_attack)
	{
		resolver::shot shot{};
		shot.damage = best_match.damage;
		shot.start = local->GetEyePos();
		shot.end = best_match.position;
		shot.hitgroup = best_match.hitgroup;
		shot.hitbox = best_match.hitbox;
		shot.time = g_GlobalVars->curtime;
		shot.record = *best_match.animation;
		shot.manual = false;
		//c_resolver::register_shot(std::move(shot));
	}

	// set correct information to user_cmd.
	angle -= g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
	cmd->viewangles = angle;
	if (!g_Options.silent)
		g_EngineClient->SetViewAngles(&cmd->viewangles);

	cmd->tick_count = TIME_TO_TICKS(best_match.animation->sim_time) + TIME_TO_TICKS(calculate_lerp());
	cmd->buttons |= best_match.alt_attack ? IN_ATTACK2 : IN_ATTACK;

	animation_system.entity_target = best_match.animation->player;

	//best_match.animation->player->setup_bones(mat, 128, 156, global_vars_base->curtime);
	//best_match.animation->player->draw_hitboxes(mat);
}
