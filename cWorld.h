#pragma once
#include "cThread.h"
#include "cObjectDB.h"
#include "cCellManager.h"
#include "Landblocks.h"
#include "cPoint3D.h"

class cNetwork;
class cInterface;


class cWorld: public cObjectDB {
public:
    cWorld();
    ~cWorld();
    void SetNetwork(cNetwork *Network);
    void SetObjectDB(cObjectDB *ObjectDB);
    void SetInterface(cInterface *Interface);

    void AddLandblock(cPortalFile *NewLB);

    int LoadNeededLandblocks();
    int LoadLandblocks(DWORD dwCurrentLandblock, int renderRadius);
    bool FindLandblocks(FILE *cell, DWORD dirPos);

    std::unordered_set<WORD>::iterator GetIterCurrentLandblocks();
    cLandblock* GetNextLandblock(std::unordered_set<WORD>::iterator &i);

private:
    std::map<WORD, cLandblock *> m_mLandblocks;
    std::unordered_set<WORD> m_mCurrentLandblocks, m_mNeedToLoadBlocks, m_mDownloadingLandblocks;

    LARGE_INTEGER liFreq, liLast;

    // time that we last sent a location update
    DWORD m_dwLastLocationUpdate;
};

#include "cNetwork.h"