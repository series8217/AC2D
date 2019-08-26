#include "stdafx.h"
#include "cModelGroup.h"
#include "cByteStream.h"
#include "AnimationIds.h"

#include "BSPTypes.h"

cModelGroup::cModelGroup()
{
	m_pTranslation = cPoint3D(0,0,0);
	for (int i=0;i<4;i++)
		m_fRotation[i] = 0;

	m_fAnimT;
	m_pfAnim = 0;
	m_fKeyData = NULL;
	m_AnimSet = NULL;
	m_DefaultAnimSet = NULL;
	m_fDefaultAnimSetSpeedScale = 1.0;
	m_AnimSetCurIndex = -1;
	m_dwDefaultAnim = 0;
	m_dwCurAnim = 0;
	m_dwEndFrame = 0;
	m_fDefaultPlaySpeed = 30.0;
}

cModelGroup::~cModelGroup()
{
	for (std::vector<cModel *>::iterator i = m_vModels.begin(); i != m_vModels.end(); i++)
		delete *i;

	delete []m_fKeyData;
}

void cModelGroup::ClearDefaultAnimations() {
	m_dwDefaultAnim = 0;
	m_DefaultAnimSet = NULL;
}

void cModelGroup::SetDefaultAnim(DWORD dwAnim, float fDefaultPlaySpeed)
{
	m_dwDefaultAnim = dwAnim;
	//if (!m_dwCurAnim)
	//	m_dwCurAnim = m_dwDefaultAnim;
	m_fDefaultPlaySpeed = fDefaultPlaySpeed;
}

void cModelGroup::SetDefaultAnim(stAnimSet* AnimSet, float fSpeedScale)
{
	m_DefaultAnimSet = AnimSet;
	m_fDefaultAnimSetSpeedScale = fSpeedScale;
	//if (!m_dwCurAnim)
	//	m_dwCurAnim = m_dwDefaultAnim;
	
}

void cModelGroup::PlayAnimation(stAnimSet* AnimSet, float PlaySpeed, bool sticky)
{
	m_AnimSet = AnimSet;
	m_AnimSetCurIndex = 0;
	m_fAnimSetSpeedScale = PlaySpeed;
	m_bAnimSetSticky = sticky;
	PlayAnimation(&m_AnimSet->vAnims[m_AnimSetCurIndex], PlaySpeed);
}

void cModelGroup::PlayAnimation(stAnimInfo* AnimInfo, float SpeedScale){
	PlayAnimation(AnimInfo->dwAnim, AnimInfo->dwStartFrame, AnimInfo->dwEndFrame, AnimInfo->fPlaySpeed * SpeedScale);
}

