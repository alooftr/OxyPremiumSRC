#include "c_resolver.h"
#include <random>


static std::random_device rd;
static std::mt19937 rng(rd());


float flAngleMod(float flAngle)
{
	return((360.0f / 65536.0f) * ((int32_t)(flAngle * (65536.0f / 360.0f)) & 65535));
}

float ApproachAngle(float flTarget, float flValue, float flSpeed)
{
	flTarget = flAngleMod(flTarget);
	flValue = flAngleMod(flValue);

	float delta = flTarget - flValue;

	// Speed is assumed to be positive
	if (flSpeed < 0)
		flSpeed = -flSpeed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > flSpeed)
		flValue += flSpeed;
	else if (delta < -flSpeed)
		flValue -= flSpeed;
	else
		flValue = flTarget;

	return flValue;
}

static auto GetSmoothedVelocity = [](float min_delta, Vector a, Vector b) {
	Vector delta = a - b;
	float delta_length = delta.Length();

	if (delta_length <= min_delta) {
		Vector result;
		if (-min_delta <= delta_length) {
			return a;
		}
		else {
			float iradius = 1.0f / (delta_length + FLT_EPSILON);
			return b - ((delta * iradius) * min_delta);
		}
	}
	else {
		float iradius = 1.0f / (delta_length + FLT_EPSILON);
		return b + ((delta * iradius) * min_delta);
	}
};

