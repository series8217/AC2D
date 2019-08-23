//** Portions of this file ported from ACE: https://github.com/ACEmulator/ACE

#include "stdafx.h"
#include "Landblocks.h"
#include "PhysicsGlobals.h"
#include "cPoint3D.h"
#include "Polygon.h"
#include "LandDefs.h"
#include "cWorld.h"
#include <algorithm>

// to fix min/max  http://www.suodenjoki.dk/us/archive/2010/min-max.htm
using namespace std;

using namespace Physics;
using namespace Physics::Common;

BYTE landColor[33][4] = {
  {84, 67, 37, 110},
  {56, 66, 21, 110},
  {147, 154, 167, 110},
  {51, 69, 10, 110},
  {71, 37, 7, 113},
  {54, 34, 23, 113},
  {39, 35, 43, 113},
  {89, 65, 34, 113},
  {57, 41, 9, 113},
  {44, 77, 2, 113},
  {144, 99, 50, 113},
  {132, 132, 97, 113},
  {138, 93, 53, 114},
  {111, 68, 41, 114},
  {75, 85, 59, 114},
  {208, 219, 233, 114},
  {62, 108, 131, 130},
  {20, 79, 56, 130},
  {31, 80, 100, 130},
  {44, 76, 94, 130},
  {43, 59, 83, 130},
  {34, 47, 6, 130},
  {62, 108, 131, 130},
  {30, 38, 26, 130},
  {100, 79, 43, 130},
  {45, 33, 33, 130},
  {72, 72, 70, 130},
  {197, 227, 242, 130},
  {100, 79, 43, 130},
  {100, 79, 43, 130},
  {100, 79, 43, 130},
  {100, 79, 43, 130},
  {138, 130, 112, 130}
};

DWORD texnum[] = {
/*	0x145C,
	0x1459,
	0x1468,
	0x1456,
	0x145E,
	0x1462,
	0x1463,
	0x1465,	//65140005

	0x145B,
	0x1457,
	0x145D,
	0x145F,
	0x1467,
	0x14A7,
	0x145A,
	0x1464,

    0x146A,
	0x1469,
	0x146C,
	0x1461,
	0x146B,
	0x1466,
	0x1820,
	0x1827,

    0x1888,
	0x181F,
	0x1924,
	0x1900,
	0x1456,
	0x1456,
	0x1466,
	0x1458*/
		0x0500145C,
		0x05001459,
		0x05001468,
		0x05001456,
		0x05001467,
		0x05001462,
		0x05001463,
		0x05001465,

		0x0500145B,
		0x05001457,
		0x0500145D,
		0x0500145F,
		0x0500145E,
		0x050014A7,
		0x0500145A,
		0x05001464,
		
		0x0500146A,
		0x05001461,
		0x0500146C,
		0x05001469,
		0x0500146B,
		0x05001466,
		0x0500146A,
		0x05001827,
		
		0x0500145C,
		0x0500181F,
		0x05001924,
		0x05001900,
		0x05001C3A,
		0x05001C3B,
		0x05001C3C,
		0x0500145C,
		
		0x05001458
};

cLandblock::cLandblock(cWorld* world)
{
	
	ID = 0;
	m_wBlock = 0;

	SideCellCount = 0;
	SideVertexCount = 0;
	SidePolyCount = 0;
	World = world;
	TransDir = LandDefs::Direction::Unknown;
	WaterType = LandDefs::WaterType::NotWater;

	// always two polygons per landblock
	for (int i = 0; i < 2; i++)
	{
		Polygons.push_back(Physics::Polygon(i, 3, Physics::CullMode::Landblock));
	}
}

cLandblock::~cLandblock()
{
	for (std::vector<cModelGroup *>::iterator i = Models.begin(); i != Models.end(); i++)
		delete *i;
}

void cLandblock::Load(WORD wBlock)
{
	m_wBlock = wBlock;

	DWORD dwLB = ((DWORD) m_wBlock) << 16;

	//First load the FFFF -- vertex data
	cPortalFile *portalFileLandblockCell = m_Cell->OpenEntry(dwLB | 0xFFFF);
	if (!portalFileLandblockCell)
	{
		// TODO: Add request mechanism here...
		ZeroMemory(&m_lbData, sizeof(stLandblockData));
		return;
	}
	
	//Fill in landblock cell structure
	memcpy(&m_lbData, portalFileLandblockCell->data, sizeof(stLandblockData));

	ID = m_lbData.dwID;

	// update heights arrays from loaded data
	updateVertexHeights();

	LandDefs::Direction trns = LandDefs::Direction::Unknown;
	Generate(ID, 1, trns);

	//If applicable, load object block
	if (m_lbData.dwObjectBlock)	//1 for block, 0 for none
	{
        LoadObjectBlock(&dwLB);
	}
}


