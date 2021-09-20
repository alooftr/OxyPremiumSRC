#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../options.hpp"
#include "autowall.hpp"
#include "../helpers/math.hpp"
struct PointInfo
{
public:
	PointInfo(float damage = 0.f, Vector point = Vector(0.f, 0.f, 0.f))
	{
		this->damage = damage;
		this->point = point;
	}

	float damage;
	Vector point;
};

struct Target_t
{
public:
	Target_t(C_BasePlayer* pEnt = nullptr, Vector vPos = Vector(0.f, 0.f, 0.f))
	{
		this->pEnt = pEnt;
		this->vPos = vPos;
	}

	C_BasePlayer* pEnt = nullptr;
	Vector vPos = { 0.f, 0.f, 0.f };
};


class Ragebot
{
public:
	void extrapolation();
	void DoRagebot(CUserCmd* cmd);
	float GetInterp();
	void AntiAim(CUserCmd* cmd);
private:
	bool HitChance(C_BasePlayer* pEnt, C_BaseCombatWeapon* pWeapon, QAngle Angle, float chance);
	Target_t FindTarget(CUserCmd* cmd);
	void Fire(CUserCmd* cmd);
	std::vector<int> GetHitboxes(C_BasePlayer* pl, matrix3x4_t* mat);
	PointInfo HitScan(C_BasePlayer* pBaseEntity);
};


extern Ragebot g_Ragebot;

