#include "stdafx.h"
#include "cWorld.h"
#include "cThread.h"


cWorld::cWorld()
{
    m_mLandblocks.clear();
    m_mCurrentLandblocks.clear();
    m_mNeedToLoadBlocks.clear();
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
        cLandblock *pLB = new cLandblock();
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

void cWorld::AddLandblock(cPortalFile *NewLB)
{
    Lock();
    m_Cell->InsertEntry(NewLB);
    m_mNeedToLoadBlocks.insert(NewLB->id >> 16);
    Unlock();
}

