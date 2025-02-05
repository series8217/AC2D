#pragma once

#include "cMessage.h"
#include "cPoint3D.h"
#include "cThread.h"
#include "cModelGroup.h"
#include "AnimationIds.h"

class cWorld;

namespace MovementType {
	enum MovementTypeEnum {
		InterpretedMotionState = 0x00,
		MoveToObject = 0x06,
		MoveToPosition = 0x07,
		TurnToObject = 0x08,
		TurnToPosition = 0x09,
	};
}

class cWObject : public cLockable {
public:
	cWObject(cWorld* world);
	~cWObject();

	void ParseMessageObjectCreate(cMessage * Message);
	void ParseMessageUpdatePosition(cMessage * Message);
	void ParseMessageMotion(cMessage * Message);
	void ParseMessageObjDesc(cMessage * Message);
	void AdjustStack(DWORD Count, DWORD Value);
	void Set229(DWORD Type, DWORD Value);
	void Set22D(DWORD Type, DWORD Value);

	void UpdatePosition(float fTimeDiff);
	void ClearDefaultAnimations();
	// by stance and ID
	void PlayAnimation(Animation::MotionCommand Motion, Animation::Stance Stance, float fSpeedScale, bool sticky=false);
	void SetDefaultAnimation(Animation::MotionCommand Motion, Animation::Stance Stance, float fSpeedScale);
	// directly by animation ID in portal.dat
	void PlayAnimation(DWORD AnimIdID, float fPlaySpeed, bool bSetDefault);
	void SetDefaultAnimation(DWORD AnimIdID, float fDefaultPlaySpeed);

	void CalcHeading();

	int Draw();

	DWORD GetGUID();
	cPoint3D GetPosition();
	stLocation * GetLocation();
	stMoveInfo GetMoveInfo();
	DWORD GetCellID();
	float GetHeading();
	std::string GetName();
	DWORD GetWielder();
	DWORD GetObjectFlags2();
	DWORD GetRadarOverride();
	Animation::Stance GetStance();
	
	void SetVelocity(cPoint3D NewVelocity);
	void SetMoveVelocities(float fFB, float fStrafe, float fTurn);

	//for user cacheing...
	std::vector<stPaletteSwap> * GetPaletteSwaps()
	{
		return &palettes;
	}
	std::vector<stTextureSwap> * GetTextureSwaps()
	{
		return &textures;
	}
	std::vector<stModelSwap> * GetModelSwaps()
	{
		return &models;
	}

	WORD GetAnimCount()
	{
		return animCount;
	}

	MotionTable MotionTable_;

private:
	void LoadMotionTable();
	void LoadLocationHeading(float fZ);

    cWorld* m_World;

	//calculated members
	float m_fHeading;
	WORD moveCount;
	cPoint3D CalcPosition;

	bool m_bMoving;
	cPoint3D Velocity;
	float fVelocityTurn, fVelocityStrafe, fVelocityFB;

	Animation::Stance m_Stance;

	bool m_bModelUpdate;

	cModelGroup *m_mgModel;

	stLocation location;

	std::vector<stPaletteSwap> palettes;
	std::vector<stTextureSwap> textures;
	std::vector<stModelSwap> models;

	//raw members
	DWORD	GUID;
	
	WORD portalMode;
	WORD unknown_1;

	WORD f74csequence;

	DWORD animConfig;
	DWORD soundset;
	DWORD unknown_blue;
	DWORD modelNumber;
	DWORD wielder;
	DWORD wieldingSlot;

	struct stEquipped {
		DWORD equipID;
		DWORD equipSlot;
	};
	std::vector<stEquipped> equipped;

	DWORD unknown_darkbrown;
	DWORD unknown_brightpurple;
	float unknown_lightgrey;
	float unknown_trio1_1, unknown_trio1_2, unknown_trio1_3;
	float unknown_trio2_1, unknown_trio2_2, unknown_trio2_3;
	float unknown_trio3_1, unknown_trio3_2, unknown_trio3_3;
	DWORD unknown_medgrey;
	float unknown_bluegrey;

	WORD numMovements;
	WORD numAnimInteract;
	WORD numBubbleMode;
	WORD numJumps;
	WORD numPortals;
	WORD animCount;
	WORD numOverride;
	WORD unknown_seagreen8;
	WORD numLogins;
//	WORD unknown_seagreen10;

	std::string objectName;
	DWORD model;
	DWORD icon;
	float m_fScale;
	DWORD category, behavior;

	std::string namePlural;
	BYTE itemSlots, packSlots;
	WORD ammunition;
	DWORD value;
	DWORD unknown_v2;
	float approachDistance;
	DWORD usableOn;
	DWORD iconHighlight;
	BYTE wieldType;
	WORD usesLeft;
	WORD totalUses;
	WORD stackCount;
	WORD stackMax;
	DWORD container;
	DWORD owner;
	DWORD coverage1, coverage2, coverage3;
    BYTE unknown5;
	BYTE unknown_v6;
	WORD unknown800000;
	float workmanship;
	WORD burden;
	WORD associatedSpell;
	DWORD houseOwnerID;
	DWORD dwellingaccess;
	WORD hookTypeUnknown;
	WORD hookType;
	DWORD hookableOn, iconOverlay, iconUnderlay;
	
	WORD unknown11a;
	WORD unknown11b;
	DWORD monarch;
	WORD icon2;
	DWORD material;
};