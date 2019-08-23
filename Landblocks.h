#pragma once

#include "cModelGroup.h"
#include "cByteStream.h"
#include "Polygon.h"
#include "VertexArray.h"
#include "LandDefs.h"

class cWorld;

namespace Physics {
	namespace Common {

		// a landblock has this many cells squared
		#define CELLDIM  8

		// vertices per cell
		#define VERTEX_PER_CELL 1

		// a landblock has this many vertices squared
		#define VERTEXDIM (VERTEX_PER_CELL*CELLDIM+1)

		// triangle polygons per cell
		#define POLY_PER_CELL (VERTEX_PER_CELL*VERTEX_PER_CELL*2)


		class LandblockCell
		{
			public:
				std::vector<Physics::Polygon*> Polygons;
				unsigned int cellID;
				Vector3 Origin;

				LandblockCell() {
					Origin.x = 0;
					Origin.y = 0;
					Origin.z = 0;
					Polygons.reserve(POLY_PER_CELL);
					for (int i = 0; i < POLY_PER_CELL; i++) {
						Polygons.push_back(NULL);
					}
				}

				LandblockCell(unsigned int cell_id) : cellID(cell_id) {
					Origin.x = 0;
					Origin.y = 0;
					Origin.z = 0;
					Polygons.reserve(POLY_PER_CELL);
					for (int i = 0; i < POLY_PER_CELL; i++) {
						Polygons.push_back(NULL);
					}
				}

				bool find_terrain_poly(Vector3 origin, Polygon* &walkable) const;
		};


		#pragma pack(push,1)
		struct stLandblockData {
			DWORD dwID;
			DWORD dwObjectBlock;
			WORD wTopo[VERTEXDIM][VERTEXDIM];
			BYTE bZ[VERTEXDIM][VERTEXDIM];
			BYTE bPadding;
		};
		#pragma pack(pop)

		class cLandblock
		{
		public:
			cLandblock(cWorld* world);
			~cLandblock();

			void Load(WORD wBlock);
			bool Generate(unsigned int landblockID, int cellScale, LandDefs::Direction transAdj);
			void InitPVArrays();
			void Destroy();

			int Draw();

			float GetZ(Vector3 point);
			LandblockCell* GetCell(Vector3 point);
			bool findTerrainPoly(Vector3 origin, Physics::Polygon* &walkable);
			void ConstructVertices();
			void ConstructPolygons(unsigned int landblockID);
			void TransAdjust();
			Physics::Polygon* AddPolygon(int polyIdx, int _v0, int _v1, int _v2);

			float vertexHeights[VERTEXDIM][VERTEXDIM];
			std::vector<char> Height;
			std::map<DWORD, LandblockCell> m_landcells;

			unsigned short SideVertexCount;
			unsigned short SideCellCount;
			unsigned short SidePolyCount;

			cWorld* World;
			LandDefs::Direction TransDir;
			LandDefs::WaterType WaterType;

		private:
			bool getSplitDirectionNESW(DWORD x, DWORD y);
			void LoadObjectBlock(DWORD *dwLB);
			void updateVertexHeights();
			void AdjustPlanes();

			DWORD ID;
			WORD m_wBlock;

			stLandblockData m_lbData;
			std::vector<cModelGroup *> Models;
			std::vector<Physics::Polygon> Polygons;
			VertexArray m_vertex_array;
			bool SWtoNEcut[CELLDIM][CELLDIM];
		};


	}
}
