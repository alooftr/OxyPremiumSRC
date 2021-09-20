#include "rifktrace.h"


__forceinline uint32_t IEngineTrace::get_filter_simple_vtable()
{
	static const auto filter_simple = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(
		Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3d);
	return filter_simple;
}

std::optional<c_trace_system::wall_pen> c_trace_system::wall_penetration(const Vector src, const Vector end,
	c_animation_system::animation* target, C_BasePlayer* override_player) const
{
	static CCSWeaponInfo override_gun{};
	override_gun.iDamage = 15000.f;
	override_gun.flRangeModifier = 1.0f;
	override_gun.flPenetration = 10.0f;
	override_gun.flArmorRatio = 2.0f;
	override_gun.flRange = 8192.f;

	auto local = g_LocalPlayer;

	if (!local || !local->IsAlive())
		return std::nullopt;

	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon && !override_player)
		return std::nullopt;

	const auto data = override_player ? &override_gun :
		weapon->GetCSWeaponData();

	if (!data)
		return std::nullopt;

	std::optional<wall_pen> result = std::nullopt;

	if (!override_player)
		run_emulated(target, [&]() -> void
			{
				// setup trace filter.
				//uint32_t filter[4] = { g_EngineTrace->get_filter_simple_vtable(),
				//	reinterpret_cast<uint32_t>(local), 0, 0 };

				// run bullet simulation
				result = fire_bullet(data, src, end,
					nullptr, target->player);
			});
	else
	{
		// setup trace filter.
		CTraceFilter filter;

		// run bullet simulation
		result = fire_bullet(data, src, end, &filter, override_player, true);
	}

	// filter low dmg.
	if (result.has_value() && result.value().damage < 1.f)
		return std::nullopt;

	// report result
	return result;
}

void c_trace_system::run_emulated(c_animation_system::animation* target, const std::function<void()> fn)
{
	// backup player
	const auto backup_origin = target->player->m_vecOrigin();
	const auto backup_abs_origin = target->player->GetAbsOrigin();
	const auto backup_obb_mins = target->player->get_mins();
	const auto backup_obb_maxs = target->player->get_maxs();
	const auto backup_cache = target->player->get_bone_cache();

	// setup trace data
	target->player->m_vecOrigin() = target->origin;
	target->player->SetOrigin(target->origin);
	target->player->get_mins() = target->obb_mins;
	target->player->get_maxs() = target->obb_maxs;
	target->player->get_bone_cache() = reinterpret_cast<matrix3x4_t**>(target->bones);

	// run emulation
	fn();

	// restore trace data
	target->player->m_vecOrigin() = backup_origin;
	target->player->SetOrigin(backup_abs_origin);
	target->player->get_mins() = backup_obb_mins;
	target->player->get_maxs() = backup_obb_maxs;
	target->player->get_bone_cache() = backup_cache;
}

void c_trace_system::extrapolate(C_BasePlayer* player, Vector& origin, Vector& velocity, int& flags, bool on_ground)
{
	static const auto sv_gravity =		g_CVar->FindVar(("sv_gravity"));
	static const auto sv_jump_impulse = g_CVar->FindVar(("sv_jump_impulse"));

	if (!(flags & FL_ONGROUND))
		velocity.z -= TICKS_TO_TIME(sv_gravity->GetFloat());
	else if (player->m_fFlags() & FL_ONGROUND && !on_ground)
		velocity.z = sv_jump_impulse->GetFloat();

	const auto src = origin;
	auto end = src + velocity * g_GlobalVars->interval_per_tick;

	Ray_t r;
	r.Init(src, end, player->get_mins(), player->get_maxs());

	trace_t t{};
	CTraceFilter filter;
	filter.pSkip = player;

	g_EngineTrace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

	if (t.fraction != 1.f)
	{
		for (auto i = 0; i < 2; i++)
		{
			velocity -= t.plane.normal * velocity.Dot(t.plane.normal);

			const auto dot = velocity.Dot(t.plane.normal);
			if (dot < 0.f)
				velocity -= Vector(dot * t.plane.normal.x,
					dot * t.plane.normal.y, dot * t.plane.normal.z);

			end = t.endpos + velocity * TICKS_TO_TIME(1.f - t.fraction);

			r.Init(t.endpos, end, player->get_mins(), player->get_maxs());
			g_EngineTrace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

			if (t.fraction == 1.f)
				break;
		}
	}

	origin = end = t.endpos;
	end.z -= 2.f;

	r.Init(origin, end, player->get_mins(), player->get_maxs());
	g_EngineTrace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

	flags &= ~FL_ONGROUND;

	if (t.DidHit() && t.plane.normal.z > .7f)
		flags |= FL_ONGROUND;
}