bool cLandblock::Generate(unsigned int landblockID, int cellScale, LandDefs::Direction transAdj)
{
	int cellWidth = LandDefs::BlockSide / cellScale;

	if (cellWidth == SideCellCount && TransDir == transAdj)
		return false;

	bool cellRegen = false;

	if (cellWidth != SideCellCount)
	{
		cellRegen = true;

		if (SideCellCount > 0)
			Destroy();

		SideCellCount = cellWidth;

		SideVertexCount = SideCellCount * LandDefs::VertexPerCell + 1;
		SidePolyCount = SideCellCount * LandDefs::VertexPerCell;

		InitPVArrays();
	}

	SideCellCount = CELLDIM;
	SideVertexCount = SideCellCount * LandDefs::VertexPerCell + 1;
	SidePolyCount = SideCellCount * LandDefs::VertexPerCell;

	TransDir = transAdj;
	ConstructVertices();

	if (TransDir != LandDefs::Direction::Inside && SideCellCount > 1 && SideCellCount < LandDefs::BlockSide){
		TransAdjust();
	}

	if (!cellRegen)
		AdjustPlanes();
	else
	{
		ConstructPolygons(landblockID);

		// XXX: TODO: implement me
		//ConstructUVs(landblockID);
	}


	// XXX: TODO: implement me
	//CalcWater();

	// TODO: implement vertex and polygon cache?
	// FinalizePVArrays();

	return cellRegen;
	
}

void cLandblock::AdjustPlanes(){
	for (int16_t x = 0; x < SidePolyCount; x++)
	{
		for (int y = 0; y < SidePolyCount; y++)
		{
			for (int polyIdx = 0; polyIdx < 2; polyIdx++)
				Polygons[(x * SidePolyCount + y) * 2 + polyIdx].make_plane();
		}
	}
}

void cLandblock::InitPVArrays(){
	// Init Arrays
	unsigned int num_vertices = SideVertexCount * SideVertexCount;
	unsigned int num_polygons = SideCellCount * SideCellCount * POLY_PER_CELL;
	unsigned int num_cells = SideCellCount * SideCellCount;

	for (int i = 0; i < num_vertices; i++) {
		m_vertex_array.Vertices.push_back(Vertex());
	}

	Polygons.reserve(num_polygons);
	for (int i = 0; i < num_polygons; i++) {
		// always 3 vertices for land cell polygons
		Polygons.push_back(Polygon(i, 3, Physics::CullMode::Landblock));
	}

	for (unsigned int i = 0; i < num_cells; i++) {
		m_landcells.try_emplace((int)i, LandblockCell((ID & LandDefs::BlockMask) + i));
	}

}

// clear PV arrays
void cLandblock::Destroy()
{
	m_vertex_array.Vertices.clear();
	Polygons.clear();
}


void cLandblock::updateVertexHeights() {

    for (int y = 0;y < LandDefs::VertexDim; y++)
    {
        for (int x = 0;x < LandDefs::VertexDim; x++)
        {
            vertexHeights[x][y] = m_lbData.bZ[x][y];
			// to make easier reuse of ACE code
			Height.push_back(m_lbData.bZ[x][y]);
        }
    }
}

// from ACE
float cLandblock::GetZ(Vector3 point) {
	LandblockCell* cell = GetCell(point);
	if (cell == NULL)
		return point.z;
	Physics::Polygon* walkable = NULL;
	if (!cell->find_terrain_poly(point, walkable)) {
		return point.z;
	}
	float adjZ = point.z;
	if (std::abs(walkable->Plane->Normal.z) > Physics::Globals::EPSILON)
		adjZ = (point.Dot2D(walkable->Plane->Normal) + walkable->Plane->D) / walkable->Plane->Normal.z * -1;
	return adjZ;
}

// from ACE
bool cLandblock::findTerrainPoly(Vector3 origin, Physics::Polygon* &walkable) {
	for (unsigned int i = 0; i < Polygons.size(); i++) {
		if (Polygons[i].point_in_poly2D(origin, Physics::Sidedness::Positive)) {
			walkable = &(Polygons[i]);
			return true;
		}
	}
	return false;
}

