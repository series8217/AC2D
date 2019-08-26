#pragma once

#include "cModel.h"

// XXX: for debug // XXX: delete me?
struct stAnimInfo {
	DWORD dwAnim;
	DWORD dwStartFrame;
	DWORD dwEndFrame;
	float fPlaySpeed;
};

struct stAnimSet {
	union {
		DWORD dwID; // full ID -- (Stance << 16) | Motion))
		struct {
			WORD wMotion; // Animation::Motion
			WORD wStance; // Animation::Stance
		};
	};
	DWORD dwFlags;
	std::vector<stAnimInfo> vAnims;
};


class cModelGroup {
public:
	cModelGroup();
	~cModelGroup();

	void SetTranslation(cPoint3D Translation);
	void SetRotation(float Rot1, float Rot2, float Rot3, float Rot4);
	void SetScale(float fScale);

	void ClearDefaultAnimations();
	void SetDefaultAnim(DWORD dwAnim, float fDefaultPlaySpeed = 30.0);
	void SetDefaultAnim(stAnimSet* AnimSet, float fSpeedScale);

	void PlayAnimation(stAnimSet* AnimSet, float PlaySpeed, bool sticky=false);
	void PlayAnimation(stAnimInfo* AnimInfo, float SpeedScale);
	void PlayAnimation(DWORD dwAnim, DWORD dwStartFrame, DWORD dwEndFrame, float fPlaySpeed);
	void UpdateAnim(float fTime);
	
	int Draw();

	bool ReadModel(DWORD dwModel, std::vector<stPaletteSwap> *vPaletteSwaps = 0, std::vector<stTextureSwap> *vTextureSwaps = 0, std::vector<stModelSwap> *vModelSwaps = 0);
	bool ReadDungeon(DWORD dwDungeon, WORD wDungeonPart, std::vector<WORD> * v_Textures);

private:
	//anims
	cPortalFile *m_pfAnim;
	float m_fAnimT;
	// current playback speed
	float m_fPlaySpeed;
	// set speed scale
	float m_fAnimSetSpeedScale;
	// repeat last animation in set when done
	bool m_bAnimSetSticky;
	float *m_fKeyData;
	DWORD m_iNumFrames, m_iNumParts;
	DWORD m_dwEndFrame;
    
	// current animation set to play through
	stAnimSet* m_AnimSet;
	// current index in the animation set
	unsigned int m_AnimSetCurIndex;
	// when this animation is over, go to the next animation index the set. if it's done, or no set, play next set, otherwise play default
	stAnimSet* m_DefaultAnimSet;
	float m_fDefaultAnimSetSpeedScale;
	DWORD m_dwCurAnim;
	DWORD m_dwDefaultAnim;
	// playback speed when reverting to the default animation
	float m_fDefaultPlaySpeed;

	std::vector<cModel *> m_vModels;

	float m_fScale;

	cPoint3D m_pTranslation;
	float m_fRotation[4];

};