void clip_ray_to_player(Vector& src, Vector& end, const uint32_t mask, C_BasePlayer* player, CTraceFilter* filter, trace_t* t)
{
	if (filter && !filter->ShouldHitEntity(player, mask))
		return;

	trace_t t_new{};
	Ray_t r{};
	r.Init(src, end);

	g_EngineTrace->ClipRayToEntity(r, mask, player, &t_new);
	if (t_new.fraction < t->fraction)
		* t = t_new;
}

void ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	static DWORD dwAddress = (DWORD)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}


std::optional<c_trace_system::wall_pen> c_trace_system::fire_bullet(CCSWeaponInfo* data, Vector src,
	const Vector pos, CTraceFilter* filter, C_BasePlayer* target, bool point)
{
	QAngle angles;
	Math::VectorAngles(pos - src, angles);

	if (angles.IsZero())
		return std::nullopt;

	Vector direction;
	Math::AngleVectors(angles, direction);

	if (!direction.IsValid())
		return std::nullopt;

	direction.Normalized();

	auto penetrate_count = 5;
	auto length = 0.f, damage = static_cast<float>(data->iDamage);
	trace_t enter_trace{};

	const auto start = src;

	while (penetrate_count > 0 && damage >= 1.0f)
	{
		const auto length_remaining = data->flRange - length;
		auto end = src + direction * length_remaining;

		Ray_t r;
		r.Init(src, end);
		g_EngineTrace->TraceRay(r, MASK_SHOT_HULL | CONTENTS_HITBOX, filter, &enter_trace);

		if (enter_trace.fraction == 1.f && !point)
			break;

		if (point && (enter_trace.fraction == 1.f ||
			(start - enter_trace.endpos).Length() > (start - pos).Length()))
			return wall_pen{
				scale_damage(target, damage, data->flArmorRatio, HITGROUP_HEAD, false),
				HITBOX_HEAD, HITGROUP_HEAD };

		auto end_extended = end + direction * 40.f;

		//clip_ray_to_player(src, end_extended, MASK_SHOT_HULL | CONTENTS_HITBOX, target, filter, &enter_trace);
		ClipTraceToPlayers(src, end_extended, MASK_SHOT_HULL | CONTENTS_HITBOX, filter, &enter_trace);

		length += enter_trace.fraction * length_remaining;
		damage *= std::powf(data->flRangeModifier, length * .002f);

		if (enter_trace.hitgroup <= 7 && enter_trace.hitgroup > 0)
		{
			if (!enter_trace.hit_entity || enter_trace.hit_entity != target)
				break;

			// we have reached our target!
			return wall_pen{
				scale_damage(target, damage, data->flArmorRatio, enter_trace.hitgroup, false),
				enter_trace.hitbox, enter_trace.hitgroup };
		}

		const auto enter_surface = g_PhysSurface->GetSurfaceData(enter_trace.surface.surfaceProps);

		if (!enter_surface || enter_surface->game.flPenetrationModifier < .1f)
			break;

		if (!handle_bullet_penetration(data, enter_trace, src, direction, penetrate_count, damage, data->flPenetration))
			break;
	}

	// nothing found
	return std::nullopt;
}