// from ACE
LandblockCell* cLandblock::GetCell(Vector3 point)
{
	if (point.x < 0 || point.y < 0 || point.x > 192 || point.y > 192)
		return NULL;

	DWORD cellX = (int)point.x / 24;
	DWORD cellY = (int)point.y / 24;

	DWORD blockCellID = (ID & 0xFFFF0000) | (DWORD)(cellX * 8 + cellY) + 1;
	return World->GetLandblockCell((unsigned int)blockCellID);
}

// from ACE
void cLandblock::ConstructVertices()
{
	float cellScale = LandDefs::BlockSide / SideCellCount;
	float cellSize = LandDefs::BlockLength / SidePolyCount;
	for (int y = 0; y < SideVertexCount; y++)
	{
		for (int x = 0; x < SideVertexCount; x++)
		{
			// TODO: look up and apply scaling factor in land height table from portal dat
			vertexHeights[x][y] = m_lbData.bZ[x][y];
		}
	}  
	for (int x = 0; x < SideVertexCount; x++)
	{
		float cellX = x * cellSize;

		for (int y = 0; y < SideVertexCount; y++)
		{
			float cellY = y * cellSize;
			unsigned short vertex_idx = x * SideVertexCount + y;

			Vertex* vertex = &(m_vertex_array.Vertices[vertex_idx]);
			float zHeight = LandDefs::LandHeightTable[vertexHeights[x][y]];

			vertex->Origin = Vector3(cellX, cellY, zHeight * cellScale);
		}
	}
}

