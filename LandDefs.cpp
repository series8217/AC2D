/*
 * This file was ported almost entirely from ACE https://github.com/ACEmulator/ACE
 */
#include "stdafx.h"

#include "LandDefs.h"
#include "Vector2.h"
#include "Vector3.h"
#include "cByteStream.h"
#include "PhysicsGlobals.h"

using namespace Physics::Common;


std::vector<float> LandDefs::LandHeightTable;

LandDefs::LandDefs()
{
	// XXX: TODO: implement static constructor that does this?
	//            ----> for now we just call InitFromPortalDat in cWorld()
	//if (DatManager.PortalDat != null)
	//    LandHeightTable = DatManager.PortalDat.RegionDesc.LandDefs.LandHeightTable;
}

void LandDefs::_Unpack(cByteStream *reader) {
	// we don't actually use any of these, though theoertical some things like VertexPerCell could vary
	int NumBlockLength = reader->ReadDWORD();
	int NumBlockWidth = reader->ReadDWORD();
	float SquareLength = reader->ReadFloat();
	int LBlockLength = reader->ReadDWORD();
	int VertexPerCell = reader->ReadDWORD();
	int MaxObjHeight = reader->ReadFloat();
	int SkyHeight = reader->ReadFloat();
	int RoadWidth = reader->ReadFloat();

	// load the land height table which provides non-linear height mapping
	for (int i = 0; i < 256; i++) {
		LandDefs::LandHeightTable.push_back(reader->ReadFloat());
	}
}

void LandDefs::InitFromPortalDat() {
	cPortalFile *PF = m_Portal->OpenEntry(0x13000000);
	cByteStream *BS = new cByteStream(PF->data, PF->length);

	BS->ReadBegin();
	DWORD Id = BS->ReadDWORD();

	DWORD RegionNumber = BS->ReadDWORD();
	DWORD Version = BS->ReadDWORD();
	char* RegionName = BS->ReadString(); // "Dereth"
	delete RegionName;
	BS->ReadAlign();

	_Unpack(BS);
	// GameTime.Unpack(reader);

	// PartsMask = BS->ReadDWORD();

	// if ((PartsMask & 0x10) != 0)
	//     SkyInfo.Unpack(reader);

	// if ((PartsMask & 0x01) != 0)
	//     SoundInfo.Unpack(reader);

	// if ((PartsMask & 0x02) != 0)
	//     SceneInfo.Unpack(reader);

	// TerrainInfo.Unpack(reader);

	// if ((PartsMask & 0x0200) != 0)
	//     RegionMisc.Unpack(reader);
	// BS->ReadWORD();

	delete BS;
}

// XXX: TODO: Implement Position class so we can use this
// static bool AdjustToOutside(Position pos)
// {
//     return AdjustToOutside(ref pos.ObjCellID, ref pos.Frame.Origin);
// }

bool LandDefs::AdjustToOutside(unsigned int &blockCellID, Vector3 &loc)
{
	unsigned int cellID = (unsigned int)(blockCellID & CellID_Mask);

	if (cell_in_range(cellID))
	{
		if (std::abs(loc.x) < Physics::Globals::EPSILON)
			loc.x = 0;
		if (std::abs(loc.y) < Physics::Globals::EPSILON)
			loc.y = 0;

		Vector2* lcoord = get_outside_lcoord(blockCellID, loc.x, loc.y);
		if (lcoord)
		{
			blockCellID = (unsigned int)lcoord_to_gid(lcoord->x, lcoord->y);
			loc.x -= (float)std::floor(loc.x / BlockLength) * BlockLength;
			loc.y -= (float)std::floor(loc.y / BlockLength) * BlockLength;
			delete lcoord;
			return true;
		}
	}
	blockCellID = 0;
	return false;
}