bool c_trace_system::handle_bullet_penetration(CCSWeaponInfo* weapon_data, trace_t& enter_trace,
	Vector& eye_position, const Vector direction, int& penetrate_count, float& current_damage, const float penetration_power)
{
	static const auto ff_damage_reduction_bullets = g_CVar->FindVar("ff_damage_reduction_bullets");
	static const auto ff_damage_bullet_penetration = g_CVar->FindVar("ff_damage_bullet_penetration");

	const auto damage_reduction_bullets = ff_damage_reduction_bullets->GetFloat();
	const auto damage_bullet_penetration = ff_damage_bullet_penetration->GetFloat();

	trace_t exit_trace{};
	auto enemy = reinterpret_cast<C_BasePlayer*>(enter_trace.hit_entity);
	const auto enter_surface_data = g_PhysSurface->GetSurfaceData(enter_trace.surface.surfaceProps);
	const int enter_material = enter_surface_data->game.material;

	const auto enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier, combined_penetration_modifier;
	const bool is_solid_surf = enter_trace.contents >> 3 & CONTENTS_SOLID;
	const bool is_light_surf = enter_trace.surface.flags >> 7 & SURF_LIGHT;

	if ((!penetrate_count && !is_light_surf && !is_solid_surf && enter_material != char_tex_grate && enter_material != char_tex_glass)
		|| weapon_data->flPenetration <= 0.f
		|| (!trace_to_exit(enter_trace, exit_trace, enter_trace.endpos, direction, weapon_data->iDamage > 10000.f)
			&& !(g_EngineTrace->GetPointContents(enter_trace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL)))
		return false;

	const auto exit_surface_data = g_PhysSurface->GetSurfaceData(exit_trace.surface.surfaceProps);
	const auto exit_material = exit_surface_data->game.material;
	const auto exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == char_tex_grate || enter_material == char_tex_glass)
	{
		combined_penetration_modifier = 3.f;
		final_damage_modifier = 0.05f;
	}
	else if (is_solid_surf || is_light_surf)
	{
		combined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else if (enter_material == char_tex_flesh && (enemy->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() && damage_reduction_bullets == 0.f))
	{
		if (damage_bullet_penetration == 0.f)
			return false;

		combined_penetration_modifier = damage_bullet_penetration;
		final_damage_modifier = 0.16f;
	}
	else
	{
		combined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) / 2.f;
		final_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == char_tex_cardboard || exit_material == char_tex_wood)
			combined_penetration_modifier = 3.f;
		else if (exit_material == char_tex_plastic)
			combined_penetration_modifier = 2.f;
	}

	auto thickness = (exit_trace.endpos - enter_trace.endpos).Length();
	thickness *= thickness;
	thickness *= fmaxf(0.f, 1.0f / combined_penetration_modifier);
	thickness /= 24.0f;

	const auto lost_damage = fmaxf(0.0f, current_damage * final_damage_modifier + fmaxf(0.f, 1.0f / combined_penetration_modifier)
		* 3.0f * fmaxf(0.0f, 3.0f / penetration_power) * 1.25f + thickness);

	if (lost_damage > current_damage)
		return false;

	if (lost_damage > 0.f)
		current_damage -= lost_damage;

	if (current_damage < 1.f)
		return false;

	eye_position = exit_trace.endpos;
	--penetrate_count;
	return true;
}

bool BreakableEntity(IClientEntity* entity)
{
	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
		return false;

	if (pClass == nullptr)
		return false;

	return pClass->m_ClassID == ClassId_CBreakableProp || pClass->m_ClassID == ClassId_CBreakableSurface;
}

bool c_trace_system::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction, const bool is_local)
{
	Vector start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
			firstContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		int pointContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents)
		{
			end = start - (direction * rayExtension);

			Ray_t r;
			r.Init(start, end);
			//CTraceFilter filter;
			//filter.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(r, MASK_SHOT_HULL | CONTENTS_HITBOX,nullptr, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				Ray_t r;
				r.Init(start, startPosition);
				CTraceFilter filter;
				filter.pSkip = exitTrace.hit_entity;
				g_EngineTrace->TraceRay(r, MASK_SHOT_HULL, &filter, &exitTrace);


				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity(enterTrace.hit_entity) && BreakableEntity(exitTrace.hit_entity))
					return true;

				if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
				{
					float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity(enterTrace.hit_entity))
				{
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}

	return false;
}

