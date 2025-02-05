#include "stdafx.h"
#include "cWObject.h"
#include "cWorld.h"
#include "Landblocks.h"
#include "cInterface.h"

#include <math.h>


cWObject::cWObject(cWorld* world)
{
    m_World = world;

	//init all values to zero...
	GUID = 0;
	wielder = 0;
	m_Stance = Animation::Stance::NonCombat;
	location.Origin.x = 0;
	location.Origin.y = 0;
	location.Origin.z = 0;
	location.Orientation.w = 0;
	location.Orientation.a = 0;
	location.Orientation.b = 0;
	location.Orientation.c = 0;
	location.cell_id = 0;
	objectName = "";

	animCount = 0;

	unknown5 = 0;
	category = 0;

	m_fScale = 1.0f;

	moveCount = 0;
	m_bMoving = false;
	Velocity = cPoint3D(0,0,0);
	fVelocityTurn = 0;
	fVelocityStrafe = 0;
	fVelocityFB = 0;
	CalcPosition = cPoint3D(0,0,0);
	//m_fHeading = 0;

	m_bModelUpdate = false;
	m_mgModel = 0;
}

cWObject::~cWObject()
{
	delete m_mgModel;
}

int cWObject::Draw()
{
	if (m_mgModel)
		return m_mgModel->Draw();
	else
		return 0;
}

void cWObject::SetMoveVelocities(float fFB, float fStrafe, float fTurn)
{
    // forward is positive
	fVelocityFB = fFB;
    // right is positive
	fVelocityStrafe = fStrafe;
    // right is positive
	fVelocityTurn = fTurn;
	m_bMoving = ((fFB != 0) || (fStrafe != 0) || (fTurn != 0));
}

void cWObject::UpdatePosition(float fTimeDiff)
{
	Lock();
	if (m_bMoving)
	{
		CalcPosition += Velocity*fTimeDiff;

		//Turning...
 		float fCurHeading = GetHeading();
		fCurHeading -= (float) (fVelocityTurn*(M_PI/2)*fTimeDiff);
		if (fCurHeading > 2*M_PI)
			fCurHeading -= (float) (2*M_PI);
		if (fCurHeading < 0)
			fCurHeading += (float) (2*M_PI);
        // load heading back into location
		LoadLocationHeading(fCurHeading);

		//Forwards/Backwards
		cPoint3D tpstrafe(0,1,0);
		tpstrafe.RotateAround(cPoint3D(0,0,0), cPoint3D(0,0,fCurHeading));
        // position (3d model)
		CalcPosition += tpstrafe*(fVelocityFB*fTimeDiff*MODEL_SCALE_FACTOR);
        // location (server space)
        location.Origin.x += -sin(fCurHeading)*fVelocityFB*fTimeDiff;
        location.Origin.y += cos(fCurHeading)*fVelocityFB*fTimeDiff;

		//Strafing
		tpstrafe.RotateAround(cPoint3D(0,0,0), cPoint3D(0,0,(float) -M_PI/2));
        // position (3d model)
		CalcPosition += tpstrafe*(fVelocityStrafe*fTimeDiff*MODEL_SCALE_FACTOR);
        // location (server space)
        location.Origin.x -= -sin(fCurHeading + 0.5*M_PI)*fVelocityStrafe*fTimeDiff;
        location.Origin.y += -cos(fCurHeading + 0.5*M_PI)*fVelocityStrafe*fTimeDiff;

        // correct the Z position to maintain contact with the ground plane
        // get the current landblock
        Lock();
        Physics::Common::cLandblock* landblock = m_World->GetLandblock(this->GetCellID());
		assert(landblock);
		// set the z position to the landblock terrain height
		float new_z = landblock->GetZ(location.Origin);
		location.Origin.z = new_z;
		CalcPosition.z = new_z* MODEL_SCALE_FACTOR;
		// TODO: inside buildings, below ground, jumping, etc.
        Unlock();

        //XXX: stLocation *lPlayer = woMyself->GetLocation();
        //XXX: stMoveInfo mPlayer = woMyself->GetMoveInfo();
        //XXX: float fPlayerHeading = woMyself->GetHeading();
        //XXX: lPlayer->Origin.x -= -sin(fPlayerHeading);
        //XXX: lPlayer->Origin.y += -cos(fPlayerHeading);

		CalcHeading();
	}
	if (m_bModelUpdate)
	{
		bool FirstSet = false;
		if (!m_mgModel)
		{
			FirstSet = true;
			m_mgModel = new cModelGroup();
		}
		m_mgModel->ReadModel(modelNumber, &palettes, &textures, &models);
		m_mgModel->SetScale(MODEL_SCALE_FACTOR * m_fScale);

		if (FirstSet)
		{
			if (MotionTable_.Cycles.find(0x003D0003) != MotionTable_.Cycles.end())
			{
				MotionData asTemp = MotionTable_.Cycles[0x003D0003];	//idle anim?
				m_mgModel->SetDefaultAnim(asTemp.Anims[0].AnimId);
			}
		}

		m_bModelUpdate = false;
	}
	m_mgModel->SetTranslation(CalcPosition);
	m_mgModel->SetRotation(location.Orientation.w, location.Orientation.a, location.Orientation.b, location.Orientation.c);
	m_mgModel->UpdateAnim(fTimeDiff);
	Unlock();
}


