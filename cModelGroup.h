#pragma once

#include "cModel.h"
#include "MotionTable.h"


class cModelGroup {
public:
	cModelGroup();
	~cModelGroup();

	void SetTranslation(cPoint3D Translation);
	void SetRotation(float Rot1, float Rot2, float Rot3, float Rot4);
	void SetScale(float fScale);

	void ClearDefaultAnimations();
	void SetDefaultAnim(DWORD AnimId, float fDefaultPlaySpeed = 30.0);
	void SetDefaultAnim(MotionData* MotionData, float fSpeedScale);

	void PlayAnimation(MotionData* MotionData, float PlaySpeed, bool sticky=false);
	void PlayAnimation(AnimData* AnimInfo, float SpeedScale);
	void PlayAnimation(DWORD AnimId, DWORD LowFrame, DWORD HighFrame, float fPlaySpeed);
	void UpdateAnim(float fTime);
	
	int Draw();

	bool ReadModel(DWORD dwModel, std::vector<stPaletteSwap> *vPaletteSwaps = 0, std::vector<stTextureSwap> *vTextureSwaps = 0, std::vector<stModelSwap> *vModelSwaps = 0);
	bool ReadDungeon(DWORD dwDungeon, WORD wDungeonPart, std::vector<WORD> * v_Textures);

private:
	void MotionDataNext();

	//anims
	cPortalFile *m_pfAnim;
	float m_fAnimT;
	// current playback speed
	float m_fPlaySpeed;
	// set speed scale
	float m_fMotionDataSpeedScale;
	// repeat last animation in set when done
	bool m_bMotionDataSticky;
	float *m_fKeyData;
	DWORD m_iNumFrames, m_iNumParts;
	DWORD m_HighFrame;
	// reverse playback -- this is set internally when the speed is less than zero
	bool m_bReverse;
    
	// current animation set to play through
	MotionData* m_MotionData;
	// current index in the animation set
	unsigned int m_MotionDataCurIndex;
	// when this animation is over, go to the next animation index the set. if it's done, or no set, play next set, otherwise play default
	MotionData* m_DefaultMotionData;
	float m_fDefaultMotionDataSpeedScale;
	DWORD m_dwCurAnim;
	DWORD m_dwDefaultAnim;
	// playback speed when reverting to the default animation
	float m_fDefaultPlaySpeed;

	std::vector<cModel *> m_vModels;

	float m_fScale;

	cPoint3D m_pTranslation;
	float m_fRotation[4];

};