/*
bool c_trace_system::trace_to_exit(trace_t& enter_trace, trace_t& exit_trace, const Vector start_position,
	const Vector direction, const bool is_local)
{
	const auto max_distance = is_local ? 200.f : 90.f;
	const auto ray_extension = is_local ? 8.f : 4.f;

	float current_distance = 0;
	auto first_contents = 0;

	while (current_distance <= max_distance)
	{
		current_distance += ray_extension;

		auto start = start_position + direction * current_distance;

		if (!first_contents)
			first_contents = engine_trace()->get_point_contents(start, MASK_SHOT_HULL | CONTENTS_HITBOX);

		const auto point_contents = engine_trace()->get_point_contents(start, MASK_SHOT_HULL | CONTENTS_HITBOX);

		if (!(point_contents & mask_shot_hull) || (point_contents & contents_hitbox && point_contents != first_contents))
		{
			const auto end = start - direction * ray_extension;

			ray r{};
			r.init(start, end);
			uint32_t filter[4] = { c_engine_trace::get_filter_simple_vtable(),
				uint32_t(C_BasePlayer::get_local_player()), 0, 0 };
			engine_trace()->trace_ray(r, MASK_SHOT_HULL | CONTENTS_HITBOX, reinterpret_cast<trace_filter*>(filter), &exit_trace);

			if (exit_trace.startsolid && exit_trace.surface.flags & surf_hitbox)
			{
				r.init(start, start_position);
				filter[1] = reinterpret_cast<uint32_t>(exit_trace.entity);
				engine_trace()->trace_ray(r, MASK_SHOT_HULL | CONTENTS_HITBOX, reinterpret_cast<trace_filter*>(filter), &exit_trace);

				if (exit_trace.did_hit() && !exit_trace.startsolid)
				{
					start = exit_trace.endpos;
					return true;
				}

				continue;
			}

			if (exit_trace.did_hit() && !exit_trace.startsolid)
			{
				if (enter_trace.entity->is_breakable() && exit_trace.hit_entity->Is())
					return true;

				if (enter_trace.surface.flags & surf_nodraw
					|| (!(exit_trace.surface.flags & surf_nodraw)
						&& exit_trace.plane.normal.dot(direction) <= 1.f))
				{
					const auto mult_amount = exit_trace.fraction * 4.f;
					start -= direction * mult_amount;
					return true;
				}

				continue;
			}

			if (!exit_trace.did_hit() || exit_trace.startsolid)
			{
				if (enter_trace.did_hit_non_world_entity() && enter_trace.entity->is_breakable())
				{
					exit_trace = enter_trace;
					exit_trace.endpos = start;
					return true;
				}
			}
		}
	}

	return false;
}
*/
// credits to n0xius
float c_trace_system::scale_damage(C_BasePlayer* target, float damage, const float weapon_armor_ratio, int hitgroup, bool is_zeus)
{
	const auto is_armored = [&]() -> bool
	{
		if (target->m_ArmorValue() > 0.f)
		{
			switch (hitgroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return true;
			case HITGROUP_HEAD:
				return target->m_bHasHelmet();
			default:
				break;
			}
		}

		return false;
	};

	if (!is_zeus)
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			if (target->m_bHasHeavyArmor())
				damage = (damage * 4.f) * .5f;
			else
				damage *= 4.f;
			break;
		case HITGROUP_STOMACH:
			damage *= 1.25f;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			damage *= .75f;
			break;
		default:
			break;
		}

	if (is_armored())
	{
		auto modifier = 1.f, armor_bonus_ratio = .5f, armor_ratio = weapon_armor_ratio * .5f;

		if (target->m_bHasHeavyArmor())
		{
			armor_bonus_ratio = 0.33f;
			armor_ratio = (weapon_armor_ratio * 0.5f) * 0.5f;
			modifier = 0.33f;
		}

		auto new_damage = damage * armor_ratio;

		if (target->m_bHasHeavyArmor())
			new_damage *= 0.85f;

		if ((damage - damage * armor_ratio) * (modifier * armor_bonus_ratio) > target->m_ArmorValue())
			new_damage = damage - target->m_ArmorValue() / armor_bonus_ratio;

		damage = new_damage;
	}

	return damage;
}
