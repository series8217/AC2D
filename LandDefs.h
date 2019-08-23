//** Ported from ACE: https://github.com/ACEmulator/ACE

#pragma once


#include "stdafx.h"
#include "Vector2.h"
#include "Vector3.h"
#include "cByteStream.h"
#include "PhysicsGlobals.h"

namespace Physics
{
    namespace Common
    {
        class LandDefs
        {
            public:

            enum class Direction
            {
                Inside = 0x0,
                North = 0x1,
                South = 0x2,
                East = 0x3,
                West = 0x4,
                NorthWest = 0x5,
                SouthWest = 0x6,
                NorthEast = 0x7,
                SouthEast = 0x8,
                Unknown = 0x9
            };

            enum class WaterType
            {
                NotWater = 0x0,
                PartiallyWater = 0x1,
                EntirelyWater = 0x2,
            };

            enum class Rotation
            {
                Rot0 = 0x0,
                Rot90 = 0x1,
                Rot180 = 0x2,
                Rot270 = 0x3
            };

            enum class PalType
            {
                SWTerrain = 0x0,
                SETerrain = 0x1,
                NETerrain = 0x2,
                NWTerrain = 0x3,
                Road = 0x4
            };

            enum class TerrainType
            {
                BarrenRock = 0x0,
                Grassland = 0x1,
                Ice = 0x2,
                LushGrass = 0x3,
                MarshSparseSwamp = 0x4,
                MudRichDirt = 0x5,
                ObsidianPlain = 0x6,
                PackedDirt = 0x7,
                PatchyDirt = 0x8,
                PatchyGrassland = 0x9,
                SandYellow = 0xA,
                SandGrey = 0xB,
                SandRockStrewn = 0xC,
                SedimentaryRock = 0xD,
                SemiBarrenRock = 0xE,
                Snow = 0xF,
                WaterRunning = 0x10,
                WaterStandingFresh = 0x11,
                WaterShallowSea = 0x12,
                WaterShallowStillSea = 0x13,
                WaterDeepSea = 0x14,
                Reserved21 = 0x15,
                Reserved22 = 0x16,
                Reserved23 = 0x17,
                Reserved24 = 0x18,
                Reserved25 = 0x19,
                Reserved26 = 0x1A,
                Reserved27 = 0x1B,
                Reserved28 = 0x1C,
                Reserved29 = 0x1D,
                Reserved30 = 0x1E,
                Reserved31 = 0x1F,
                RoadType = 0x20
            };

            static const DWORD BlockCellID = 0x0000FFFF;
            static const int FirstEnvCellID = 0x100;
            static const int LastEnvCellID = 0xFFFD;
            static const int FirstLandCellID = 1;
            static const int LastLandCellID = 64;

            static const DWORD BlockMask = 0xFFFF0000;
            static const WORD BlockX_Mask = 0xFF00;
            static const WORD BlockY_Mask = 0x00FF;
            static const DWORD CellID_Mask = 0x0000FFFF;
            static const int LandblockMask = 7;

            static const int BlockPartShift = 16;
            static const int LandblockShift = 3;
            static const int MaxBlockShift = 8;

            static const int BlockLength = 192;
            static const int CellLength = 24;
            static const int LandLength = 2040;
            static const int VertexDim = 9;

            static const int BlockSide = 8;
            static const int VertexPerCell = 1;
            static const int HalfSquareLength = 12;
            static const int SquareLength = 24;

            // mapping to non-linear land vertex heights
            static std::vector<float> LandHeightTable;

            LandDefs();

			static void _Unpack(cByteStream *reader);

            static void InitFromPortalDat();

            // XXX: TODO: Implement Position class to use this
            // static bool AdjustToOutside(Position pos);

            static bool AdjustToOutside(unsigned int &blockCellID, Vector3 &loc);

            static Vector3* GetBlockOffset(unsigned int cellFrom, unsigned int cellTo);

            static bool InBlock(Vector3 pos, float radius);

            static Vector2* blockid_to_lcoord(unsigned int cellID);
            static Vector2* gid_to_lcoord(DWORD cellID, bool envCheck = true);
            static Vector2* get_outside_lcoord(unsigned int blockCellID, float _x, float _y);

            static bool cell_in_range(unsigned int cellID);
            static int lcoord_to_gid(float _x, float _y);
            static bool inbound_valid_cellid(unsigned int blockCellID);
        };
    }
}