Vector3* LandDefs::GetBlockOffset(unsigned int cellFrom, unsigned int cellTo)
{
	if (cellFrom >> BlockPartShift == cellTo >> BlockPartShift)
		return new Vector3(0.0, 0.0, 0.0);

	Vector2* localFrom = blockid_to_lcoord(cellFrom);
	Vector2* localTo = blockid_to_lcoord(cellTo);

	Vector3* offset = new Vector3((localTo->x - localFrom->x) * SquareLength, (localTo->y - localFrom->y) * SquareLength, 0.0f);

	delete localFrom;
	delete localTo;

	return offset;
}

bool LandDefs::InBlock(Vector3 pos, float radius)
{
	if (pos.x < radius || pos.y < radius)
		return false;

	float blockRadius = LandDefs::BlockLength - radius;
	return pos.x < blockRadius && pos.y < blockRadius;
}

Vector2* LandDefs::blockid_to_lcoord(unsigned int cellID)
{
	Vector2* lcoord;
	int x = (cellID >> BlockPartShift & BlockX_Mask) >> MaxBlockShift << LandblockShift;
	int y = (cellID >> BlockPartShift & BlockY_Mask) << LandblockShift;

	if (x < 0 || y < 0 || x >= LandLength || y >= LandLength)
		lcoord = NULL;
	else
		lcoord = new Vector2(x, y);
	return lcoord;
}

Vector2* LandDefs::gid_to_lcoord(DWORD cellID, bool envCheck)
{
	if (!inbound_valid_cellid(cellID))
		return NULL;

	if (envCheck && (cellID & CellID_Mask) >= FirstEnvCellID)
		return NULL;

	int x = (cellID >> BlockPartShift & BlockX_Mask) >> MaxBlockShift << LandblockShift;
	int y = (cellID >> BlockPartShift & BlockY_Mask) << LandblockShift;

	x += ((cellID & CellID_Mask) - FirstLandCellID) >> LandblockShift;
	y += ((cellID & CellID_Mask) - FirstLandCellID) & LandblockMask;

	if (x < 0 || y < 0 || x >= LandLength || y >= LandLength)
		return NULL;

	return new Vector2(x, y);
}

Vector2* LandDefs::get_outside_lcoord(unsigned int blockCellID, float _x, float _y)
{
	unsigned int cellID = (unsigned int)(blockCellID & CellID_Mask);

	if (cell_in_range(cellID))
	{
		Vector2* offset = blockid_to_lcoord(blockCellID);
		if (!offset) {
			return NULL;
		}
		float x = offset->x + (float)std::floor(_x / CellLength);
		float y = offset->y + (float)std::floor(_y / CellLength);

		delete offset;

		if (x < 0 || y < 0 || x >= LandLength || y >= LandLength)
			return NULL;
		else
			return new Vector2(x, y);
	}
	return NULL;
}

bool LandDefs::cell_in_range(unsigned int cellID)
{
	return cellID == BlockCellID ||
		cellID >= FirstLandCellID && cellID <= LastLandCellID ||
		cellID >= FirstEnvCellID && cellID <= LastEnvCellID;
}

int LandDefs::lcoord_to_gid(float _x, float _y)
{
	int x = (int)_x;
	int y = (int)_y;

	if (x < 0 || y < 0 || x >= LandLength || y >= LandLength)
		return 0;

	int block = (x >> LandblockShift << MaxBlockShift) | (y >> LandblockShift);
	int cell = FirstLandCellID + ((x & LandblockMask) << LandblockShift) + (y & LandblockMask);

	return block << BlockPartShift | cell;
}

bool LandDefs::inbound_valid_cellid(unsigned int blockCellID)
{
	int cellID = (unsigned int)(blockCellID & CellID_Mask);

	if (cell_in_range(cellID))
	{
		unsigned int block_x = (blockCellID >> BlockPartShift & BlockX_Mask) >> MaxBlockShift << LandblockShift;
		if (block_x >= 0 && block_x < LandLength)
			return true;
	}
	return false;
}

