#pragma once
#include "Ragebot.h"


Ragebot g_Ragebot;

float DotProduct(const float* a, const float* b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}


// lots of math stuff, i need to fix the math.cpp file because you cant access like 50% of stuff from it. sorry for ugly code. -drifttwo/voozez
// fixed this

// this is so fucking ugly, but ya know what, who cares.

float Ragebot::GetInterp()
{
	int updaterate = g_CVar->FindVar("cl_updaterate")->GetInt();
	ConVar* minupdate = g_CVar->FindVar("sv_minupdaterate");
	ConVar* maxupdate = g_CVar->FindVar("sv_maxupdaterate");

	if (minupdate && maxupdate)
		updaterate = maxupdate->GetInt();

	float ratio = g_CVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_CVar->FindVar("cl_interp")->GetFloat();
	ConVar* cmin = g_CVar->FindVar("sv_client_min_interp_ratio");
	ConVar* cmax = g_CVar->FindVar("sv_client_max_interp_ratio");

	if (cmin && cmax && cmin->GetFloat() != 1)
		ratio = std::clamp(ratio, cmin->GetFloat(), cmax->GetFloat());

	return std::max(lerp, (ratio / updaterate));
}

bool ShouldBaim(C_BasePlayer* pEnt, matrix3x4_t* boneMatrix) // probably dosnt make sense
{

	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

	std::vector<int> baim_hitboxes;

	baim_hitboxes.push_back((int)HITBOX_STOMACH);
	baim_hitboxes.push_back((int)HITBOX_PELVIS);
	baim_hitboxes.push_back((int)HITBOX_CHEST);

	float Damage = 0;
	for (auto HitBoxID : baim_hitboxes)
	{
		Damage = g_AutoWall.CanHit(pEnt->get_hitbox_position(HitBoxID, boneMatrix).value());
	}

	if (pEnt->m_iHealth() + 5 <= Damage)
		return true;

	return false;
}

std::vector<int> Ragebot::GetHitboxes(C_BasePlayer* pl, matrix3x4_t* mat)
{
	std::vector<int> hitboxes;
	bool baim = ShouldBaim(pl, mat);
	if (!baim) {
		if (g_Options.rage_hitscan[0])
			hitboxes.push_back((int)Hitboxes::HITBOX_HEAD);
		if (g_Options.rage_hitscan[1])
			hitboxes.push_back((int)Hitboxes::HITBOX_NECK);
		if (g_Options.rage_hitscan[2])
		{
			hitboxes.push_back((int)Hitboxes::HITBOX_UPPER_CHEST);
			hitboxes.push_back((int)Hitboxes::HITBOX_CHEST);
			hitboxes.push_back((int)Hitboxes::HITBOX_LOWER_CHEST);
		}
		if (g_Options.rage_hitscan[3])
			hitboxes.push_back((int)Hitboxes::HITBOX_STOMACH);
		if (g_Options.rage_hitscan[4])
			hitboxes.push_back((int)Hitboxes::HITBOX_PELVIS);
		if (g_Options.rage_hitscan[5])
		{
			hitboxes.push_back((int)Hitboxes::HITBOX_LEFT_FOREARM);
			hitboxes.push_back((int)Hitboxes::HITBOX_RIGHT_FOREARM);
			hitboxes.push_back((int)Hitboxes::HITBOX_LEFT_UPPER_ARM);
			hitboxes.push_back((int)Hitboxes::HITBOX_RIGHT_UPPER_ARM);
			hitboxes.push_back((int)Hitboxes::HITBOX_LEFT_HAND);
			hitboxes.push_back((int)Hitboxes::HITBOX_RIGHT_HAND);
		}
		if (g_Options.rage_hitscan[6])
		{
			hitboxes.push_back((int)Hitboxes::HITBOX_RIGHT_CALF);
			hitboxes.push_back((int)Hitboxes::HITBOX_LEFT_CALF);
			hitboxes.push_back((int)Hitboxes::HITBOX_RIGHT_THIGH);
			hitboxes.push_back((int)Hitboxes::HITBOX_LEFT_THIGH);
			hitboxes.push_back((int)Hitboxes::HITBOX_LEFT_FOOT);
			hitboxes.push_back((int)Hitboxes::HITBOX_RIGHT_FOOT);
		}
	}
	else {
		
			hitboxes.push_back((int)Hitboxes::HITBOX_UPPER_CHEST);
			hitboxes.push_back((int)Hitboxes::HITBOX_CHEST);
			hitboxes.push_back((int)Hitboxes::HITBOX_LOWER_CHEST);		
			hitboxes.push_back((int)Hitboxes::HITBOX_STOMACH);
			hitboxes.push_back((int)Hitboxes::HITBOX_PELVIS);
	}

	return hitboxes;
}

