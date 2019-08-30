#include "stdafx.h"
#include "MotionTable.h"
#include "AnimationIds.h"

AnimData::AnimData() {
	AnimId = 0;
	LowFrame = 0;
	HighFrame = 0;
	AnimFramerate = 1.0;
}

void AnimData::Unpack(cByteStream* reader) {
	AnimId = reader->ReadDWORD();
	LowFrame = reader->ReadDWORD();
	HighFrame = reader->ReadDWORD();
	AnimFramerate = reader->ReadFloat();
	if (AnimFramerate == 0) {
		// TODO: modify this in playback instead
		AnimFramerate = 30.0f;
	}
}

MotionData::MotionData() {
    Bitfield = 0;
    Flags = 0;
}

void MotionData::Unpack(cByteStream* reader){
    BYTE numAnims = reader->ReadByte();
    Bitfield = reader->ReadByte();
    Flags = reader->ReadByte();
    reader->ReadAlign();

    for (unsigned int j=0; j<numAnims; j++){
        AnimData thisAnim;
		thisAnim.Unpack(reader);
        Anims.push_back(thisAnim);
    }

    if ((Flags & 0x1) != 0) {
        float x = reader->ReadFloat();
        float y = reader->ReadFloat();
        float z = reader->ReadFloat();
        Velocity = Vector3(x,y,z);
    }

    if ((Flags & 0x2) != 0) {
        float x = reader->ReadFloat();
        float y = reader->ReadFloat();
        float z = reader->ReadFloat();
        Omega = Vector3(x,y,z);
    }
}

MotionTable::MotionTable(){
    Id = 0;
    DefaultStyle = 0;
}


void MotionTable::Unpack(cByteStream* reader){
    	//portal ID
	Id = reader->ReadDWORD();

	DefaultStyle = reader->ReadDWORD();

	DWORD numStyleDefaults = reader->ReadDWORD();
	for (DWORD i=0; i< numStyleDefaults; i++)
	{
		DWORD key = reader->ReadDWORD();
		DWORD val = reader->ReadDWORD();
        StyleDefaults[key] = val;
	}

	DWORD numCycles = reader->ReadDWORD();
	for (unsigned int i=0; i<numCycles; i++)
	{
        DWORD cycleID = reader->ReadDWORD();
        MotionData md = MotionData();
        md.Unpack(reader);
        Cycles[cycleID] = md;
	}

	DWORD numModifiers = reader->ReadDWORD();
	for (unsigned int  i=0; i<numModifiers; i++)
	{
		DWORD modifierID = reader->ReadDWORD();
        MotionData md = MotionData();
        md.Unpack(reader);
        Modifiers[modifierID] = md;
	}

	DWORD numLinks = reader->ReadDWORD();
	for (unsigned int i=0; i<numLinks; i++)
	{
		DWORD thisKey = reader->ReadDWORD();
        std::map<DWORD, MotionData> thisMap;
		DWORD thisCount = reader->ReadDWORD();
		for (unsigned int h=0; h<thisCount; h++)
		{
            DWORD motionID = reader->ReadDWORD();
            MotionData md = MotionData();
            md.Unpack(reader);
            Modifiers[motionID] = md;
		}
        Links[thisKey] = thisMap;
	}
}


// From ACE:

/// <summary>
/// Gets the default style for the requested Animation::Stance
/// </summary>
/// <returns>The default style or Animation::MotionCommand.Invalid if not found</returns>
Animation::MotionCommand MotionTable::GetDefaultMotion(Animation::Stance style)
{
    DWORD dwStyle = static_cast<DWORD>(style);
    auto it = StyleDefaults.find(dwStyle);
    if (it != StyleDefaults.end()){
        return Animation::MotionCommandFromID(StyleDefaults[dwStyle]);
    }

    return Animation::MotionCommand::Invalid;
}

float MotionTable::GetAnimationLength(Animation::MotionCommand motion)
{
    Animation::Stance defaultStance = Animation::StanceFromID(DefaultStyle);
    Animation::MotionCommand defaultMotion = GetDefaultMotion(defaultStance);

    return GetAnimationLength(defaultStance, motion, defaultMotion);
}

float MotionTable::GetCycleLength(Animation::Stance stance, Animation::MotionCommand motion)
{
    DWORD key = (DWORD)stance << 16 | (DWORD)motion & 0xFFFFF;

    auto it = Cycles.find(key);
    if (it == Cycles.end()){
        return 0.0f;
    }

    MotionData* motionData = &(it->second);
    float length = 0.0f;
    for (unsigned int i=0; i < motionData->Anims.size(); i++)
        length += GetAnimationLength(&motionData->Anims[i]);

    return length;
}