// from ACE
void cLandblock::TransAdjust()
{
	// refactor me..
	if (TransDir == LandDefs::Direction::North || TransDir == LandDefs::Direction::NorthWest || TransDir == LandDefs::Direction::NorthEast)
	{
		for (int i = 1; i < SidePolyCount; i += 2)
		{
			Vertex* v0 = &m_vertex_array.Vertices[((i - 1) * SideVertexCount) + SidePolyCount];
			Vertex* v1 = &m_vertex_array.Vertices[((i + 1) * SideVertexCount) + SidePolyCount];
			Vertex* v2 = &m_vertex_array.Vertices[(i * SideVertexCount) + SidePolyCount];

			v2->Origin.z = (v0->Origin.z + v1->Origin.z) / 2;
		}
	}
	if (TransDir == LandDefs::Direction::West || TransDir == LandDefs::Direction::NorthWest || TransDir == LandDefs::Direction::SouthWest)
	{
		for (int i = 1; i < SidePolyCount; i += 2)
		{
			Vertex* v0 = &m_vertex_array.Vertices[i - 1];
			Vertex* v1 = &m_vertex_array.Vertices[i + 1];
			Vertex* v2 = &m_vertex_array.Vertices[i];

			v2->Origin.z = (v0->Origin.z + v1->Origin.z) / 2;
		}
	}
	if (TransDir == LandDefs::Direction::South || TransDir == LandDefs::Direction::SouthWest || TransDir == LandDefs::Direction::SouthEast)
	{
		for (int i = 1; i < SidePolyCount; i += 2)
		{
			Vertex* v0 = &m_vertex_array.Vertices[(i - 1) * SideVertexCount];
			Vertex* v1 = &m_vertex_array.Vertices[(i + 1) * SideVertexCount];
			Vertex* v2 = &m_vertex_array.Vertices[i * SideVertexCount];

			v2->Origin.z = (v0->Origin.z + v1->Origin.z) / 2;
		}
	}
	if (TransDir == LandDefs::Direction::East || TransDir == LandDefs::Direction::NorthEast || TransDir == LandDefs::Direction::SouthEast)
	{
		for (int i = 1; i < SidePolyCount; i += 2)
		{
			Vertex* v0 = &m_vertex_array.Vertices[SideVertexCount * SidePolyCount + i - 1];
			Vertex* v1 = &m_vertex_array.Vertices[SideVertexCount * SidePolyCount + i + 1];
			Vertex* v2 = &m_vertex_array.Vertices[SideVertexCount * SidePolyCount + i];

			v2->Origin.z = (v0->Origin.z + v1->Origin.z) / 2;
		}
	}

	if (SideCellCount != LandDefs::BlockSide / 2)
		return;

	if (TransDir == LandDefs::Direction::North)
	{
		for (int i = 1, j = 4; i < SidePolyCount; i += 2, j += 4)
		{
			Vertex* vertex = &m_vertex_array.Vertices[i * SideVertexCount];

			float height0 = LandDefs::LandHeightTable[Height[(j - 1) * SideVertexCount]];
			float height1 = LandDefs::LandHeightTable[Height[j * SideVertexCount]];
			float height2 = LandDefs::LandHeightTable[Height[(j - 3) * SideVertexCount]];
			float height3 = LandDefs::LandHeightTable[Height[(j - 4) * SideVertexCount]];
			
			vertex->Origin.z = min(vertex->Origin.z, height2 * 2 - height3);
			vertex->Origin.z = min(vertex->Origin.z, height0 * 2 - height1);
		}
	}

	if (TransDir == LandDefs::Direction::South)
	{
		for (int i = 1, j = 4; i < SidePolyCount; i += 2, j += 4)
		{
			Vertex* vertex = &m_vertex_array.Vertices[i * SideVertexCount + SidePolyCount];

			float height0 = LandDefs::LandHeightTable[Height[(j - 1) * SideVertexCount + SideVertexCount - 1]];
			float height1 = LandDefs::LandHeightTable[Height[j * SideVertexCount + SideVertexCount - 1]];
			float height2 = LandDefs::LandHeightTable[Height[(j - 3) * SideVertexCount + SideVertexCount - 1]];
			float height3 = LandDefs::LandHeightTable[Height[(j - 4) * SideVertexCount + SideVertexCount - 1]];

			vertex->Origin.z = min(vertex->Origin.z, height2 * 2 - height3);
			vertex->Origin.z = min(vertex->Origin.z, height0 * 2 - height1);
		}
	}

	if (TransDir == LandDefs::Direction::East)
	{
		for (int i = 1; i < SidePolyCount; i += 2)
		{
			Vertex* vertex = &m_vertex_array.Vertices[i];

			float height0 = LandDefs::LandHeightTable[Height[i * 2 + 1]];
			float height1 = LandDefs::LandHeightTable[Height[i * 2 + 2]];
			float height2 = LandDefs::LandHeightTable[Height[i * 2 - 1]];
			float height3 = LandDefs::LandHeightTable[Height[i * 2 - 2]];

			vertex->Origin.z = min(vertex->Origin.z, height2 * 2 - height3);
			vertex->Origin.z = min(vertex->Origin.z, height0 * 2 - height1);
		}
	}

	if (TransDir == LandDefs::Direction::West)
	{
		for (int i = 1; i < SidePolyCount; i += 2)
		{
			Vertex* vertex = &m_vertex_array.Vertices[i + SideVertexCount * SideVertexCount];

			float height0 = LandDefs::LandHeightTable[Height[SideVertexCount * (SideVertexCount - 1) + i * 2 + 1]];
			float height1 = LandDefs::LandHeightTable[Height[SideVertexCount * (SideVertexCount - 1) + i * 2 + 2]];
			float height2 = LandDefs::LandHeightTable[Height[SideVertexCount * (SideVertexCount - 1) + i * 2 - 1]];
			float height3 = LandDefs::LandHeightTable[Height[SideVertexCount * (SideVertexCount - 1) + i * 2 - 2]];

			vertex->Origin.z = min(vertex->Origin.z, height2 * 2 - height3);
			vertex->Origin.z = min(vertex->Origin.z, height0 * 2 - height1);
		}
	}
}


Physics::Polygon* cLandblock::AddPolygon(int polyIdx, int _v0, int _v1, int _v2)
{
	Polygon* polygon = &Polygons[polyIdx];
	short v0 = (short)_v0;
	short v1 = (short)_v1;
	short v2 = (short)_v2;

	polygon->Vertices[0] = m_vertex_array.Vertices[v0];
	polygon->Vertices[1] = m_vertex_array.Vertices[v1];
	polygon->Vertices[2] = m_vertex_array.Vertices[v2];

	polygon->VertexIDs[0] = v0;
	polygon->VertexIDs[1] = v1;
	polygon->VertexIDs[2] = v2;

	polygon->make_plane();

	polygon->PosSurface = 0;

	if (polygon->Vertices[0].Origin.z != 0 || polygon->Vertices[1].Origin.z != 0 || polygon->Vertices[2].Origin.z != 0)
		polygon->PosSurface = 1;

	return &(*polygon);
}