void cWObject::ClearDefaultAnimations() {
	m_mgModel->ClearDefaultAnimations();
}

// Play animation using stance and ID
void cWObject::PlayAnimation(Animation::MotionCommand Motion, Animation::Stance Stance, float fSpeedScale, bool sticky)
{
	//fix this to cache...
	if (!m_mgModel)
		return;

	DWORD stance = static_cast<DWORD>(Stance);
	DWORD anim = static_cast<DWORD>(Motion);

	DWORD motionLookup = (stance << 16) | (anim & 0xFFFFFF);

	if (MotionTable_.Cycles.find(motionLookup) != MotionTable_.Cycles.end())
	{
		MotionData *asTemp = &MotionTable_.Cycles[motionLookup];
		m_mgModel->PlayAnimation(asTemp, fSpeedScale, sticky);
	}
}

// Set default animation using stance and ID
void cWObject::SetDefaultAnimation(Animation::MotionCommand Motion, Animation::Stance Stance, float fSpeedScale)
{
	//fix this to cache...
	if (!m_mgModel)
		return;

	DWORD stance = static_cast<DWORD>(Stance);
	DWORD anim = static_cast<DWORD>(Motion);

	DWORD motionLookup = (stance << 16) | (anim & 0xFFFFFF);

	if (MotionTable_.Cycles.find(motionLookup) != MotionTable_.Cycles.end())
	{
		MotionData *asTemp = &MotionTable_.Cycles[motionLookup];
		m_mgModel->SetDefaultAnim(asTemp, fSpeedScale);
	}
}

// Play animation directly by ID in portal dat file
void cWObject::PlayAnimation(DWORD AnimIdID, float fPlaySpeed, bool bSetDefault)
{
	//fix this to cache...
	if (!m_mgModel)
		return;

	if (bSetDefault) {
		m_mgModel->SetDefaultAnim(AnimIdID, fPlaySpeed);
	}
	m_mgModel->PlayAnimation(AnimIdID, 0, 0xFFFFFFFF, fPlaySpeed);
}

// Set default animation directly by ID in portal dat file
void cWObject::SetDefaultAnimation(DWORD AnimIdID, float fDefaultPlaySpeed)
{
	//fix this to cache...
	if (!m_mgModel)
		return;

	m_mgModel->SetDefaultAnim(AnimIdID, fDefaultPlaySpeed);
}