void c_resolver::resolve(C_BasePlayer* player)
{
	if (!player || !player->GetPlayerAnimState())
		return;



	auto animState = player->GetPlayerAnimState();

	if (!animState)
		return;

	// Rebuild setup velocity to receive flMinBodyYaw and flMaxBodyYaw
	Vector velocity = player->m_vecVelocity();
	float spd = velocity.LengthSqr();
	if (spd > std::powf(1.2f * 260.0f, 2.f)) {
		Vector velocity_normalized = velocity.Normalized();
		velocity = velocity_normalized * (1.2f * 260.0f);
	}

	float Resolveyaw = animState->m_flGoalFeetYaw;

	auto delta_time
		= fmaxf(g_GlobalVars->curtime - animState->m_flLastClientSideAnimationUpdateTime, 0.f);

	float deltatime = fabs(delta_time);
	float stop_to_full_running_fraction = 0.f;
	bool is_standing = true;
	float v25 = std::clamp(player->m_flDuckAmount() + animState->m_fLandingDuckAdditiveSomething, 0.0f, 1.0f);
	float v26 = animState->m_fDuckAmount;
	float v27 = deltatime * 6.0f;
	float v28;

	// clamp
	if ((v25 - v26) <= v27) {
		if (-v27 <= (v25 - v26))
			v28 = v25;
		else
			v28 = v26 - v27;
	}
	else {
		v28 = v26 + v27;
	}

	float flDuckAmount = std::clamp(v28, 0.0f, 1.0f);

	Vector animationVelocity = velocity;
	float speed = std::fminf(animationVelocity.Length(), 260.0f);

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)
		return;

	auto wpndata = weapon->GetCSWeaponData();

	if (!wpndata)
		return;

	float flMaxMovementSpeed = 260.0f;
	if (weapon) {
		flMaxMovementSpeed = std::fmaxf(wpndata->flMaxPlayerSpeed, 0.001f);
	}

	float flRunningSpeed = speed / (flMaxMovementSpeed * 0.520f);
	float flDuckingSpeed_2 = speed / (flMaxMovementSpeed * 0.340f);

	flRunningSpeed = std::clamp(flRunningSpeed, 0.0f, 1.0f);

	float flYawModifier = (((stop_to_full_running_fraction * -0.3f) - 0.2f) * flRunningSpeed) + 1.0f;
	if (flDuckAmount > 0.0f) {
		float flDuckingSpeed = std::clamp(flDuckingSpeed_2, 0.0f, 1.0f);
		flYawModifier += (flDuckAmount * flDuckingSpeed) * (0.5f - flYawModifier);
	}

	float flMaxBodyYaw = *reinterpret_cast<float*>(&animState->pad10[512]);
	float flMinBodyYaw = *reinterpret_cast<float*>(&animState->pad10[516]);



	float flEyeYaw = player->m_angEyeAngles().yaw;

	float flEyeDiff = std::remainderf(flEyeYaw - Resolveyaw, 360.f);

	if (flEyeDiff <= flMaxBodyYaw) {
		if (flMinBodyYaw > flEyeDiff)
			Resolveyaw = fabs(flMinBodyYaw) + flEyeYaw;
	}
	else {
		Resolveyaw = flEyeYaw - fabs(flMaxBodyYaw);
	}

	if (speed > 0.1f || fabs(velocity.z) > 100.0f) {
		Resolveyaw = ApproachAngle(
			flEyeYaw,
			Resolveyaw,
			((stop_to_full_running_fraction * 20.0f) + 30.0f)
			* deltatime);
	}
	else {
		Resolveyaw = ApproachAngle(
			player->m_flLowerBodyYawTarget(),
			Resolveyaw,
			deltatime * 100.0f);
	}

	if (stop_to_full_running_fraction > 0.0 && stop_to_full_running_fraction < 1.0)
	{
		const auto interval = g_GlobalVars->interval_per_tick * 2.f;

		if (is_standing)
			stop_to_full_running_fraction = stop_to_full_running_fraction - interval;
		else
			stop_to_full_running_fraction = interval + stop_to_full_running_fraction;

		stop_to_full_running_fraction = std::clamp(stop_to_full_running_fraction, 0.f, 1.f);
	}

	if (speed > 135.2f && is_standing)
	{
		stop_to_full_running_fraction = fmaxf(stop_to_full_running_fraction, .0099999998f);
		is_standing = false;
	}

	if (speed < 135.2f && !is_standing)
	{
		stop_to_full_running_fraction = fminf(stop_to_full_running_fraction, .99000001f);
		is_standing = true;
	}


	float Left = flEyeYaw + flMinBodyYaw;
	float Right = flEyeYaw + flMaxBodyYaw;
	float brute_yaw = Resolveyaw;
	/*switch (info->brute_state)
	{
	case resolver_start:
		info->brute_yaw = Resolveyaw;
		break;
	case resolver_still:
		info->brute_yaw = Left;
		break;
	case resolver_still2:
		info->brute_yaw = Right;
		break;
	default:
		break;
	}*/
	//Math::Normalize3(info->brute_yaw);
	brute_yaw = std::remainderf(brute_yaw, 360.f);
	player->GetPlayerAnimState()->m_flGoalFeetYaw = brute_yaw;
}
/*
bool can_hit_hitbox(const Vector start, const Vector end, matrix3x4_t* bones, studiohdr_t* hdr, int box);

void c_resolver::resolve_shot(resolver::shot& shot)
{
	if (!g_Options.rage_enabled || shot.manual)
		return;

	const auto player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(shot.record.index));

	if (player != shot.record.player)
		return;

	const auto hdr = g_MdlInfo->GetStudiomodel(shot.record.player->GetModel());

	if (!hdr)
		return;

	const auto info = animation_system.get_animation_info(player);

	if (!info)
		return;

	const auto angle = Math::CalcAngle(shot.start, shot.server_info.impacts.back());
	Vector forward;
	Math::AngleVectors(angle, forward);
	auto end = shot.server_info.impacts.back() + forward * 2000.f;
	const auto spread_miss = !can_hit_hitbox(shot.start, end, shot.record.bones, hdr, shot.hitbox);

	if (shot.server_info.damage > 0)
	{
		/*static const auto hit_msg = __("Hit %s in %s for %d damage.");
		_rt(hit, hit_msg);
		char msg[255];

		switch (shot.server_info.hitgroup)
		{
		case hitgroup_head:
			sprintf_s(msg, hit, player->get_info().name, _("head"), shot.server_info.damage);
			break;
		case hitgroup_leftleg:
		case hitgroup_rightleg:
			sprintf_s(msg, hit, player->get_info().name, _("leg"), shot.server_info.damage);
			break;
		case hitgroup_stomach:
			sprintf_s(msg, hit, player->get_info().name, _("stomach"), shot.server_info.damage);
			break;
		default:
			sprintf_s(msg, hit, player->get_info().name, _("body"), shot.server_info.damage);
			break;
		}
		*//*
		//logging->info(msg);
	}
	else if (spread_miss)
	{
		//logging->info(_("Missed shot due to spread."));
		++info->missed_due_to_spread;
	}


	if (!shot.record.player->IsAlive() || shot.record.player->get_info().fakeplayer
		|| !shot.record.has_anim_state || !shot.record.player->GetPlayerAnimState() || !info)
		return;

	// note old brute_yaw.
	const auto old_brute_yaw = info->brute_yaw;

	// check deviation from server.
	auto backup = c_animation_system::animation(shot.record.player);
	shot.record.apply(player);
	const auto trace = g_AutoWall.CanHitDetail(end);
	auto does_match = (trace.has_value() && trace.value().hitgroup == shot.server_info.hitgroup)
		|| (!trace.has_value() && spread_miss);

	// start brute.

	if (!does_match)
	{
		switch (info->brute_state)
		{
		case resolver_start:
			info->brute_state = resolver_still;
			//logging->debug("BRUTE: RIGHT");
			break;
		case resolver_still:
			info->brute_state = resolver_still2;
			//logging->debug("BRUTE: LEFT");			
			break;
		case resolver_still2:
			info->brute_state = resolver_start;
			//logging->debug("BRUTE: AUTO");
			break;
		default:
			break;
		}

	}



	// apply changes.
	if (!info->frames.empty())
	{
		c_animation_system::animation* previous = nullptr;

		// jump back to the beginning.
		*player->GetPlayerAnimState() = info->frames.back().anim_state;

		for (auto it = info->frames.rbegin(); it != info->frames.rend(); ++it)
		{
			auto& frame = *it;
			const auto frame_player = reinterpret_cast<C_BasePlayer*>(
				g_EntityList->GetClientEntity(frame.index));

			if (frame_player == frame.player
				&& frame.player == player)
			{
				// re-run complete animation code and repredict all animations in between!
				frame.anim_state = *player->GetPlayerAnimState();
				frame.apply(player);
				player->m_fFlags() = frame.flags;
				*player->GetAnimOverlays() = frame.layers;
				player->m_flSimulationTime() = frame.sim_time;

				info->update_animations(&frame, previous);
				frame.abs_ang.yaw = player->GetPlayerAnimState()->m_flGoalFeetYaw;
				frame.flags = player->m_fFlags();
				*player->GetAnimOverlays() = frame.layers;
				frame.build_server_bones(player);
				previous = &frame;
			}
		}
	}
}

void c_resolver::register_shot(resolver::shot&& s)
{
	shots.emplace_front(std::move(s));
}
/*
void LogDamage(int dmg, int hitbox, C_BasePlayer* player) {
	static const auto hit_msg = __("Hit %s in %s for %d damage.");
	_rt(hit, hit_msg);
	char msg[255];

	switch (hitbox)
	{
	case hitgroup_head:
		sprintf_s(msg, hit, player->get_info().name, _("head"), dmg);
		break;
	case hitgroup_leftleg:
	case hitgroup_rightleg:
		sprintf_s(msg, hit, player->get_info().name, _("leg"), dmg);
		break;
	case hitgroup_stomach:
		sprintf_s(msg, hit, player->get_info().name, _("stomach"), dmg);
		break;
	default:
		sprintf_s(msg, hit, player->get_info().name, _("body"), dmg);
		break;
	}

	logging->info(msg);
}
*//*
void c_resolver::on_player_hurt(IGameEvent* event)
{

	const auto attacker = event->GetInt(("attacker"));
	const auto attacker_index = g_EngineClient->GetPlayerForUserID(attacker);

	if (attacker_index != g_EngineClient->GetLocalPlayer())
		return;

	if (shots.empty())
		return;

	const auto userid = event->GetInt(("userid"));
	const auto index = g_EngineClient->GetPlayerForUserID(userid);

	//LogDamage(event->GetInt(_("dmg_health")), event->GetInt(_("hitgroup")), reinterpret_cast<C_BasePlayer*>(client_entity_list()->get_client_entity(index)));

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (it->confirmed && !it->skip)
		{
			last_confirmed = &*it;
			break;
		}
	}

	if (!last_confirmed)
		return;



	if (index != last_confirmed->record.index)
		return;

	last_confirmed->server_info.index = index;
	last_confirmed->server_info.damage = event->GetInt(("dmg_health"));
	last_confirmed->server_info.hitgroup = event->GetInt(("hitgroup"));
}

void c_resolver::on_bullet_impact(IGameEvent* event)
{

	const auto userid = event->GetInt(("userid"));
	const auto index = g_EngineClient->GetPlayerForUserID(userid);

	if (index != g_EngineClient->GetLocalPlayer())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (it->confirmed && !it->skip)
		{
			last_confirmed = &*it;
			break;
		}
	}

	if (!last_confirmed)
		return;

	last_confirmed->impacted = true;
	last_confirmed->server_info.impacts.emplace_back(event->GetFloat(("x")),
		event->GetFloat(("y")),
		event->GetFloat(("z")));
}

void c_resolver::on_weapon_fire(IGameEvent* event)
{
	const auto userid = event->GetInt(("userid"));
	const auto index = g_EngineClient->GetPlayerForUserID(userid);

	if (index != g_EngineClient->GetLocalPlayer())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_unconfirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		if (!it->confirmed)
		{
			last_unconfirmed = &*it;
			break;
		}

		it->skip = true;
	}

	if (!last_unconfirmed)
		return;

	last_unconfirmed->confirmed = true;
}

void c_resolver::Listener()
{

}

void c_resolver::on_render_start()
{
	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->time + 1.f < g_GlobalVars->curtime)
			it = shots.erase(it);
		else
			it = next(it);
	}

	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->confirmed && it->impacted)
		{
			resolve_shot(*it);
			//c_esp::draw_local_impact(it->start, it->server_info.impacts.back());
			it = shots.erase(it);
		}
		else
			it = next(it);
	}
}

*/