void cLandblock::ConstructPolygons(unsigned int landblockID) {
	Vector2* lcoord = LandDefs::blockid_to_lcoord(landblockID);
	assert(lcoord);

	int polyNum = 1;
	float seedA = (unsigned int)((int)lcoord->x * LandDefs::VertexPerCell * 214614067);
	float seedB = (unsigned int)((int)lcoord->y * LandDefs::VertexPerCell * 1109124029);
	int vertexCnt = 0;

	for (unsigned int x = 0; x < SideCellCount; x++)
	{
		DWORD lcoordX = lcoord->x + x;
		unsigned int originY = 1;
		unsigned int colVertexCnt = 0;
		unsigned int originX = polyNum;

		for (unsigned int y = 0; y < SideCellCount; y++)
		{
			DWORD lcoordY = (int)lcoord->y + y;
			float magicB = seedB;
            float magicA = seedA + 1813693831;

			for (unsigned int i = 0; i < LandDefs::VertexPerCell; i++)
			{
				unsigned int idxI = vertexCnt + i;
				unsigned int idxJ = colVertexCnt;

				for (unsigned int j = 0; j < LandDefs::VertexPerCell; j++)
				{
					bool NESW = getSplitDirectionNESW(x, y);
					unsigned int polyIdx = (LandDefs::VertexPerCell * i + j) * 2;

					unsigned int cellIdx = x * SideCellCount + y;
					unsigned int vertIdx = idxI * SideVertexCount + idxJ;
					unsigned int firstVertex = (idxI * SidePolyCount + idxJ) * 2;
					unsigned int nextVertIdx = (idxI + 1) * SideVertexCount + idxJ;

					LandblockCell* lcell = &m_landcells[cellIdx];
					
					if (!NESW)
					{
						//  2    1---0
						//  | \   \  |
						//  |  \   \ |
						//  0---1    2
						SWtoNEcut[x][y] = false;

						lcell->Polygons[polyIdx] = AddPolygon(firstVertex, vertIdx, nextVertIdx, vertIdx + 1);
						lcell->Polygons[polyIdx + 1] = AddPolygon(firstVertex + 1, nextVertIdx + 1, vertIdx + 1, nextVertIdx);
					}
					else
					{
						//     2   2---1
						//    / |  |  /
						//   /  |  | /
						//  0---1  0

						SWtoNEcut[x][y] = true;

						lcell->Polygons[polyIdx] = AddPolygon(firstVertex, vertIdx, nextVertIdx, nextVertIdx + 1);
						lcell->Polygons[polyIdx + 1] = AddPolygon(firstVertex + 1, vertIdx, nextVertIdx + 1, vertIdx + 1);
					}
					idxJ++;
				}
			}

			unsigned int cellID = (unsigned int)LandDefs::lcoord_to_gid(lcoordX, lcoordY);
			LandblockCell* landCell = &m_landcells[x * SideCellCount + y];
			landCell->cellID = cellID;
			// TOOD: implement Pos member / Position class:
			//landCell->Pos.ObjCellID = cellID;
			landCell->Origin.x = originX * LandDefs::HalfSquareLength;
			landCell->Origin.y = originY * LandDefs::HalfSquareLength;
			landCell->Origin.z = 0;

			originY += 2;
			colVertexCnt += LandDefs::VertexPerCell;
		}
		vertexCnt += LandDefs::VertexPerCell;
		seedB += 1109124029 * (unsigned int)LandDefs::VertexPerCell;
        seedA += 214614067 * (unsigned int)LandDefs::VertexPerCell;
		polyNum += 2;

	}
	delete lcoord;
}