void cWObject::ParseMessageMotion(cMessage * Message)
{
    /* 0xF74C event */
	Lock();
	numLogins = Message->ReadWORD();
	// movement sequence value for this object
	WORD sequence = Message->ReadWORD();
	// server control sequence value for the object
	animCount = Message->ReadWORD();
	// 0x0 - server controlled, 0x1 - autonomous
	WORD autonomous = Message->ReadWORD();

	BYTE movementType = Message->ReadByte();
	// options (sticky, standing long jump)
	BYTE optionFlags = Message->ReadByte();
	// current stance
	m_Stance = Animation::StanceFromID(Message->ReadWORD());

	switch (movementType)
	{
	case MovementType::InterpretedMotionState:
		{
			//general animation
			DWORD flags = Message->ReadDWORD();

			Animation::MotionCommand Motion = Animation::MotionCommand::Ready;
			bool bSetDefault = true;
			float fSpeed = 1.0f;
			m_bMoving = false;

			if (flags & 0x1)
			{
				// stance mode
				WORD stance2 = Message->ReadWORD();
				if (m_Stance != Animation::StanceFromID(stance2))
					int a = 4;
			}
			if (flags & 0x2)
			{
				// movement command
				//hold animation until stopped
				//holding out hand does this
				Motion = Animation::MotionCommandFromID(Message->ReadWORD());
			}
			if (flags & 0x8)
			{
				// sidestep command
				Motion = Animation::MotionCommandFromID(Message->ReadWORD());
				m_bMoving = true;
			}
			else
				fVelocityStrafe = 0;

			if (flags & 0x20)
			{
				// turn command
				Motion = Animation::MotionCommandFromID(Message->ReadWORD());
				m_bMoving = true;
			}
			else
				fVelocityTurn = 0;

			if (flags & 0x4)
			{
				// movement speed (forward/backwards)
                fVelocityFB = Message->ReadFloat();
				fSpeed = fabs(fVelocityFB);
                // XXX: where does this scaling factor come from?
				fVelocityFB *= 3;
				m_bMoving = true;
			}
			else
				fVelocityFB = 0;

			if (flags & 0x10)
			{
				// sidestep speed
				fVelocityStrafe = Message->ReadFloat();
				fSpeed = fabs(fVelocityStrafe);
				m_bMoving = true;
			}
			if (flags & 0x40)
			{
				// turn speed
				fVelocityTurn = Message->ReadFloat();
				fSpeed = fabs(fVelocityTurn);
				m_bMoving = true;
			}
			if (flags & 0x80)
			{
				//anim sequence?
				Motion = Animation::MotionCommandFromID(Message->ReadWORD());
				WORD animSequence = Message->ReadWORD();
				fSpeed = Message->ReadFloat();
				bSetDefault = false;
			}

			if (!m_bMoving)
			{
                // XXX: ???
//				fVelocityStrafe = 0;
//				fVelocityTurn = 0;
//				fVelocityFB = 0;
			}

			PlayAnimation(Motion, m_Stance, fSpeed, bSetDefault);

			break;
		}
	case MovementType::MoveToObject:
		{
			// ID and location of target being moved to
			DWORD MoveToObjectID = Message->ReadDWORD();
			DWORD MoveToCellID = Message->ReadDWORD();
			Origin MoveToOrigin;
			memcpy(&MoveToOrigin, Message->ReadGroup(sizeof(Origin)), sizeof(Origin));
			stLocation targetLocation;
			targetLocation.cell_id = MoveToCellID;
			targetLocation.Origin = MoveToOrigin;
			// Move to movement parameters
			unsigned int bitmember = Message->ReadWORD();
			// distance to given location
			float distanceToObject = Message->ReadFloat();
			// min distance required for the movement
			float minDistance = Message->ReadFloat();
			// distance at which movement will fail
			float failDistance = Message->ReadFloat();
			// speed of animation
			float animation_speed = Message->ReadFloat();
			// distance from the location that determines if player walks or runs to it
			float walkRunThreshold = Message->ReadFloat();
			// heading the object is turning to
			float desiredHeading = Message->ReadFloat();
			cPoint3D GoTo;
			GoTo.CalcFromLocation(&targetLocation);
			Velocity = (GoTo - CalcPosition);
			Velocity.Normalize();
			Velocity *= animation_speed*MODEL_SCALE_FACTOR;
			m_bMoving = true;

			break;
		}
	case MovementType::MoveToPosition:
		{
			// location in the world to move to
			DWORD MoveToCellID = Message->ReadDWORD();
			Origin MoveToOrigin;
			memcpy(&MoveToOrigin, Message->ReadGroup(sizeof(Origin)), sizeof(Origin));
			stLocation targetLocation;
			targetLocation.cell_id = MoveToCellID;
			targetLocation.Origin = MoveToOrigin;
			// Move to movement parameters
			unsigned int bitmember = Message->ReadWORD();
			// distance to given location
			float distanceToObject = Message->ReadFloat();
			// min distance required for the movement
			float minDistance = Message->ReadFloat();
			// distance at which movement will fail
			float failDistance = Message->ReadFloat();
			// speed of animation
			float animation_speed = Message->ReadFloat();
			// distance from the location that determines if player walks or runs to it
			float walkRunThreshold = Message->ReadFloat();
			// heading the object is turning to
			float desiredHeading = Message->ReadFloat();
			cPoint3D GoTo;
			GoTo.CalcFromLocation(&targetLocation);
			Velocity = (GoTo - CalcPosition);
			Velocity.Normalize();
			Velocity *= animation_speed*MODEL_SCALE_FACTOR;
			m_bMoving = true;

			break;
		}
	case MovementType::TurnToObject:
		{
			// ID of target being faced
			DWORD TurnToObjectID = Message->ReadDWORD();
			// heading of the target to turn to (used instead of desired heading below)
			float desiredHeading = Message->ReadFloat();
			// Move to movement parameters
			unsigned int bitmember = Message->ReadWORD();
			// speed of animation
			float animation_speed = Message->ReadFloat();
			// heading the object is turning to. not used.
			float desiredHeadingUnused = Message->ReadFloat();

			// TODO: XXX: set up turn movement

			break;
		}
	case MovementType::TurnToPosition:
		{
			// Move to movement parameters
			unsigned int bitmember = Message->ReadWORD();
			// speed of animation
			float animation_speed = Message->ReadFloat();
			// heading to turn to
			float desiredHeading = Message->ReadFloat();

			// TODO: XXX: set up turn movement
			break;
		}
	default:
		{
			int a = 4;
			break;
		}
	};

	//TODO: more stuff after this... check protocol and figure it all out...
	Message->ReadAlign();
	Unlock();
}