void cModelGroup::PlayAnimation(DWORD dwAnim, DWORD dwStartFrame, DWORD dwEndFrame, float fPlaySpeed)
{
	// XXX: TODO: get negative play speed working properly.
	m_fAnimT = dwStartFrame/fPlaySpeed;
	m_fPlaySpeed = fPlaySpeed;
	m_dwEndFrame = dwEndFrame;
	m_dwCurAnim = dwAnim;

	delete []m_fKeyData;
	m_fKeyData = NULL;

	//now, load animation data
	m_pfAnim = m_Portal->OpenEntry(Animation::BaseID | dwAnim);
	if (!m_pfAnim)
		return;

	//crazyass motherfucking kewtcode
	int				a,num_parts,num_frames,*num;
	int				count,stride,skip;
	int				anim_type;
	float			*flt;
	DWORD			*hex,*start,fil;
	BYTE			*frames;

	frames=(BYTE *)				m_pfAnim->data;
	if (!frames) return;
	
	start=(DWORD *)frames; fil=*start;
	num=(int *)&frames[4]; anim_type=*num; num++; num_parts=*num; num++; num_frames=*num;
	
	//init member data (killacode)
	m_fKeyData = new float[num_frames*num_parts*7];
	ZeroMemory(m_fKeyData, num_frames*num_parts*7*sizeof(float));
	m_iNumFrames = num_frames;
	m_iNumParts = num_parts;
	if (dwEndFrame == 0xFFFFFFFF)
		m_dwEndFrame = m_iNumFrames;

	//Index frame data
	skip=16;
	if ((anim_type==1)||(anim_type==3)) skip+=(num_frames*7*4);
	stride=num_parts*7;
	flt=(float *) &frames[skip];
	for (a=0;a<num_frames;a++)
	{
		memcpy(&m_fKeyData[a*num_parts*7], flt, 7*num_parts*sizeof(float));
		flt+=stride; hex=(DWORD *)flt;
		if (*hex==0) flt++; else
		{
			count=hex[0]; hex++;
			while (count)
			{
				if (hex[0]==0x01) hex+=3; else	//0x01,0x00,soundref
				if (hex[0]==0x02) hex+=3; else	//0x02,0x00,0x0C
				if (hex[0]==0x03) hex+=9; else	//0x03,0x00,0x14,6floats
				if (hex[0]==0x05) hex+=3; else	//0x05,0x00,0xBB401
				if (hex[0]==0x06) hex+=3; else	//0x06,0x01,0x01
				if (hex[0]==0x07) hex+=6; else	//0x07,0x00,0x0A,1.0,1.0,0x00
				if (hex[0]==0x0D) hex+=12; else	//0x0D,lotsa stuff (3 floats in there somewhere)
				if (hex[0]==0x0F) hex+=3; else	//0x0F,0x00,0x01
				if (hex[0]==0x11) hex+=2; else	//0x11,0x00
				if (hex[0]==0x13) hex+=4; else	//0x13,0x00,someREF,0x00
				if (hex[0]==0x14) hex+=5; else	//0x14,0x00,0x00,0x00,0x00
				if (hex[0]==0x15) hex+=6; else	//0x15,0x00,soundref,3floats
				if (hex[0]==0x16) hex+=5; else	//0x16,0x00,0x00,2floats
				{
//					Whine ("Unknown ANIM_EXTRA","(FILE:0x%08X):(OFS:0x%04X):(EXTRA:0x%08X)",
//						(anm),(hex-start)*4,*hex);
					MessageBeep(-1);
                    return;
				}
				count--;
			}
			flt=(float *)hex;
		}
	}
}

void cModelGroup::UpdateAnim(float fTime)
{
	if (!m_dwCurAnim) {
		if (m_dwDefaultAnim) {
			PlayAnimation(m_dwDefaultAnim, 0, 0xFFFFFFFF, m_fDefaultPlaySpeed);
		}
		else {
			return;
		}
	}

	m_fAnimT += fTime;
	DWORD iFrameNum = (int)(m_fPlaySpeed*m_fAnimT);

	if ((!m_fKeyData) || ((iFrameNum >= m_dwEndFrame) && (m_dwCurAnim != m_dwDefaultAnim)))
	{
		// currently in an animation set
		if (m_AnimSet) {
			m_AnimSetCurIndex++;
			if (m_AnimSetCurIndex < m_AnimSet->vAnims.size()){
				// play next animation
				PlayAnimation(&m_AnimSet->vAnims[m_AnimSetCurIndex], m_fAnimSetSpeedScale);
				iFrameNum = 0;
			} else {
				if (m_bAnimSetSticky){
					// repeat last animation
					m_AnimSetCurIndex = m_AnimSet->vAnims.size() - 1;
	 				PlayAnimation(&m_AnimSet->vAnims[m_AnimSetCurIndex], m_fAnimSetSpeedScale);
					//iFrameNum = 0;
				} else {
					// done with set
					m_AnimSet = NULL;
					m_AnimSetCurIndex = -1;
					m_fAnimSetSpeedScale = 1.0;
				}
			}

		}
		if (!m_AnimSet){
			if (m_DefaultAnimSet) {
				// FIXME: this may get called excessively since m_dwCurAnim != m_dwDefaultAnim when playing default set.
				PlayAnimation(m_DefaultAnimSet, m_fDefaultAnimSetSpeedScale);
				// FIXME: if only one animation in set, this will cause stuttering when playing back at high speed...
				// XXX: this also probably breaks negative play speed animations
				iFrameNum = 0;
			}
			else {
				//revert to default anim
				PlayAnimation(m_dwDefaultAnim, 0, 0xFFFFFFFF, m_fDefaultPlaySpeed);
				// XXX: this might break negative play speed animations
				iFrameNum = 0;
			}
		}

		//in case it fucks up
		if (!m_fKeyData)
			return;
	}
	iFrameNum %= m_iNumFrames;

	//Animate the nodes
//	for (a=0;a<m_iNumParts;a++)
	for (int a=0;a<(int) m_vModels.size();a++)
	{
		float *flt = &m_fKeyData[iFrameNum*m_iNumParts*7];
        // XXX: something else is probably wrong with this, because original code didn't check a < m_iNumParts and overran the array.
        if (a < m_iNumParts) {
            flt += a * 7;
            m_vModels[a]->SetTranslation(cPoint3D(flt[0], flt[1], flt[2]));
            m_vModels[a]->SetRotation(flt[3], flt[4], flt[5], flt[6]);
        }
	}
}