void cLandblock::LoadObjectBlock(DWORD *dwLB) {
    //Now load the FFFE
    cPortalFile *pfFFFE = m_Cell->OpenEntry(*dwLB | 0xFFFE);
    if (!pfFFFE)
    {
        //Add request mechanism here...
        return;
    }

    cByteStream pBS(pfFFFE->data, pfFFFE->length);

    pBS.ReadBegin();
    DWORD dwID = pBS.ReadDWORD();

    DWORD iNumCells = pBS.ReadDWORD();
    for (DWORD j = 0;j < iNumCells;j++)
    {
        //Read the cells (0xYYXX01xx
        DWORD pfID = (*dwLB | 0x0100) + j;
        cPortalFile *pfTemp = m_Cell->OpenEntry(pfID);
        if (!pfTemp)
            continue;

        cByteStream pBS2(pfTemp->data, pfTemp->length);
        pBS2.ReadBegin();

        pBS2.ReadDWORD();	//ID
        DWORD dwCellFlags = pBS2.ReadDWORD();
        pBS2.ReadDWORD(); //ID again for some reason

        BYTE bTexCount = pBS2.ReadByte();	//Num of words below
        BYTE bConnCount = pBS2.ReadByte();	//2x this = number of extra DWORDs down below
        WORD wVisCount = pBS2.ReadWORD();

        std::vector<WORD> vTexes;
        vTexes.clear();
        //These are texture refs for the dungeonpart
        for (int i = 0; i < bTexCount; i++)
            vTexes.push_back(pBS2.ReadWORD());

        DWORD DungeonRef = 0x0D000000 | pBS2.ReadWORD();	//Dungeon Ref
        WORD DungeonPart = pBS2.ReadWORD();	//Which part of the dungeon ref it needs

        stLocation tpld;
        tpld.Origin.x = pBS2.ReadFloat();
        tpld.Origin.y = pBS2.ReadFloat();
        tpld.Origin.z = pBS2.ReadFloat();
        tpld.Orientation.w = pBS2.ReadFloat();
        tpld.Orientation.a = pBS2.ReadFloat();
        tpld.Orientation.b = pBS2.ReadFloat();
        tpld.Orientation.c = pBS2.ReadFloat();
        tpld.cell_id = *dwLB;
        cPoint3D tp3d;
        tp3d.CalcFromLocation(&tpld);

        cModelGroup *DModel = new cModelGroup();
        if (!DModel->ReadDungeon(DungeonRef, DungeonPart, &vTexes))
        {
            return;
        }
        DModel->SetScale(MODEL_SCALE_FACTOR);
        DModel->SetTranslation(tp3d);
        DModel->SetRotation(tpld.Orientation.w, tpld.Orientation.a, tpld.Orientation.b, tpld.Orientation.c);
        Models.push_back(DModel);

        //Kewt says this is connections to adjascent cells
        for (int i = 0; i < bConnCount; i++)
        {
            pBS2.ReadWORD();	//Type
            pBS2.ReadWORD();	//Face
            pBS2.ReadWORD();	//Block
            pBS2.ReadWORD();	//Unknown
        }

        //List of visible cells from this cell
        for (int i = 0; i < wVisCount; i++)
            pBS2.ReadWORD();

        if (dwCellFlags & 1)
        {
            //?...
        }

        if (dwCellFlags & 2)
        {
            //models!
            DWORD dwModelCount = pBS2.ReadDWORD();

            for (int i = 0; i < (int)dwModelCount; i++)
            {
                DWORD dwModelID = pBS2.ReadDWORD();
                stLocation tpl;
                tpl.Origin.x = pBS2.ReadFloat();
                tpl.Origin.y = pBS2.ReadFloat();
                tpl.Origin.z = pBS2.ReadFloat();
                tpl.Orientation.w = pBS2.ReadFloat();
                tpl.Orientation.a = pBS2.ReadFloat();
                tpl.Orientation.b = pBS2.ReadFloat();
                tpl.Orientation.c = pBS2.ReadFloat();
                tpl.cell_id = *dwLB;
                cPoint3D translation_point3d;
                translation_point3d.CalcFromLocation(&tpl);

                cModelGroup *Model = new cModelGroup();
                if (!Model->ReadModel(dwModelID))
                {
                    return;
                    //			OutputString(eRed, "Failed model load: %08X", m_lbFFFE.Objects[j].dwID);
                    //				OutputConsoleString("Failed model load: %08X", dwModelID);
                }
                Model->SetScale(MODEL_SCALE_FACTOR);
                Model->SetTranslation(translation_point3d);
                Model->SetRotation(tpl.Orientation.w, tpl.Orientation.a, tpl.Orientation.b, tpl.Orientation.c);
                Models.push_back(Model);
            }
        }

        if (dwCellFlags & 8)
        {
            pBS2.ReadDWORD();	//? - 7D2221A0
        }
    }

    DWORD iNumObjects = pBS.ReadDWORD();
    for (DWORD j = 0;j < iNumObjects;j++)
    {
        DWORD dwModelID = pBS.ReadDWORD();
        stLocation tpl;
        tpl.Origin.x = pBS.ReadFloat();
        tpl.Origin.y = pBS.ReadFloat();
        tpl.Origin.z = pBS.ReadFloat();
        tpl.Orientation.w = pBS.ReadFloat();
        tpl.Orientation.a = pBS.ReadFloat();
        tpl.Orientation.b = pBS.ReadFloat();
        tpl.Orientation.c = pBS.ReadFloat();
        tpl.cell_id = *dwLB;
        cPoint3D tp3d;
        tp3d.CalcFromLocation(&tpl);

        cModelGroup *Model = new cModelGroup();
        if (!Model->ReadModel(dwModelID))
        {
            return;
            //			OutputString(eRed, "Failed model load: %08X", m_lbFFFE.Objects[j].dwID);
//				OutputConsoleString("Failed model load: %08X", dwModelID);
        }
        Model->SetScale(MODEL_SCALE_FACTOR);
        Model->SetTranslation(tp3d);
        Model->SetRotation(tpl.Orientation.w, tpl.Orientation.a, tpl.Orientation.b, tpl.Orientation.c);
        Models.push_back(Model);
    }

    WORD wNumSkins = pBS.ReadWORD();
    WORD wSkinUnknown = pBS.ReadWORD();
    for (int j = 0; j < wNumSkins; j++)
    {
        DWORD dwModelID = pBS.ReadDWORD();
        stLocation tpl;
        tpl.Origin.x = pBS.ReadFloat();
        tpl.Origin.y = pBS.ReadFloat();
        tpl.Origin.z = pBS.ReadFloat();
        tpl.Orientation.w = pBS.ReadFloat();
        tpl.Orientation.a = pBS.ReadFloat();
        tpl.Orientation.b = pBS.ReadFloat();
        tpl.Orientation.c = pBS.ReadFloat();
        tpl.cell_id = *dwLB;
        cPoint3D tp3d;
        tp3d.CalcFromLocation(&tpl);

        cModelGroup *Model = new cModelGroup();
        if (!Model->ReadModel(dwModelID))
        {
            return;
            //			OutputString(eRed, "Failed model load: %08X", dwModelID);
//				OutputConsoleString("Failed model load: %08X", dwModelID);
        }
        Model->SetScale(MODEL_SCALE_FACTOR);
        Model->SetTranslation(tp3d);
        Model->SetRotation(tpl.Orientation.w, tpl.Orientation.a, tpl.Orientation.b, tpl.Orientation.c);
        Models.push_back(Model);

        //Skip the strange data 
        DWORD unknown1 = pBS.ReadDWORD();
        DWORD dwWeirdCount = pBS.ReadDWORD();
        for (DWORD k = 0; k < dwWeirdCount; k++)
        {
            DWORD dwUnk1 = pBS.ReadDWORD();
            WORD wUnk1 = pBS.ReadWORD();
            WORD wTableCount = pBS.ReadWORD();
            for (int l = 0; l < wTableCount; l++)
            {
                WORD wUnk2 = pBS.ReadWORD();
            }
            if (wTableCount & 1)
                pBS.ReadWORD();
        }
    }
}