float MotionTable::GetAnimationLength(Animation::Stance stance, Animation::MotionCommand motion, Animation::MotionCommand currentMotion)
{
    if (currentMotion == Animation::MotionCommand::Invalid)
        currentMotion = GetDefaultMotion(stance);
    DWORD motionHash = static_cast<DWORD>(stance) << 16 | static_cast<DWORD>(currentMotion) & 0xFFFFF;

    std::map<DWORD, MotionData> *link = NULL;
    auto itMap = Links.find(motionHash);
    if (itMap != Links.end()){
        link = &(itMap->second);
    }
    if (link == NULL) return 0.0f;

    MotionData *motionData = NULL;
    auto itMotionData = link->find(static_cast<DWORD>(motion));
    if (itMotionData != link->end()){
        motionData = &(itMotionData->second);
    }
    if (motionData == NULL)
    {
        // try again with current motion removed from motion hash
        motionHash = (DWORD)stance << 16;
        itMap = Links.find(motionHash);
        if (itMap != Links.end()){
            link = &(itMap->second);
        }
        if (link == NULL) return 0.0f;
        itMotionData = link->find(static_cast<DWORD>(motion));
        if (itMotionData != link->end()){
            motionData = &(itMotionData->second);
        }
        if (motionData == NULL){
            return 0.0f;
        }
    }

    float length = 0.0f;
    for (unsigned int i=0; i<motionData->Anims.size(); i++){
        AnimData* anim = &motionData->Anims[i];
        length += GetAnimationLength(anim);
    }

    return length;
}

float MotionTable::GetAnimationLength(AnimData* anim)
{
    unsigned int highFrame = anim->HighFrame;
    if (anim->HighFrame == -1)
    {
        // get the actual high frame from the animation length
        // XXX: implement me
        //var animation = DatManager.PortalDat.ReadFromDat<Animation>(anim->AnimId);
        //highFrame = (int)animation->NumFrames;
    }

    unsigned int numFrames = highFrame - anim->LowFrame;

    return numFrames / std::abs(anim->AnimFramerate); // framerates can be negative, which tells the client to play in reverse
}

// TODO: implement me
// ACE.Entity.Position GetAnimationFinalPositionFromStart(ACE.Entity.Position position, float objScale, Animation::MotionCommand motion)
// {
//     Animation::Stance defaultStyle = (Animation::Stance)DefaultStyle;

//     // get the default motion for the default
//     Animation::MotionCommand defaultMotion = GetDefaultMotion(defaultStyle);
//     return GetAnimationFinalPositionFromStart(position, objScale, defaultMotion, defaultStyle, motion);
// }

// ACE.Entity.Position GetAnimationFinalPositionFromStart(ACE.Entity.Position position, float objScale, Animation::MotionCommand currentMotionState, Animation::Stance style, Animation::MotionCommand motion)
// {
//     float length = 0; // init our length var...will return as 0 if not found

//     ACE.Entity.Position finalPosition = new ACE.Entity.Position();

//     DWORD motionHash = ((DWORD)currentMotionState & 0xFFFFFF) | ((DWORD)style << 16);

//     if (Links.ContainsKey(motionHash))
//     {
//         Dictionary<DWORD, MotionData> links = Links[motionHash];

//         if (links.ContainsKey((DWORD)motion))
//         {
//             // loop through all that animations to get our total count
//             for (int i = 0; i < links[(DWORD)motion].Anims.Count; i++)
//             {
//                 AnimData anim = links[(DWORD)motion].Anims[i];

//                 DWORD numFrames;

//                 // check if the animation is set to play the whole thing, in which case we need to get the numbers of frames in the raw animation
//                 if ((anim.LowFrame == 0) && (anim.HighFrame == -1))
//                 {
//                     var animation = DatManager.PortalDat.ReadFromDat<Animation>(anim.AnimId);
//                     numFrames = animation.NumFrames;

//                     if (animation.PosFrames.Count > 0)
//                     {
//                         finalPosition = position;
//                         var origin = new Vector3(position.PositionX, position.PositionY, position.PositionZ);
//                         var orientation = new Quaternion(position.RotationX, position.RotationY, position.RotationZ, position.RotationW);
//                         foreach (var posFrame in animation.PosFrames)
//                         {
//                             origin += Vector3.Transform(posFrame.Origin, orientation) * objScale;

//                             orientation *= posFrame.Orientation;
//                             orientation = Quaternion.Normalize(orientation);
//                         }

//                         finalPosition.PositionX = origin.X;
//                         finalPosition.PositionY = origin.Y;
//                         finalPosition.PositionZ = origin.Z;

//                         finalPosition.RotationW = orientation.W;
//                         finalPosition.RotationX = orientation.X;
//                         finalPosition.RotationY = orientation.Y;
//                         finalPosition.RotationZ = orientation.Z;
//                     }
//                     else
//                         return position;
//                 }
//                 else
//                     numFrames = (DWORD)(anim.HighFrame - anim.LowFrame);

//                 length += numFrames / Math.Abs(anim.Framerate); // Framerates can be negative, which tells the client to play in reverse
//             }
//         }
//     }

//     return finalPosition;
// }