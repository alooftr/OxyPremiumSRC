#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../helpers/math.hpp"

class PoseDebugger;

class _BoneSetup {
public:
	_BoneSetup(const studiohdr_t* pStudioHdr, int boneMask, const float poseParameter[], PoseDebugger* pPoseDebugger = nullptr)
	{
		m_boneMask = boneMask;
		m_flPoseParameters = poseParameter;
		m_pStudioHdr = pStudioHdr;
		m_pPoseDebugger = pPoseDebugger;
	}

	void InitPose(Vector* pos, Quaternion* q)
	{
		static void* fn = nullptr;
		if (!fn) fn = Utils::PatternScan(GetModuleHandleA("server.dll"), "55 8B EC 83 EC 10 53 8B D9 89 55");

		auto studioHdr = m_pStudioHdr;
		auto boneMask = m_boneMask;

		__asm
		{
			pushad
			pushfd

			mov ecx, studioHdr
			mov edx, pos
			push boneMask
			push q
			call fn
			add esp, 8

			popfd
			popad
		}
	}

	void AccumulatePose(Vector pos[], Quaternion q[], int iSequence, float flCycle, float flWeight, float flTime, IKContext* pIKContext)
	{
#ifdef _DEBUG
		//Remove breakpoint when debugger is attached
		static bool onceOnly = false;
		if (!onceOnly)
		{
			onceOnly = true;
			auto pattern = Utils::PatternScan(GetModuleHandleA("server.dll"), "CC F3 0F 10 4D ? 0F 57");
			if (pattern)
			{
				DWORD oldProt;
				VirtualProtect(pattern, 1, PAGE_EXECUTE_READWRITE, &oldProt);
				*pattern = 0x90;
				VirtualProtect(pattern, 1, oldProt, &oldProt);
			}
		}
#endif

		static void* fn = nullptr;
		if (!fn) fn = (Utils::PatternScan(GetModuleHandleA("server.dll"), "B8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 56 57 8B F9 B9") - 0x6);

		__asm
		{
			pushad
			pushfd

			mov ecx, this
			push pIKContext
			push flTime
			push flWeight
			push flCycle
			push iSequence
			push q
			push pos
			call fn

			popfd
			popad
		}
	}

	void CalcAutoplaySequences(Vector pos[], Quaternion q[], float flRealTime, IKContext* pIKContext)
	{
		static void* fn = nullptr;
		if (!fn) fn = (Utils::PatternScan(GetModuleHandleA("server.dll"), "55 8B EC 83 EC 10 53 56 57 8B 7D 10"));
		__asm
		{
			mov     eax, g_GlobalVars
			mov     ecx, this
			push    0
			push    q
			push    pos
			movss   xmm3, dword ptr[eax + 10h]; a3
			call    fn
		}
	}

	void CalcBoneAdj(Vector pos[], Quaternion q[], const float* encodedControllerArray)
	{
		static void* fn = nullptr;
		if (!fn) fn = (Utils::PatternScan(GetModuleHandleA("server.dll"), "55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89"));

		auto studioHdr = m_pStudioHdr;
		auto boneMask = m_boneMask;

		__asm
		{
			pushad
			pushfd

			mov ecx, studioHdr
			mov edx, pos
			push boneMask
			push encodedControllerArray
			push q
			call fn
			add esp, 0ch

			popfd
			popad
		}

	}

	const studiohdr_t* m_pStudioHdr;
	int m_boneMask;
	const float* m_flPoseParameters;
	PoseDebugger* m_pPoseDebugger;
};


class BoneSetup
{
public:
	BoneSetup(const studiohdr_t* pStudioHdr, int boneMask, const float* poseParameter)
	{
		m_pBoneSetup = new _BoneSetup(pStudioHdr, boneMask, poseParameter);
	}
	~BoneSetup()
	{
		if (m_pBoneSetup)
			delete m_pBoneSetup;
	}
	void InitPose(Vector* pos, Quaternion* q)
	{
		m_pBoneSetup->InitPose(pos, q);
	}
	void AccumulatePose(Vector pos[], Quaternion q[], int sequence, float cycle, float flWeight, float flTime, IKContext* pIKContext)
	{
		m_pBoneSetup->AccumulatePose(pos, q, sequence, cycle, flWeight, flTime, pIKContext);
	}
	void CalcAutoplaySequences(Vector pos[], Quaternion q[], float flRealTime, IKContext* pIKContext)
	{
		m_pBoneSetup->CalcAutoplaySequences(pos, q, flRealTime, pIKContext);
	}
	void CalcBoneAdj(Vector pos[], Quaternion q[], const float* encodedControllerArray)
	{
		m_pBoneSetup->CalcBoneAdj(pos, q, encodedControllerArray);
	}
private:
	_BoneSetup* m_pBoneSetup;
};