void cWObject::ParseMessageObjDesc(cMessage * Message)
{
    // 0xF625 event
	Lock();
	Message->ReadByte();	//eleven
	int paletteCount = Message->ReadByte();
	int textureCount = Message->ReadByte();
	int modelCount = Message->ReadByte();
	
	palettes.clear();
	textures.clear();
	models.clear();

	if (paletteCount)
	{
		DWORD palflags = Message->ReadPackedDWORD();

		for (int i=0;i<paletteCount;i++)
		{
			stPaletteSwap tpPal;
			tpPal.newPalette = Message->ReadPackedDWORD();
			tpPal.offset = Message->ReadByte();
			tpPal.length = Message->ReadByte();
			palettes.push_back(tpPal);
		}
	}

	for (int i=0;i<textureCount;i++)
	{
		stTextureSwap tpTex;
		tpTex.modelIndex = Message->ReadByte();
		tpTex.oldTexture = Message->ReadPackedDWORD();
		tpTex.newTexture = Message->ReadPackedDWORD();
		textures.push_back(tpTex);
	}

	for (int i=0;i<modelCount;i++)
	{
		stModelSwap tpMod;
		tpMod.modelIndex = Message->ReadByte();
		tpMod.newModel = Message->ReadPackedDWORD();
		models.push_back(tpMod);
	}

	Message->ReadAlign();

	m_bModelUpdate = true;
	Unlock();
}