bool cLandblock::getSplitDirectionNESW(DWORD cellX, DWORD cellY)
{
    // get global tile offsets
    DWORD dwBlockX = (m_lbData.dwID & 0xff000000) >> 24;
    DWORD dwBlockY = (m_lbData.dwID & 0x00ff0000) >> 16;

    DWORD x = (dwBlockX * 8) | cellX;
    DWORD y = (dwBlockY * 8) | cellY;

    /** Determines the split line direction for a cell triangulation */
	DWORD dw = x * y * 0x0CCAC033 - x * 0x421BE3BD + y * 0x6C1AC587 - 0x519B8F25;
    /* returns true if NE-SW split, FALSE if NW-SE split */
	return (dw & 0x80000000) != 0;
}

int cLandblock::Draw()
{
	// XXX: TODO: use stored polygons and UV instead of generating them here each time
	/*

	Landblock notes:
	Landblocks are XXYY____
	x is EW
	y is NS
	Landblocks start at (0,0) = SW corner

	*/
	//Draw the FFFF
	float fXCorner, fYCorner;
	DWORD dwBlockX = (m_lbData.dwID & 0xff000000) >> 24;
	DWORD dwBlockY = (m_lbData.dwID & 0x00ff0000) >> 16;
	
	// TODO: indoor
/*	if (dwBlockX < 3)
	{
		// dungeon, dwBlockX and dwBlockY remain constant - just use x and y
		fXCorner = 0;
		fYCorner = 0;
	}
	else*/
	{
		// outdoors
		fXCorner = (float) ((((dwBlockX+1.00f) * 8.0f) - 1027.5) * LANDBLOCK_SCALE);
		fYCorner = (float) ((((dwBlockY+1.00f) * 8.0f) - 1027.5) * LANDBLOCK_SCALE);
	}

	// TODO: rescale heights from table?
	float fDiv = (1.0f/ MODEL_SCALE_FACTOR) / 2.0f;
    glPushName(0xDEADBEEF);

	for (int y=0;y<CELLDIM;y++)
	{
		for (int x=0;x<CELLDIM;x++)
		{
			int type0;
            if ((m_lbData.wTopo[x][y] & 0x0003)) {
                type0 = 32;
            }
            else {
                type0 = (m_lbData.wTopo[x][y] & 0x00FF) >> 2;
            }

			bool NESW = getSplitDirectionNESW(x, y);

			int tex1 = m_Portal->FindTexturePalette(texnum[type0], 0);

			glBindTexture( GL_TEXTURE_2D, tex1);

			glBegin(GL_TRIANGLE_FAN);
			glColor4f(1,1,1,1.0f);
            // polygons

			if (NESW)
			{
				//  D---C
				//  | / | 
				//  |/  |
				//  A---B
                glTexCoord2f(0,0);
				glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x][y]/fDiv);

				glTexCoord2f(2,0);
				glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x+1][y]/fDiv);

				glTexCoord2f(2,2);
				glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x+1][y+1]/fDiv);

				glTexCoord2f(0,2);
				glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x][y+1]/fDiv);
			}
			else
			{ // NW-SE
				//  C---B
				//  |\  |
				//  | \ |
				//  D---A
				glTexCoord2f(2,0);
				glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x+1][y]/fDiv);

				glTexCoord2f(2,2);
				glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x+1][y+1]/fDiv);

				glTexCoord2f(0,2);
				glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x][y+1]/fDiv);

				glTexCoord2f(0,0);
				glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x][y]/fDiv);
			}
			glEnd();
		}
	}
	glPopName();


	//Draw the Objects
	int m_iTriCount = 8*8*2;
	for (std::vector<cModelGroup *>::iterator i = Models.begin(); i != Models.end(); i++)
		m_iTriCount += (*i)->Draw();

	return m_iTriCount;
}

