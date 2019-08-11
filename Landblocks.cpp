#include "stdafx.h"
#include "Landblocks.h"

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

cLandblock::cLandblock()
{
	m_wBlock = 0;
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

	//First load the FFFF
	cPortalFile *portalFileLandblockCell = m_Cell->OpenEntry(dwLB | 0xFFFF);
	if (!portalFileLandblockCell)
	{
		// TODO: Add request mechanism here...
		ZeroMemory(&m_lbCell, sizeof(stLandblockCell));
		return;
	}
	
	//Fill in landblock cell structure
	memcpy(&m_lbCell, portalFileLandblockCell->data, sizeof(stLandblockCell));

    // calculate vertices
    updateVertexHeights();

	//If applicable, load object block
	if (m_lbCell.dwObjectBlock)	//1 for block, 0 for none
	{
        LoadObjectBlock(&dwLB);
	}
}

void cLandblock::updateVertexHeights() {

    for (int y = 0;y < VERTEXDIM; y++)
    {
        for (int x = 0;x < VERTEXDIM; x++)
        {
            // TODO: look up and apply scaling factor in land height table from portal dat
            vertexHeights[x][y] = m_lbCell.bZ[x][y];
        }
    }
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
        tpld.xOffset = pBS2.ReadFloat();
        tpld.yOffset = pBS2.ReadFloat();
        tpld.zOffset = pBS2.ReadFloat();
        tpld.wHeading = pBS2.ReadFloat();
        tpld.aHeading = pBS2.ReadFloat();
        tpld.bHeading = pBS2.ReadFloat();
        tpld.cHeading = pBS2.ReadFloat();
        tpld.landblock = *dwLB;
        cPoint3D tp3d;
        tp3d.CalcFromLocation(&tpld);

        cModelGroup *DModel = new cModelGroup();
        if (!DModel->ReadDungeon(DungeonRef, DungeonPart, &vTexes))
        {
            return;
        }
        DModel->SetScale(MODEL_SCALE_FACTOR);
        DModel->SetTranslation(tp3d);
        DModel->SetRotation(tpld.wHeading, tpld.aHeading, tpld.bHeading, tpld.cHeading);
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
                tpl.xOffset = pBS2.ReadFloat();
                tpl.yOffset = pBS2.ReadFloat();
                tpl.zOffset = pBS2.ReadFloat();
                tpl.wHeading = pBS2.ReadFloat();
                tpl.aHeading = pBS2.ReadFloat();
                tpl.bHeading = pBS2.ReadFloat();
                tpl.cHeading = pBS2.ReadFloat();
                tpl.landblock = *dwLB;
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
                Model->SetRotation(tpl.wHeading, tpl.aHeading, tpl.bHeading, tpl.cHeading);
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
        tpl.xOffset = pBS.ReadFloat();
        tpl.yOffset = pBS.ReadFloat();
        tpl.zOffset = pBS.ReadFloat();
        tpl.wHeading = pBS.ReadFloat();
        tpl.aHeading = pBS.ReadFloat();
        tpl.bHeading = pBS.ReadFloat();
        tpl.cHeading = pBS.ReadFloat();
        tpl.landblock = *dwLB;
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
        Model->SetRotation(tpl.wHeading, tpl.aHeading, tpl.bHeading, tpl.cHeading);
        Models.push_back(Model);
    }

    WORD wNumSkins = pBS.ReadWORD();
    WORD wSkinUnknown = pBS.ReadWORD();
    for (int j = 0; j < wNumSkins; j++)
    {
        DWORD dwModelID = pBS.ReadDWORD();
        stLocation tpl;
        tpl.xOffset = pBS.ReadFloat();
        tpl.yOffset = pBS.ReadFloat();
        tpl.zOffset = pBS.ReadFloat();
        tpl.wHeading = pBS.ReadFloat();
        tpl.aHeading = pBS.ReadFloat();
        tpl.bHeading = pBS.ReadFloat();
        tpl.cHeading = pBS.ReadFloat();
        tpl.landblock = *dwLB;
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
        Model->SetRotation(tpl.wHeading, tpl.aHeading, tpl.bHeading, tpl.cHeading);
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
    DWORD dwBlockX = (m_lbCell.dwID & 0xff000000) >> 24;
    DWORD dwBlockY = (m_lbCell.dwID & 0x00ff0000) >> 16;

    DWORD x = (dwBlockX * 8) | cellX;
    DWORD y = (dwBlockY * 8) | cellY;

    /** Determines the split line direction for a cell triangulation */
	DWORD dw = x * y * 0x0CCAC033 - x * 0x421BE3BD + y * 0x6C1AC587 - 0x519B8F25;
    /* returns true if NE-SW split, FALSE if NW-SE split */
	return (dw & 0x80000000) != 0;
}