PointInfo BestHitPoint(C_BasePlayer* player, int prioritized, matrix3x4_t matrix[])
{
	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel(player->GetModel());

	if (!pStudioModel)
		return 0.f;

	mstudiohitboxset_t* hitset = pStudioModel->GetHitboxSet(0);

	if (!hitset)
		return 0.f;

	mstudiobbox_t* hitbox = hitset->GetHitbox(prioritized);
	if (!hitbox)
		return 0.f;


	std::vector<Vector> vecArray;

	float mod = hitbox->m_flRadius != -1.f ? hitbox->m_flRadius : 0.f;

	Vector max;
	Vector min;

	Math::VectorTransform(hitbox->bbmax + mod, matrix[hitbox->bone], max);
	Math::VectorTransform(hitbox->bbmin - mod, matrix[hitbox->bone], min);

	auto center = (min + max) * 0.5f;

	QAngle curAngles = Math::CalcAngle(center, g_LocalPlayer->GetEyePos());

	Vector forward;
	Math::AngleVectors(curAngles, forward);

	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);

	Vector top = Vector(0, 0, 1);
	Vector bot = Vector(0, 0, -1);

	const float POINT_SCALE = g_Options.rage_mpsize / 10.f;
	if (g_Options.rage_mpsize)
	{
		switch (prioritized)
		{
		case HITBOX_HEAD:
			for (auto i = 0; i < 4; ++i)
			{
				vecArray.emplace_back(center);
			}
			vecArray[1] += top * (hitbox->m_flRadius * POINT_SCALE);
			vecArray[2] += right * (hitbox->m_flRadius * POINT_SCALE);
			vecArray[3] += left * (hitbox->m_flRadius * POINT_SCALE);
			break;

		default:

			for (auto i = 0; i < 3; ++i)
			{
				vecArray.emplace_back(center);
			}
			vecArray[1] += right * (hitbox->m_flRadius * POINT_SCALE);
			vecArray[2] += left * (hitbox->m_flRadius * POINT_SCALE);
			break;
		}
	}
	else
		vecArray.emplace_back(center);

	PointInfo pointinfo;

	for (Vector cur : vecArray)
	{
		float flCurDamage = g_AutoWall.CanHit(cur);

		if (!flCurDamage)
			continue;

		auto mindamage = std::min(player->m_iHealth(),g_Options.rage_mindmg);

		if ((flCurDamage > pointinfo.damage) && (flCurDamage > mindamage))
		{
			pointinfo.damage = flCurDamage;
			pointinfo.point = cur;


			if (pointinfo.damage > player->m_iHealth())
				return pointinfo;
		}

	}

	return pointinfo;
}