int cModelGroup::Draw()
{
	int tricount = 0;

	glPushMatrix();

	glTranslatef(m_pTranslation.x, m_pTranslation.y, m_pTranslation.z);
	glScalef(m_fScale, m_fScale, m_fScale);

	float s = m_fRotation[1]*m_fRotation[1] + m_fRotation[2]*m_fRotation[2] + m_fRotation[3]*m_fRotation[3];
	if (s > 0)
		glRotatef(2*acos(m_fRotation[0])*180.0f/(float)M_PI, m_fRotation[1]/s, m_fRotation[2]/s, m_fRotation[3]/s);

	for (std::vector<cModel *>::iterator smi = m_vModels.begin(); smi != m_vModels.end(); smi++)
	{
		tricount += (*smi)->Draw();
	}

	glPopMatrix();

	return tricount;
}

void cModelGroup::SetScale(float fScale)
{
	m_fScale = fScale;
}

void cModelGroup::SetTranslation(cPoint3D Translation)
{
	m_pTranslation = Translation;
}

void cModelGroup::SetRotation(float Rot1, float Rot2, float Rot3, float Rot4)
{
	m_fRotation[0] = Rot1;
	m_fRotation[1] = Rot2;
	m_fRotation[2] = Rot3;
	m_fRotation[3] = Rot4;
}

bool cModelGroup::ReadDungeon(DWORD dwDungeon, WORD wDungeonPart, std::vector<WORD> * v_Textures)
{
	cModel *mModel = new cModel();

	bool bRet = mModel->ReadDungeonPart(dwDungeon, wDungeonPart, v_Textures);
	if (!bRet)
	{
		delete mModel;
		return false;
	}

	m_vModels.push_back(mModel);

	return true;
}