//				glColor4ub(landColor[type1][0], landColor[type1][1], landColor[type1][2], 255);

/*			glBegin(GL_LINE_STRIP);

			glColor4ub(landColor[type0][0], landColor[type0][1], landColor[type0][2], 255);
			glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x][y]/fDiv);

			glColor4ub(landColor[type1][0], landColor[type1][1], landColor[type1][2], 255);
			glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x+1][y]/fDiv);

			glColor4ub(landColor[type2][0], landColor[type2][1], landColor[type2][2], 255);
			glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x+1][y+1]/fDiv);

			glColor4ub(landColor[type3][0], landColor[type3][1], landColor[type3][2], 255);
			glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x][y+1]/fDiv);

			glEnd();

			glBegin(GL_LINE_STRIP);

			if (NESW)
			{
			glColor4ub(landColor[type0][0], landColor[type0][1], landColor[type0][2], 255);
			glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x][y]/fDiv);
			glColor4ub(landColor[type2][0], landColor[type2][1], landColor[type2][2], 255);
			glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x+1][y+1]/fDiv);
			}
			else
			{
			glColor4ub(landColor[type1][0], landColor[type1][1], landColor[type1][2], 255);
			glVertex3f(fXCorner + (x+1)*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x+1][y]/fDiv);
			glColor4ub(landColor[type3][0], landColor[type3][1], landColor[type3][2], 255);
			glVertex3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + (y+1)*LANDBLOCK_SCALE, vertexHeights[x][y+1]/fDiv);
			}

			glEnd();*/

/*			glBindTexture( GL_TEXTURE_2D, 0);
			if (type0 == 32)
			{
				glColor3f(1.0,1.0,1.0);
				glRasterPos3f(fXCorner + x*LANDBLOCK_SCALE, fYCorner + y*LANDBLOCK_SCALE, vertexHeights[x][y]/fDiv);

				char lele[50];
				snprintf(lele, 50, "%02X - %08X - %04X", type0, texnum[type0], m_lbData.wTopo[x][y]);
				glCallLists(strlen(lele), GL_UNSIGNED_BYTE, lele); 
			}*/


bool LandblockCell::find_terrain_poly(Vector3 origin, Physics::Polygon* &walkable) const{
	for (int i = 0; i < 2; i++)
	{
		if (Polygons[i]->point_in_poly2D(origin, Physics::Sidedness::Positive))
		{
			walkable = (Polygons[i]);
			return true;
		}
	}
	return false;
}