int cLandblock::Draw()
{
	/*

	Landblock notes:
	Landblocks are XXYY____
	x is EW
	y is NS
	Landblocks start at (0,0) = SW corner

	*/
	//Draw the FFFF
	float fXCorner, fYCorner;
	DWORD dwBlockX = (m_lbCell.dwID & 0xff000000) >> 24;
	DWORD dwBlockY = (m_lbCell.dwID & 0x00ff0000) >> 16;

/*	if (dwBlockX < 3)
	{
		// dungeon, dwBlockX and dwBlockY remain constant - just use x and y
		fXCorner = 0;
		fYCorner = 0;
	}
	else*/
	{
		// outdoors
		fXCorner = (float) ((((dwBlockX+1.00f) * 8.0f) - 1027.5) / 10.0f);
		fYCorner = (float) ((((dwBlockY+1.00f) * 8.0f) - 1027.5) / 10.0f);
	}

	float fDiv = (1.0f/ MODEL_SCALE_FACTOR) / 2.0f;
    glPushName(0xDEADBEEF);

	for (int y=0;y<CELLDIM;y++)
	{
		for (int x=0;x<CELLDIM;x++)
		{
			int type0;
            if ((m_lbCell.wTopo[x][y] & 0x0003)) {
                type0 = 32;
            }
            else {
                type0 = (m_lbCell.wTopo[x][y] & 0x00FF) >> 2;
            }

			bool NESW = getSplitDirectionNESW(x, y);

			int tex1 = m_Portal->FindTexturePalette(texnum[type0], 0);

			glBindTexture( GL_TEXTURE_2D, tex1);

			glBegin(GL_TRIANGLE_FAN);
			glColor4f(1,1,1,1.0f);
			if (NESW)
			{
                glTexCoord2f(0,0);
				glVertex3f(fXCorner + x/10.0f, fYCorner + y/10.0f, vertexHeights[x][y]/fDiv);

				glTexCoord2f(2,0);
				glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + y/10.0f, vertexHeights[x+1][y]/fDiv);

				glTexCoord2f(2,2);
				glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x+1][y+1]/fDiv);

				glTexCoord2f(0,2);
				glVertex3f(fXCorner + x/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x][y+1]/fDiv);
			}
			else
			{ // NW-SE
				glTexCoord2f(2,0);
				glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + y/10.0f, vertexHeights[x+1][y]/fDiv);

				glTexCoord2f(2,2);
				glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x+1][y+1]/fDiv);

				glTexCoord2f(0,2);
				glVertex3f(fXCorner + x/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x][y+1]/fDiv);

				glTexCoord2f(0,0);
				glVertex3f(fXCorner + x/10.0f, fYCorner + y/10.0f, vertexHeights[x][y]/fDiv);
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
			glVertex3f(fXCorner + x/10.0f, fYCorner + y/10.0f, vertexHeights[x][y]/fDiv);

			glColor4ub(landColor[type1][0], landColor[type1][1], landColor[type1][2], 255);
			glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + y/10.0f, vertexHeights[x+1][y]/fDiv);

			glColor4ub(landColor[type2][0], landColor[type2][1], landColor[type2][2], 255);
			glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x+1][y+1]/fDiv);

			glColor4ub(landColor[type3][0], landColor[type3][1], landColor[type3][2], 255);
			glVertex3f(fXCorner + x/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x][y+1]/fDiv);

			glEnd();

			glBegin(GL_LINE_STRIP);

			if (NESW)
			{
			glColor4ub(landColor[type0][0], landColor[type0][1], landColor[type0][2], 255);
			glVertex3f(fXCorner + x/10.0f, fYCorner + y/10.0f, vertexHeights[x][y]/fDiv);
			glColor4ub(landColor[type2][0], landColor[type2][1], landColor[type2][2], 255);
			glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x+1][y+1]/fDiv);
			}
			else
			{
			glColor4ub(landColor[type1][0], landColor[type1][1], landColor[type1][2], 255);
			glVertex3f(fXCorner + (x+1)/10.0f, fYCorner + y/10.0f, vertexHeights[x+1][y]/fDiv);
			glColor4ub(landColor[type3][0], landColor[type3][1], landColor[type3][2], 255);
			glVertex3f(fXCorner + x/10.0f, fYCorner + (y+1)/10.0f, vertexHeights[x][y+1]/fDiv);
			}

			glEnd();*/

/*			glBindTexture( GL_TEXTURE_2D, 0);
			if (type0 == 32)
			{
				glColor3f(1.0,1.0,1.0);
				glRasterPos3f(fXCorner + x/10.0f, fYCorner + y/10.0f, vertexHeights[x][y]/fDiv);

				char lele[50];
				snprintf(lele, 50, "%02X - %08X - %04X", type0, texnum[type0], m_lbCell.wTopo[x][y]);
				glCallLists(strlen(lele), GL_UNSIGNED_BYTE, lele); 
			}*/