PointInfo Ragebot::HitScan(C_BasePlayer* pBaseEntity)
{


	matrix3x4_t matrix[128];


	*(Vector*)((uintptr_t)pBaseEntity + 0xA0) = pBaseEntity->m_vecOrigin();
	*(int*)((uintptr_t)pBaseEntity + 0xA68) = 0;
	*(int*)((uintptr_t)pBaseEntity + 0xA30) = 0;
	pBaseEntity->InvalidateBoneCache();
	if (!pBaseEntity->SetupBones(matrix, 128, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
		return PointInfo();

	auto hitboxes = GetHitboxes(pBaseEntity, matrix);

	PointInfo bestPoint;

	for (auto hitbox : hitboxes)
	{
		auto point = BestHitPoint(pBaseEntity, hitbox, matrix);

		if (point.damage > bestPoint.damage)
		{
			bestPoint = point;
		}
	}

	return bestPoint;
}

Target_t Ragebot::FindTarget(CUserCmd* cmd)
{
	float bestDistance = 8192.f;
	Target_t bestTarget;
	PointInfo bestInfo;

	for (int it = 1; it < g_EngineClient->GetMaxClients(); ++it)
	{
		C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(it);

		if(   !pEntity
			|| pEntity == g_LocalPlayer
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive()
			|| pEntity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		{
			continue;
		}

		float distance = pEntity->GetEyePos().DistTo(g_LocalPlayer->GetEyePos());

		if (distance < bestDistance)
		{
			auto pointinfo = HitScan(pEntity);

			if (pointinfo.damage > bestInfo.damage)
			{
				bestDistance = distance;
				bestInfo = pointinfo;
				bestTarget = Target_t(pEntity, pointinfo.point);
			}
		}
	}

	return bestTarget;
}

void Autostop(CUserCmd* cmd)
{
	if (!g_Options.rage_autostop)
		return;

	cmd->forwardmove = 10;
	cmd->sidemove = 10;
	//	g::autostoping = true;

		// P1000000 SELF CODE DUDE!!!!!!!11
}
bool Ragebot::HitChance(C_BasePlayer* pEnt, C_BaseCombatWeapon* pWeapon, QAngle Angle, float chance)
{
	float Seeds = (chance <= 1.f) ? 356.f : 256.f;

	Vector forward, right, up;

	Math::AngleVectors(Angle, forward, right, up);

	int Hits = 0, neededHits = (Seeds * (chance / 100.f));

	pWeapon->UpdateAccuracyPenalty();

	float weapSpread = pWeapon->GetSpread(), weapInaccuracy = pWeapon->GetInaccuracy();

	for (int i = 0; i < Seeds; i++)
	{


		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * M_PI);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * M_PI);

		if (pWeapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}
		else if (pWeapon->m_iItemDefinitionIndex() == WEAPON_NEGEV && pWeapon->m_flRecoilIndex() < 3.f)
		{
			for (int i = 3; i > pWeapon->m_flRecoilIndex(); --i)
			{
				a *= a;
				c *= c;
			}

			a = 1.f - a;
			c = 1.f - c;
		}
		float Inaccuracy = a * weapInaccuracy;
		float Spread = c * weapSpread;

		Vector spreadView((cos(b) * Inaccuracy) + (cos(d) * Spread), (sin(b) * Inaccuracy) + (sin(d) * Spread), 0), direction, viewForward;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewanglesSpread;

		Math::VectorAngles(direction, up, viewanglesSpread);
		Math::FixAngles(viewanglesSpread);

		Math::AngleVectors(viewanglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = g_LocalPlayer->GetEyePos() + (direction * pWeapon->GetCSWeaponData()->flRange);

		trace_t tr;
		Ray_t ray;

		ray.Init(g_LocalPlayer->GetEyePos(), viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, pEnt, &tr);

		if (tr.hit_entity == pEnt)
			Hits++;

		if (((Hits / Seeds) * 100.f) >= chance)
			return true;

		if ((Seeds - i + Hits) < neededHits)
			return false;
	}

	return false;
	/*
	float inaccuracy = pWeapon->GetInaccuracy();
	if (inaccuracy == 0) inaccuracy = 0.0000001;
	inaccuracy = 1 / inaccuracy;
	return inaccuracy < chance;*/
}


bool CanFire()
{

	if (!g_LocalPlayer->m_hActiveWeapon())
		return false;

	if (g_LocalPlayer->m_hActiveWeapon()->IsReloading() || g_LocalPlayer->m_hActiveWeapon()->m_iClip1() <= 0)
		return false;

	auto flServerTime = g_GlobalVars->curtime;

	return g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() < flServerTime;
}

void Autoscope(CUserCmd* cmd) {
	if (g_Options.autoscope) {
		if (g_LocalPlayer->m_hActiveWeapon()->IsSniper()) {
			if (!g_LocalPlayer->m_bIsScoped()) {
				cmd->buttons |= IN_ATTACK2;
			}
		}
	} //autoscope

}

bool CanFirePostpone(float time) {

	float rdyTime = g_LocalPlayer->m_hActiveWeapon()->GetFireReadyTime();

	if (rdyTime > 0 && rdyTime < g_GlobalVars->curtime)
		return true;

	return false;
}

void Ragebot::AntiAim(CUserCmd* cmd) {

	if (g_Options.antiuntrusted)
	{

	if (!g_LocalPlayer->IsAlive())
		return;

	if (cmd->buttons & IN_USE)
		return;

	if (g_LocalPlayer->m_nMoveType() & MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() & MOVETYPE_NOCLIP)
		return;

	float flServerTime = g_GlobalVars->curtime;
	if (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE) {
		float next_secondary_attack = g_LocalPlayer->m_hActiveWeapon()->m_flNextSecondaryAttack() - flServerTime;
		float next_primary_attack = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - flServerTime;
		if ((cmd->buttons & IN_ATTACK && next_primary_attack < 0.f) || (cmd->buttons & IN_ATTACK2 && next_secondary_attack < 0.f))
			return;
	}

	if (!g_LocalPlayer->m_hActiveWeapon()->is_grenade()) {
		if (g_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex() != WEAPON_REVOLVER) {
			if (cmd->buttons & IN_ATTACK && g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() <= flServerTime) {
				return;
			}
		}
		else {
			if (cmd->buttons & IN_ATTACK2)
				return;

			if (CanFirePostpone(flServerTime) && (cmd->buttons & IN_ATTACK))
				return;
		}
	}
	else
	{
		if (g_LocalPlayer->m_hActiveWeapon()->is_being_thrown())
			return;
	}
	switch (g_Options.aa_x) // pitches
	{
	case 1:
		cmd->viewangles.pitch = 89.f; //.down pitch
		break;
	case 2:
		cmd->viewangles.pitch = -89.f; //.up pitch
		break;
	case 3:
		cmd->viewangles.pitch = 0.f; //mixed pitch
		break;
	}

	switch (g_Options.aa_y) // pitches
	{
	case 1:
		cmd->viewangles.yaw -= 180.f; //up pitch
		break;
	}
}
}

void Ragebot::Fire(CUserCmd* cmd)
{
	auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!CanFire())
		return;
	auto tTarget = this->FindTarget(cmd); // gets target for aimbot
	if (!(cmd->buttons & IN_ATTACK2) && tTarget.pEnt && tTarget.vPos.IsValid())
	{
		QAngle result; // shot angle

		result = Math::CalcAngle(g_LocalPlayer->GetEyePos(), tTarget.vPos); //calculates shot angle

		auto aimpunchangle = g_LocalPlayer->m_aimPunchAngle(); //gets aim punch for no recoil

		result -= (aimpunchangle * g_CVar->FindVar("weapon_recoil_scale")->GetFloat()); // more recoilkstuff

		bool hitchance = HitChance(tTarget.pEnt, weapon, result, g_Options.rage_hitchance); // hitchance calculation

		Autoscope(cmd);

		Autostop(cmd);

		float damage = g_AutoWall.CanHit(tTarget.vPos);

		if (hitchance) // if hitchance is true(aka can hit)
		{
			auto mindamage = std::min(tTarget.pEnt->m_iHealth(), g_Options.rage_mindmg);

			if (damage > mindamage) { // second min damage calculation, for accuracy.
				cmd->viewangles = result; // sets to result aka where it aims
				cmd->buttons |= IN_ATTACK; //shoots
				cmd->tick_count = TIME_TO_TICKS(tTarget.pEnt->m_flSimulationTime() + GetInterp()); // lag comp stuff(or helps to hit moving enemies maybe? dont know tbh.
			}

			//G::RealAngles = cmd->viewangles;
		}
	}

}

// i really hate this as a ragebot, it confuses me but whatever what can i do



void Ragebot::extrapolation() {
	for (int i = 1; i < g_GlobalVars->maxClients; i++) {
		C_BasePlayer* e = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!e
			|| !e->IsAlive()
			|| e->IsDormant()) {
			continue;
		}

		float simtime_delta = e->m_flSimulationTime() - e->m_flOldSimulationTime();
		int choked_ticks = simtime_delta;
		Vector lastOrig;

		if (lastOrig.Length() != e->m_vecOrigin().Length())
			lastOrig = e->m_vecOrigin();

		float delta_distance = (e->m_vecOrigin() - lastOrig).LengthSqr();
		if (delta_distance > 4096.f) {
			Vector velocity_per_tick = e->m_vecVelocity() * g_GlobalVars->interval_per_tick;
			auto new_origin = e->m_vecOrigin() + (velocity_per_tick * choked_ticks);
			e->SetOrigin(new_origin);
		}
	}
} //vro extrapolation

void AutoRevolver(CUserCmd* cmd)
{
	auto me = g_LocalPlayer;
	auto weapon = me->m_hActiveWeapon();

	if (!me || !me->IsAlive() || !weapon)
		return;

	if (weapon->m_iItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
	{
		static int delay = 0; /// pasted delay meme from uc so we'll stop shooting on high ping
		delay++;

		if (delay <= 15)
			cmd->buttons |= IN_ATTACK;
		else
			delay = 0;
	}
}
void Ragebot::DoRagebot(CUserCmd* cmd) { // runs ragebot stuff
	if (!g_Options.rage_enabled)
		return;

	if (!g_LocalPlayer->IsAlive())
		return;
	// checks if localentity exists, if is ingame, and is alive
	
	AutoRevolver(cmd);
	this->Fire(cmd); // shoots at it
}