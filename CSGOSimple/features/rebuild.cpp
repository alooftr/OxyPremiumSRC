#include "rebuild.h"
/*
void Studio_BuildMatrices(const studiohdr_t* pStudioHdr, const QAngle& angles, const Vector& origin,
	const Vector pos[], const Quaternion q[], int iBone, float flScale,
	matrix3x4_t bonetoworld[128], int boneMask)
{
	int i, j;

	int chain[128] = {};
	int chainlength = 0;

	if (iBone < -1 || iBone >= pStudioHdr->numbones)
		iBone = 0;

	// build list of what bones to use
	if (iBone == -1)
	{
		// all bones
		chainlength = pStudioHdr->numbones;
		for (i = 0; i < pStudioHdr->numbones; i++)
		{
			chain[chainlength - i - 1] = i;
		}
	}
	else
	{
		// only the parent bones
		i = iBone;
		while (i != -1)
		{
			chain[chainlength++] = i;
			i = pStudioHdr->boneParent(i);
		}
	}

	matrix3x4_t bonematrix;
	matrix3x4_t rotationmatrix; // model to world transformation

	QAngle q2 = angles;
	q2.roll += 0.f;
	Math::AngleMatrix(q2, origin, rotationmatrix);

	// Account for a change in scale
	if (flScale < 1.0f - FLT_EPSILON || flScale > 1.0f + FLT_EPSILON)
	{
		Vector vecOffset;
		MatrixGetColumn(rotationmatrix, 3, vecOffset);
		vecOffset -= origin;
		vecOffset *= flScale;
		vecOffset += origin;
		Math::MatrixSetColumn(vecOffset, 3, rotationmatrix);

		// Scale it uniformly
		Math::VectorScale(rotationmatrix[0], flScale, rotationmatrix[0]);
		Math::VectorScale(rotationmatrix[1], flScale, rotationmatrix[1]);
		Math::VectorScale(rotationmatrix[2], flScale, rotationmatrix[2]);
	}

	for (j = chainlength - 1; j >= 0; j--)
	{
		i = chain[j];
		if (pStudioHdr->boneFlags(i) & boneMask)
		{
			QuaternionMatrix(q[i], pos[i], bonematrix);

			const auto boneParent = pStudioHdr->boneParent(i);

			if (boneParent == -1)
			{
				ConcatTransforms(rotationmatrix, bonematrix, bonetoworld[i]);
			}
			else
			{
				ConcatTransforms(bonetoworld[boneParent], bonematrix, bonetoworld[i]);
			}
		}
	}
}*/

void GetSkeleton(C_BasePlayer* player, studiohdr_t* studioHdr, Vector* pos, Quaternion* q, int boneMask)
{
	BoneSetup boneSetup(studioHdr, boneMask, player->m_flPoseParameter().data());
	boneSetup.InitPose(pos, q);
	//
	if (!player->m_pIk())
	{
		*player->m_pIk() = *IKContext::CreateIKContext();
		player->m_pIk()->Init(studioHdr, player->m_angAbsAngles(), player->GetAbsOrigin(), g_GlobalVars->curtime, 0, 0x40000);
	}

	boneSetup.AccumulatePose(pos, q, player->m_nSequence(), player->m_flCycle(), 1.f, g_GlobalVars->curtime, player->m_pIk());

	int layers[20];
	memset(layers, 0, sizeof(layers));

	for (auto animLayerIndex = 0; animLayerIndex < player->GetNumAnimOverlays(); animLayerIndex++)
	{
		auto pLayer = player->GetAnimOverlay(animLayerIndex);
		if (pLayer->m_flWeight > 0 && pLayer->m_nOrder >= 0 && pLayer->m_nOrder < player->GetNumAnimOverlays()) {

			boneSetup.AccumulatePose(
				pos, q,
				pLayer->m_nSequence, pLayer->m_flCycle, pLayer->m_flWeight,
				g_GlobalVars->curtime, player->m_pIk());
		}
	}

	if (player->m_pIk())
	{
		IKContext auto_ik;
		auto_ik.Init(studioHdr, player->m_angAbsAngles(), player->m_vecOrigin(), g_GlobalVars->curtime, 0, boneMask);
		boneSetup.CalcAutoplaySequences(pos, q, g_GlobalVars->curtime, &auto_ik);
	}
	else {
		boneSetup.CalcAutoplaySequences(pos, q, g_GlobalVars->curtime, 0);
	}

	boneSetup.CalcBoneAdj(pos, q, player->m_flEncodedController());
}