void cWObject::ParseMessageObjectCreate(cMessage * Message)
{
    /* 0xF745 event */
	Lock();
	GUID = Message->ReadDWORD();

	//mmm, code reuse
	ParseMessageObjDesc(Message);

	DWORD flags = Message->ReadDWORD();
	portalMode = Message->ReadWORD();
	unknown_1 = Message->ReadWORD();

	//flags mask..
	if (flags & 0x00010000)
	{
		DWORD unknownCount = Message->ReadDWORD();
		for (int i=0;i<(int) unknownCount;i++)
		{
			BYTE unknownByte = Message->ReadByte();
		}
		DWORD unknownDword = Message->ReadDWORD();
	}
	if (flags & 0x00020000)
	{
		DWORD unknown = Message->ReadDWORD();
	}
	if (flags & 0x00008000)
	{
		memcpy(&location, Message->ReadGroup(sizeof(stLocation)), sizeof(stLocation));
	}
	if (flags & 0x00000002)
	{
		animConfig = Message->ReadDWORD();
		
		LoadMotionTable();
	}
	if (flags & 0x00000800)
	{
		soundset = Message->ReadDWORD();
	}
	if (flags & 0x00001000)
	{
		unknown_blue = Message->ReadDWORD();
	}
	if (flags & 0x00000001)
	{
		modelNumber = Message->ReadDWORD();
	}
	if (flags & 0x00000020)
	{
		wielder = Message->ReadDWORD();
		wieldingSlot = Message->ReadDWORD();
	}
	if (flags & 0x00000040)
	{
		int equipCount = Message->ReadDWORD();
		for (int i=0;i<equipCount;i++)
		{
			stEquipped tpEquip;
			tpEquip.equipID = Message->ReadDWORD();
			tpEquip.equipSlot = Message->ReadDWORD();
			equipped.push_back(tpEquip);
		}
	}
	if (flags & 0x00000080)
	{
		m_fScale = Message->ReadFloat();
	}
	if (flags & 0x00000100)
	{
		unknown_darkbrown = Message->ReadDWORD();
	}
	if (flags & 0x00000200)
	{
		unknown_brightpurple = Message->ReadDWORD();
	}
	if (flags & 0x00040000)
	{
		unknown_lightgrey = Message->ReadFloat();
	}
	if (flags & 0x00000004)
	{
		unknown_trio1_1 = Message->ReadFloat();
		unknown_trio1_2 = Message->ReadFloat();
		unknown_trio1_3 = Message->ReadFloat();
	}
	if (flags & 0x00000008)
	{
		unknown_trio2_1 = Message->ReadFloat();
		unknown_trio2_2 = Message->ReadFloat();
		unknown_trio2_3 = Message->ReadFloat();
	}
	if (flags & 0x00000010)
	{
		unknown_trio3_1 = Message->ReadFloat();
		unknown_trio3_2 = Message->ReadFloat();
		unknown_trio3_3 = Message->ReadFloat();
	}
	if (flags & 0x00002000)
	{
		unknown_medgrey = Message->ReadDWORD();
	}
	if (flags & 0x00004000)
	{
		unknown_bluegrey = Message->ReadFloat();
	}
	//end of flags section

	//balh?
	if ((flags & 0x4004) == 0x4004)
	{
		Velocity = cPoint3D(unknown_trio1_1, unknown_trio1_2, unknown_trio1_3);
		Velocity *= MODEL_SCALE_FACTOR;
//		Velocity *= unknown_bluegrey*MODEL_SCALE_FACTOR;	//unknown_bluegrey's connection hasn't been explored yet
		m_bMoving = true;
	}
	//blah!

	//one of these might not exist...
	numMovements = Message->ReadWORD();
	numAnimInteract = Message->ReadWORD();
	numBubbleMode = Message->ReadWORD();
	numJumps = Message->ReadWORD();
	numPortals = Message->ReadWORD();
	animCount = Message->ReadWORD();
	numOverride = Message->ReadWORD();
	unknown_seagreen8 = Message->ReadWORD();
	numLogins = Message->ReadWORD();
//	unknown_seagreen10 = Message->ReadWORD();
	Message->ReadAlign();

	//Game data
	DWORD flags1 = Message->ReadDWORD();
	
	char *tpString = Message->ReadString();
	objectName = tpString;
	delete []tpString;

	model = Message->ReadPackedDWORD();
	icon = Message->ReadPackedDWORD();

	category = Message->ReadDWORD();
	behavior = Message->ReadDWORD();
	Message->ReadAlign();

	DWORD flags2 = 0;
	if (behavior & 0x04000000)
		flags2 = Message->ReadDWORD();

	//flags1 masks...
	if (flags1 & 0x00000001)
	{
		namePlural = Message->ReadString();
	}
	if (flags1 & 0x00000002)
	{
		itemSlots = Message->ReadByte();
	}
	if (flags1 & 0x00000004)
	{
		packSlots = Message->ReadByte();
	}
	if (flags1 & 0x00000100)
	{
		ammunition = Message->ReadWORD();
	}
	if (flags1 & 0x00000008)
	{
		value = Message->ReadDWORD();
	}
	if (flags1 & 0x00000010)
	{
		unknown_v2 = Message->ReadDWORD();
	}
	if (flags1 & 0x00000020)
	{
		approachDistance = Message->ReadFloat();
	}
	if (flags1 & 0x00080000)
	{
		usableOn = Message->ReadDWORD();
	}
	if (flags1 & 0x00000080)
	{
		iconHighlight = Message->ReadDWORD();
	}
	if (flags1 & 0x00000200)
	{
		wieldType = Message->ReadByte();
	}
	if (flags1 & 0x00000400)
	{
		usesLeft = Message->ReadWORD();
	}
	if (flags1 & 0x00000800)
	{
		totalUses = Message->ReadWORD();
	}
	if (flags1 & 0x00001000)
	{
		stackCount = Message->ReadWORD();
	}
	if (flags1 & 0x00002000)
	{
		stackMax = Message->ReadWORD();
	}
	if (flags1 & 0x00004000)
	{
		container = Message->ReadDWORD();
	}
	if (flags1 & 0x00008000)
	{
		owner = Message->ReadDWORD();
	}
	if (flags1 & 0x00010000)
	{
		coverage1 = Message->ReadDWORD();
	}
	if (flags1 & 0x00020000)
	{
		coverage2 = Message->ReadDWORD();
	}
	if (flags1 & 0x00040000)
	{
		coverage3 = Message->ReadDWORD();
	}
	if (flags1 & 0x00100000)
	{
		unknown5 = Message->ReadByte();
	}
	if (flags1 & 0x00800000)
	{
		unknown_v6 = Message->ReadByte();
	}
	if (flags1 & 0x08000000)
	{
		unknown800000 = Message->ReadWORD();
	}
	if (flags1 & 0x01000000)
	{
		workmanship = Message->ReadFloat();
	}
	if (flags1 & 0x00200000)
	{
		burden = Message->ReadWORD();
	}
	if (flags1 & 0x00400000)
	{
		associatedSpell = Message->ReadWORD();
	}
	if (flags1 & 0x02000000)
	{
		houseOwnerID = Message->ReadDWORD();
	}
	if (flags1 & 0x04000000)
	{
		dwellingaccess = Message->ReadDWORD();
	}
	if (flags1 & 0x20000000)
	{
		hookTypeUnknown = Message->ReadWORD();
		hookType = Message->ReadWORD();
	}
	if (flags1 & 0x00000040)
	{
		monarch = Message->ReadDWORD();
	}
	if (flags1 & 0x10000000)
	{
		hookableOn = Message->ReadWORD();
	}
	if (flags1 & 0x40000000)
	{
		iconOverlay = Message->ReadPackedDWORD();
	}
	if (behavior & 0x04000000)
	{
//		if (flags2 & 0x00000001)	//yes this should be flags2
//		{
			iconUnderlay = Message->ReadPackedDWORD();
//		}
	}
	if (flags1 & 0x80000000)
	{
		material = Message->ReadDWORD();
	}
	//end flags1

	//now precalc shit
	CalcPosition.CalcFromLocation(&location);
	CalcHeading();

	Unlock();
}