bool cModelGroup::ReadModel(DWORD dwModel, std::vector<stPaletteSwap> *vPaletteSwaps, std::vector<stTextureSwap> *vTextureSwaps, std::vector<stModelSwap> *vModelSwaps)
{
	//clear out the old
	for (std::vector<cModel *>::iterator i = m_vModels.begin(); i != m_vModels.end(); i++)
		delete *i;

	m_vModels.clear();

	if (((dwModel & 0xFF000000) >> 24) == 1){
		cModel *mModel = new cModel();

		bool bRet = mModel->ReadModel(dwModel);
		if (!bRet)
		{
			delete mModel;
			return false;
		}

		m_vModels.push_back(mModel);
	}

	if (((dwModel & 0xFF000000) >> 24) == 2)
	{
		cPortalFile *pf = m_Portal->OpenEntry(dwModel);
		if (!pf)
		{
			return false;
		}


		cByteStream pBS(pf->data, pf->length);

		pBS.ReadBegin();
		DWORD dwID = pBS.ReadDWORD();
		DWORD dwType = pBS.ReadDWORD();
		DWORD dwNumObjs = pBS.ReadDWORD();
		
		//Load Basic Objects
		for (DWORD i = 0; i < dwNumObjs; i++)
		{
			cModel *mModel = new cModel();
			DWORD dwSubModel = pBS.ReadDWORD();

			if (vModelSwaps)
			{
				for (std::vector<stModelSwap>::iterator h = vModelSwaps->begin(); h != vModelSwaps->end(); h++)
				{
					if ((*h).modelIndex == i)
					{
						dwSubModel = 0x01000000 | (*h).newModel;
						break;
					}
				}
			}

			bool bRet = mModel->ReadModel(dwSubModel, i, vPaletteSwaps, vTextureSwaps);
			if (!bRet)
			{
				delete mModel;
				return false;
			}

			m_vModels.push_back(mModel);
		}

		//Figure out what the hell the rest of this stuff is later
		if (dwType & 1)
		{
			for (DWORD i = 0; i < dwNumObjs; i++)
			{
				DWORD dwUnknown = pBS.ReadDWORD();
			}
		}

		if (dwType & 2)
		{
			for (DWORD i = 0; i < dwNumObjs; i++)
			{
				float fScaleX = pBS.ReadFloat();
				float fScaleY = pBS.ReadFloat();
				float fScaleZ = pBS.ReadFloat();
			}
		}

		if (dwType & 4)
		{
		}

		if (dwType & 8)
		{
		}

		DWORD dwLT94Count = pBS.ReadDWORD();

		for (DWORD i = 0; i < dwLT94Count; i++)
		{
			DWORD dwKey = pBS.ReadDWORD();
			DWORD dwLandcell = pBS.ReadDWORD();
			float fX = pBS.ReadFloat();
			float fY = pBS.ReadFloat();
			float fZ = pBS.ReadFloat();
			float fW = pBS.ReadFloat();
			float fA = pBS.ReadFloat();
			float fB = pBS.ReadFloat();
			float fC = pBS.ReadFloat();
		}

		DWORD dwLT98Count = pBS.ReadDWORD();

		for (DWORD i = 0; i < dwLT98Count; i++)
		{
			DWORD dwKey = pBS.ReadDWORD();
			DWORD dwLandcell = pBS.ReadDWORD();
			float fX = pBS.ReadFloat();
			float fY = pBS.ReadFloat();
			float fZ = pBS.ReadFloat();
			float fW = pBS.ReadFloat();
			float fA = pBS.ReadFloat();
			float fB = pBS.ReadFloat();
			float fC = pBS.ReadFloat();
		}

		DWORD dwPT9CCount = pBS.ReadDWORD();
		if (dwPT9CCount > 1) dwPT9CCount= 1;
		for (DWORD i = 0; i < dwPT9CCount; i++)
		{
			DWORD dwKey = pBS.ReadDWORD();
			for (DWORD h = 0; h < dwNumObjs; h++)
			{
				stLocation tpld;
				tpld.Origin.x = pBS.ReadFloat();
				tpld.Origin.y = pBS.ReadFloat();
				tpld.Origin.z = pBS.ReadFloat();
				tpld.Orientation.w = pBS.ReadFloat();
				tpld.Orientation.a = pBS.ReadFloat();
				tpld.Orientation.b = pBS.ReadFloat();
				tpld.Orientation.c = pBS.ReadFloat();
//				tpld.cell_id = 0;
//				cPoint3D tp3d;
//				tp3d.CalcFromLocation(&tpld);

				m_vModels[h]->SetTranslation(cPoint3D(tpld.Origin.x, tpld.Origin.y, tpld.Origin.z));
				m_vModels[h]->SetRotation(tpld.Orientation.w, tpld.Orientation.a, tpld.Orientation.b, tpld.Orientation.c);
			}
/*			DWORD dwHookCount = pBS.ReadDWORD();
			for (DWORD h = 0; h < dwHookCount; h++)
			{
				//Unpack AnimHook
			}*/

		}
	}

	return true;
}
