#pragma once

#include "stdafx.h"
#include "Vector3.h"
#include "cByteStream.h"
#include "AnimationIds.h"

class AnimData {

public:
	AnimData();
	void Unpack(cByteStream* reader);

	DWORD AnimId;
	DWORD LowFrame; // XXX signed?
	DWORD HighFrame; // XXX signed?
	float AnimFramerate;
};

class MotionData {

public:
    MotionData();
	
	void Unpack(cByteStream* reader);
	
    BYTE Bitfield;
    // 0x1 = HasVelocity, 0x2 = HasOmega
    BYTE Flags;
	// union {
	// 	DWORD dwID; // full ID -- (Stance << 16) | Motion))
	// 	struct {
	// 		WORD wMotion; // Animation::Motion
	// 		WORD wStance; // Animation::Stance
	// 	};
	// };
    Vector3 Velocity;
    Vector3 Omega;
	std::vector<AnimData> Anims;
};

class MotionTable {

public:
    MotionTable();
	void Unpack(cByteStream* reader);
	// From ACE:
	Animation::MotionCommand MotionTable::GetDefaultMotion(Animation::Stance style);
	float MotionTable::GetCycleLength(Animation::Stance stance, Animation::MotionCommand motion);
	float MotionTable::GetAnimationLength(Animation::MotionCommand motion);
	float MotionTable::GetAnimationLength(Animation::Stance stance, Animation::MotionCommand motion, Animation::MotionCommand currentMotion = Animation::MotionCommand::Invalid);
	float MotionTable::GetAnimationLength(AnimData* anim);
// TODO: implement me
// ACE.Entity.Position GetAnimationFinalPositionFromStart(ACE.Entity.Position position, float objScale, Animation::MotionCommand motion);
// ACE.Entity.Position GetAnimationFinalPositionFromStart(ACE.Entity.Position position, float objScale, Animation::MotionCommand currentMotionState, Animation::Stance style, Animation::MotionCommand motion);

	DWORD Id;
	DWORD DefaultStyle;
	std::map<DWORD, DWORD> StyleDefaults;
	std::map<DWORD, MotionData> Cycles;
	std::map<DWORD, MotionData> Modifiers;
	std::map<DWORD, std::map<DWORD, MotionData> > Links;
};