void cWObject::ParseMessageUpdatePosition(cMessage * Message)
{
    // 0xF748 message
    // Server updated position of object
	Lock();

	DWORD flags = Message->ReadDWORD();

	location.cell_id = Message->ReadDWORD();
	location.Origin.x = Message->ReadFloat();
	location.Origin.y = Message->ReadFloat();
	location.Origin.z = Message->ReadFloat();
	if (~flags & 0x08) location.Orientation.w = Message->ReadFloat();
	if (~flags & 0x10) location.Orientation.a = Message->ReadFloat();// else location.Orientation.a = 0;
	if (~flags & 0x20) location.Orientation.b = Message->ReadFloat();// else location.Orientation.b = 0;
	if (~flags & 0x40) location.Orientation.c = Message->ReadFloat();
	if (flags & 0x01)
	{
		//velocity
		float tx = Message->ReadFloat();
		float ty = Message->ReadFloat();
		float tz = Message->ReadFloat();
		Velocity = cPoint3D(tx, ty, tz);
	}
	if (flags & 0x02)
	{
		DWORD unknown = Message->ReadDWORD();
	}
	if (flags & 0x04)
	{
		numLogins = Message->ReadWORD();
		moveCount = Message->ReadWORD();
		numPortals = Message->ReadWORD();
		numOverride = Message->ReadWORD();
	}

	CalcPosition.CalcFromLocation(&location);
	CalcHeading();

	Unlock();
}

void cWObject::AdjustStack(DWORD Count, DWORD Value)
{
	//hmmmm, make sure this is right at some point
	Lock();
	stackCount = (WORD) Count;
	value = Value;
	Unlock();
}

void cWObject::Set229(DWORD Type, DWORD Value)
{
	Lock();
	switch (Type)
	{
	case 0x0A:
		{
			//set coverage
			coverage2 = Value;
			break;
		}
	case 0x5C:
		{
			//uses remaining
			usesLeft = (WORD) Value;
			break;
		}
	case 0xC1:
		{
			//keys on keyring
			usesLeft = (WORD) Value;//same variable?
			break;
		}
	};
	Unlock();
}

