#pragma once

#include "cModelGroup.h"
#include "cByteStream.h"

// a landblock has this many cells squared
#define CELLDIM  8

// a landblock has this many vertices squared
#define VERTEXDIM (CELLDIM+1)

#pragma pack(1)
struct stLandblockCell {
	DWORD dwID;
	DWORD dwObjectBlock;
	WORD wTopo[VERTEXDIM][VERTEXDIM];
	BYTE bZ[VERTEXDIM][VERTEXDIM];
	BYTE bPadding;
};
#pragma pack(4)

class cLandblock
{
public:
	cLandblock();
	~cLandblock();

	void Load(WORD wBlock);
	int Draw();

    float vertexHeights[VERTEXDIM][VERTEXDIM];

private:
	bool getSplitDirectionNESW(DWORD x, DWORD y);
    void LoadObjectBlock(DWORD *dwLB);
    void updateVertexHeights();

	WORD m_wBlock;

	stLandblockCell m_lbCell;
	std::vector<cModelGroup *> Models;
};