#include "stdafx.h"
#include "cWorld.h"
#include "cThread.h"
#include "LandDefs.h"
#include "Vector2.h"


using namespace Physics::Common;


cWorld::cWorld()
{
    m_mLandblocks.clear();
    m_mCurrentLandblocks.clear();
    m_mNeedToLoadBlocks.clear();

    LandDefs::InitFromPortalDat();
}

cWorld::~cWorld()
{
    for (std::map<WORD, cLandblock *>::iterator i = m_mLandblocks.begin(); i != m_mLandblocks.end(); i++)
        delete i->second;
}

int cWorld::LoadNeededLandblocks()
{
    //	OutputString(eYellow, "Loading Landblocks...");
    int numLandblocks = 0;

    for (std::unordered_set<WORD>::iterator i = m_mNeedToLoadBlocks.begin(); i != m_mNeedToLoadBlocks.end(); i++)
    {
        cLandblock *pLB = new cLandblock(this);
        pLB->Load(*i);
        m_mLandblocks[*i] = pLB;
        numLandblocks++;
    }

    m_mNeedToLoadBlocks.clear();

    //	OutputString(eYellow, "Done Loading.");
    return numLandblocks;
}

std::unordered_set<WORD>::iterator cWorld::GetIterCurrentLandblocks() {
    return m_mCurrentLandblocks.begin();
}


cLandblock *cWorld::GetNextLandblock(std::unordered_set<WORD>::iterator &i) {
    cLandblock *pLB = NULL;
    if (i == m_mCurrentLandblocks.end()) {
        return NULL;
    }

    if (m_mLandblocks.find(*i) != m_mLandblocks.end())
    {
        pLB = m_mLandblocks.find(*i)->second;
    }
    i++;
    return pLB;
}

int cWorld::LoadLandblocks(DWORD dwCurrentLandblock, int renderRadius) {
    int numLandblocks = 0;
    DWORD LBX = (dwCurrentLandblock >> 24) & 0xFF;
    DWORD LBY = (dwCurrentLandblock >> 16) & 0xFF;

    m_mCurrentLandblocks.clear();
    int Y1 = LBY - renderRadius, Y2 = LBY + renderRadius;
    if (Y1 < 0) Y1 = 0;	if (Y2 > 255) Y2 = 255;
    int X1 = LBX - renderRadius, X2 = LBX + renderRadius;
    if (X1 < 0) X1 = 0;	if (X2 > 255) X2 = 255;
    for (DWORD y = Y1;y <= (DWORD)Y2;y++)
    {
        for (DWORD x = X1;x <= (DWORD)X2;x++)
        {
            WORD wLB = (x << 8) | y;
            m_mCurrentLandblocks.insert(wLB);
            if (m_mLandblocks.find(wLB) == m_mLandblocks.end())
                if (m_mDownloadingLandblocks.find(wLB) == m_mDownloadingLandblocks.end())
                    m_mNeedToLoadBlocks.insert(wLB);
        }
    }

    if (m_mNeedToLoadBlocks.size()) {
        numLandblocks = LoadNeededLandblocks();
    }

    return numLandblocks;

}

cLandblock* cWorld::GetLandblock(DWORD cellID) {
    std::map<WORD, cLandblock*>::iterator i;
    cLandblock *pLB = NULL;

	// landblock ID is the 4 MSBs
	WORD landblockID = cellID >> 16;
    i = m_mLandblocks.find(landblockID);
    if (i == m_mLandblocks.end()) {
        return NULL;
    }

    pLB = i->second;
    return pLB;
}

void cWorld::AddLandblock(cPortalFile *NewLB)
{
    Lock();
    m_Cell->InsertEntry(NewLB);
    m_mNeedToLoadBlocks.insert(NewLB->id >> 16);
    Unlock();
}

// same as get_landcell in ACE
LandblockCell* cWorld::GetLandblockCell(DWORD block_cell_id) {
	//Console.WriteLine($"get_landcell({blockCellID:X8}");

	cLandblock* landblock = GetLandblock(block_cell_id);
	if (landblock == NULL)
		return NULL;

	DWORD cellID = block_cell_id & 0xFFFF;
	LandblockCell* cell = NULL;

	// outdoor cells
	if (cellID < 0x100)
	{
		Vector2* lcoord = LandDefs::gid_to_lcoord(block_cell_id, false);
		if (lcoord == NULL) return NULL;
		unsigned int landCellIdx = ((int)lcoord->y % 8) + ((int)lcoord->x % 8) * landblock->SideCellCount;
		delete lcoord;
		// XXX: TODO: lock the landblock? needs to be concurrency-safe map
		auto it = landblock->m_landcells.find(landCellIdx);
		if (it != landblock->m_landcells.end()) {
			cell = &it->second;
		}
	}
	// indoor cells
	else
	{
		auto it = landblock->m_landcells.find((int)cellID);
		if (it != landblock->m_landcells.end()) {
			cell = &it->second;
		}

		// XXX: TODO: implement me
		//else {
		//	lock(landblock.LandCellMutex)
		//	{
		//		if (landblock.LandCells.TryGetValue((int)cellID, out cell))
		//			return cell;

		//		cell = DBObj.GetEnvCell(blockCellID);
		//		landblock.LandCells.TryAdd((int)cellID, cell);
		//		var envCell = (EnvCell)cell;
		//		envCell.PostInit();
		//	}
		//}
	}
	return cell;
}