void cWObject::Set22D(DWORD Type, DWORD Value)
{
	Lock();
	switch (Type)
	{
	case 0x02:
		{
			//set as container
			wielder = 0;
			container = Value;
			break;
		}
	case 0x03:
		{
			//set as wielder
			wielder = Value;
			container = 0;
			break;
		}
	};
	Unlock();
}

DWORD cWObject::GetGUID()
{
	return GUID;
}

DWORD cWObject::GetWielder()
{
	return wielder;
}

cPoint3D cWObject::GetPosition()
{
	return CalcPosition;
}

float cWObject::GetHeading()
{
	return m_fHeading;
}

DWORD cWObject::GetObjectFlags2()
{
	return category;
}

DWORD cWObject::GetRadarOverride()
{
	return unknown5;
}

void cWObject::CalcHeading()
{
	//trimmed way down quat->euler algo

	double matrix[3][3];
	double sy;
	double cz,sz;

	// CONVERT QUATERNION TO MATRIX - I DON'T REALLY NEED ALL OF IT
	matrix[0][0] = 1.0f - (2.0f * location.Orientation.b * location.Orientation.b) - (2.0f * location.Orientation.c * location.Orientation.c);
//	matrix[0][1] = (2.0f * location.Orientation.a * location.Orientation.b) - (2.0f * location.Orientation.w * location.Orientation.c);
//	matrix[0][2] = (2.0f * location.Orientation.a * location.Orientation.c) + (2.0f * location.Orientation.w * location.Orientation.b);
	matrix[1][0] = (2.0f * location.Orientation.a * location.Orientation.b) + (2.0f * location.Orientation.w * location.Orientation.c);
//	matrix[1][1] = 1.0f - (2.0f * location.Orientation.a * location.Orientation.a) - (2.0f * location.Orientation.c * location.Orientation.c);
//	matrix[1][2] = (2.0f * location.Orientation.b * location.Orientation.c) - (2.0f * location.Orientation.w * location.Orientation.a);
	matrix[2][0] = (2.0f * location.Orientation.a * location.Orientation.c) - (2.0f * location.Orientation.w * location.Orientation.b);
//	matrix[2][1] = (2.0f * location.Orientation.b * location.Orientation.c) + (2.0f * location.Orientation.w * location.Orientation.a);
//	matrix[2][2] = 1.0f - (2.0f * location.Orientation.a * location.Orientation.a) - (2.0f * location.Orientation.b * location.Orientation.b);

	sy = -matrix[2][0];

	if ((sy != 1.0f) && (sy != -1.0f))	
	{
		double cy = sqrt(1 - (sy * sy));
		cz = matrix[0][0] / cy;
		sz = matrix[1][0] / cy;
		m_fHeading = (float)atan2(sz,cz);
	}
	else
	{
		cz = 1.0f;
		sz = 0.0f;
		m_fHeading = (float)atan2(sz,cz);
	}
}

void cWObject::LoadLocationHeading(float fZ)
{
	//only 
	float c2 = cos(fZ/2);
	float s2 = sin(fZ/2);
	
//	w = c1 c2 c3 - s1 s2 s3
//	x = s1 s2 c3 +c1 c2 s3
//	y = s1 c2 c3 + c1 s2 s3
//	z = c1 s2 c3 - s1 c2 s3
	location.Orientation.w = c2;
	location.Orientation.a = 0;
	location.Orientation.b = 0;
	location.Orientation.c = s2;
}

std::string cWObject::GetName()
{
	return objectName;
}

// get the landblock / cell ID
DWORD cWObject::GetCellID()
{
	return location.cell_id;
}

stLocation * cWObject::GetLocation()
{
	return &location;
}

stMoveInfo cWObject::GetMoveInfo()
{
	stMoveInfo miTemp;
	miTemp.moveCount = moveCount;
	miTemp.numLogins = numLogins;
	miTemp.numOverride = numOverride;
	miTemp.numPortals = numPortals;
	return miTemp;
}

void cWObject::SetVelocity(cPoint3D NewVelocity)
{
	Velocity = NewVelocity;
}


void cWObject::LoadMotionTable()
{
	cPortalFile *pf = m_Portal->OpenEntry(animConfig);
	if (!pf)
		return;

	cByteStream BS(pf->data, pf->length);
	BS.ReadBegin();

	MotionTable_.Unpack(&BS);
}

Animation::Stance cWObject::GetStance()
{
	return m_Stance;
}
