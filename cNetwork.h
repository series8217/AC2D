#pragma once

#include "cPacket.h"
#include "cMessage.h"
#include "cInterface.h"
#include "cThread.h"
#include "cWorld.h"
#include "cCharInfo.h"
#include "ChecksumXorGenerator.h"

//Speed Consts
const float sidestep_factor			= 0.5f;
const float backwards_factor		= 0.64999998f;
const float run_turn_factor			= 1.5f;
const float run_anim_speed			= 4.0f;
const float walk_anim_speed			= 3.1199999f;
const float sidestep_anim_speed		= 1.25f;
const float max_sidestep_anim_rate	= 3.0f;

//Server flags
#define SF_CONNECTED (1 << 0)
#define SF_CRCSEEDS (1 << 2)

// timeout and interval settings
// how long we wait for a packet to arrive before servicing other network stuff
#define TIMEOUT_PACKET_POLL_MS 100
// Connection attempt timeout
#define TIMEOUT_CONNECT_MS  5000
// enter world timeout
#define TIMEOUT_ENTER_GAME_MS 5000
// timeout after connection established
#define TIMEOUT_SERVER_MS 10000
// Interval between ack messages sent by us
#define ACK_INTERVAL_MS 2000
// Interval between player location updates sent client to server
#define LOCATION_UPDATE_INTERVAL_MS 1000

//enum OptionalHeaderFlags
//{
//    kDisposable = 0x00000001, // this header may be removed from a retransmission
//    kExclusive = 0x00000002, // a packet with this header has its own sequence number
//    kNotConn = 0x00000004, // this header is sent before connect request/reply handshake completes
//    kTimeSensitive = 0x00000008,
//    kShouldPiggyBack = 0x00000010, // this header should ride along in a packet with others headers and content
//    kHighPriority = 0x00000020,
//    kCountsAsTouch = 0x00000040,
//    kEncrypted = 0x20000000, // a packet with this header has its checksum encrypted
//    kSigned = 0x40000000
//};


/* server authentication type */
enum NetAuthType
{
	kAuthUndef = 0x00000000,
	kAuthAccount = 0x00000001,
	kAuthAccountPassword = 0x00000002,
	kAuthGlsTicket = 0x40000002
};


enum PacketFlags
{
    kRetransmission = 0x00000001,
    kEncryptedChecksum = 0x00000002,
    kBlobFragments = 0x00000004, // Packet contains a fragment of a larger blob. All world packets are blob fragments.
    kServerSwitch = 0x00000100, // CServerSwitchStruct (60, kHighPriority|kCountsAsTouch)
    kLogonServerAddr = 0x00000200, // CLogonRouteHeader (sockaddr_in) (7, kDisposable|kExclusive|kNotConn)
    kEmptyHeader1 = 0x00000400, // EmptyHeader (7, kDisposable|kExclusive|kNotConn)
    kReferral = 0x00000800, // CReferralStruct (40000062, kExclusive|kHighPriority|kCountsAsTouch|kSigned)
    kRequestRetransmit = 0x00001000, // SeqIDList (33, kDisposable|kExclusive|kShouldPiggyBack|kHighPriority)
    kRejectRetransmit = 0x00002000, // SeqIDList (33, kDisposable|kExclusive|kShouldPiggyBack|kHighPriority)
    kAckSequence = 0x00004000, // CPakHeader (unsigned long) (1, kDisposable)
    kDisconnect = 0x00008000, // EmptyHeader (3, kDisposable|kExclusive)
    kLogon = 0x00010000, // CLogonHeader (?)
    kReferred = 0x00020000, // uint64_t (7, kDisposable|kExclusive|kNotConn)
    kConnectRequest = 0x00040000, // CConnectHeader (?)
    kConnectResponse = 0x00080000, // uint64_t (20000007, kDisposable|kExclusive|kNotConn|kEncrypted)
    kNetError1 = 0x00100000, // PackObjHeader<NetError> (7, kDisposable|kExclusive|kNotConn)
    kNetError2 = 0x00200000, // PackObjHeader<NetError> (2, kExclusive)
    kCICMDCommand = 0x00400000, // CICMDCommandStruct (7, kDisposable|kExclusive|kNotConn)
    kTimeSync = 0x01000000, // CTimeSyncHeader (?)
    kEchoRequest = 0x02000000, // CEchoRequestHeader (?)
    kEchoResponse = 0x04000000, // CEchoResponseHeader (?)
    kFlow = 0x08000000  // CFlowStruct (10, kShouldPiggyBack)
};


enum RawMotionFlags
{
    // flags for RawMotionState packet 0xF61C
    // below types are the data types that follow the 4-byte packed flags DWORD.
    // after the flags DWORD, each value for the flags that are set must follow in
    // the same order as below.
    kInvalid = 0x0,
    kCurrentHoldKey = 0x1, // uint32 - walk or run.    0: Invalid, 1: None, 2: Run
    kCurrentStyle = 0x2,   // uint32 - current stance - MotionStance
    kForwardCommand = 0x4, // uint32 - forward move or motion command - MotionCommand
    kForwardHoldKey = 0x8, // uint32 - whether forward/back key being held - 0: Invalid, 1: None, 2: Run
    kForwardSpeed = 0x10,  // single float
    kSideStepCommand = 0x20, // uint32 - sidestep movement command - MotionCommand
    kSideStepHoldKey = 0x40, // uint32 - indicates whether a sidestep key is being held - 0: Invalid, 1: None, 2: Run
    kSideStepSpeed = 0x80, // single float
    kTurnCommand = 0x100, // uint32 - always sent as 1 direction in RawMotion with negative speed for opposite direction.- MotionCommand
    kTurnHoldKey = 0x200, // uint32 -- whether turn key is being held, or mouselook in progress
    kTurnSpeed = 0x400 // single float -- turn movement speed
};

struct stServerInfo {
	SOCKADDR_IN m_saServer;
	std::list <cPacket *> m_lSentPackets;
	std::list <cMessage *> m_lIncomingMessages;

	DWORD	m_dwSendSequence;
	WORD	m_wLogicalID;
	WORD	m_wTable;

    WORD    m_wBasePort;

    QWORD   m_qwCookie;

	//Tracking our received packets
    // Last received packet sequence number in sequence (does not increment if out of sequence)
	DWORD	m_dwRecvSequence;

	//Logical
	DWORD	m_dwConnectAttempts;

	//Time based stuff
    DWORD   m_dwLastPacketAck;
	DWORD	m_dwLastConnectAttempt;
	DWORD	m_dwLastPacketSent;
	DWORD	m_dwLastPing;
	DWORD	m_dwLastSyncSent;
	DWORD	m_dwLastSyncRecv;
    double	m_flServerTime; //At our last sync, this was the server's time

	//Flags to determine our status/phase
	DWORD	m_dwFlags;

    // the rng used to generate xor values for server packets
    ChecksumXorGenerator *serverXorGen;

    // the rng used to generate xor values for client packets
    ChecksumXorGenerator *clientXorGen;
};

class cNetwork : public cThread {
public:
	cNetwork();
	~cNetwork();

	void SetInterface(cInterface *Interface);
	void SetWorld(cWorld *World);
	void SetCharInfo(cCharInfo *CharInfo);

	void SendLSGameEvent(cPacket *Packet, WORD wGroup);
	void SendWSGameEvent(cPacket *Packet, WORD wGroup);
    void SendLSMessage(cPacket *Packet, WORD wGroup);
	void SendWSMessage(cPacket *Packet, WORD wGroup);
	void SendLSPacket(cPacket *Packet, bool IncludeSeq, bool IncrementSeq);
	void SendPacket(cPacket *Packet, stServerInfo *Target, bool IncludeSeq, bool IncrementSeq);
	void SendLostPacket(int iSendSequence, stServerInfo *Target);
	void SendPacket(cPacket *Packet, stServerInfo *Target);
    
    void SendAckPacket(stServerInfo *Server);

    void ReceivePacket();
    void ProcessFragment(cByteStream* stream, stServerInfo *Server);
	void ProcessPacket(cPacket *Packet, stServerInfo *Server);
	void ProcessWSPacket(cPacket *Packet, stServerInfo *Server);
	void ProcessMessage(cMessage *Msg, stServerInfo *Server);

	void ServerLoginError(DWORD Error);
	void ServerCharacterError(DWORD Error);
	void ServerCharCreateError(DWORD Error);

	void Run();
	void Stop();

	void CheckPings();
	void PingServer(stServerInfo *Server);
	void SyncServer(stServerInfo *Server);

    void SendDDDInterrogationResponse();
    void SendDDDEndMessage();
	void SendEnterWorldRequest(DWORD GUID);
    void SendEnterWorldMessage(DWORD GUID, char *account);
	void DownloadLandblock(DWORD Landblock);
	void SendPositionUpdate(stLocation *Location, stMoveInfo *MoveInfo);
	void SendMoveUpdate(int iFB, int iStrafe, int iTurn, bool bRunning);
	void SetCombatMode(bool CombatMode);
	void CastSpell(DWORD Target, DWORD Spell);
	void SendAllegianceRecall();
	void SendHouseRecall();
	void SendLifestoneRecall();
	void SendMarketplaceRecall();
	void SendPublicMessage(std::string & Message);
	void SendTell(std::string & Name, std::string & Message);
	void SendTell(DWORD dwGUID, std::string & Message);
	void UseItem(DWORD Item, DWORD Target = 0);
	void SendHouseInfoQuery();
	void RequestAllegianceUpdate();

	void Reset();
	void Connect();
	void Disconnect();
	void CloseConnection(stServerInfo *Server);

    void SendLoginPacket();

	stServerInfo * AddWorldServer(SOCKADDR_IN NewServer);
	void SetActiveWorldServer(SOCKADDR_IN NewServer);
    void DumpWorldServerList();

	void SendMaterialize();

private:
    void SendConnectResponse();
	
	cInterface *m_Interface;
	cWorld *m_World;
	cCharInfo *m_CharInfo;

	WORD GetTime();

	SOCKET m_sSocket;
    FD_SET m_readfds;
    FD_SET m_masterfds;
    struct timeval m_socketTimeout;

	DWORD m_dwStartTicks;
	int iConnPacketCount;

	DWORD m_dwFragmentSequenceOut;
	DWORD m_dwGameEventOut;

    DWORD m_dwNextConnectionTimeout;
    DWORD m_dwNextServerAliveTimeout;

	stServerInfo m_siLoginServer;
	std::list <stServerInfo> m_siWorldServers;
	stServerInfo * m_pActiveWorld;

	BYTE m_zTicketSize;
	BYTE m_zTicket[0x100];

#define TICKETKEY_SIZESIZE 100
#define ACCOUNTNAME_SIZE 40
#define PASSWORD_SIZE 100

	char m_zTicketKey[TICKETKEY_SIZESIZE];
	char m_zAccountName[ACCOUNTNAME_SIZE];
	char m_zPassword[PASSWORD_SIZE];

    DWORD m_dwGUIDLogin;

	WORD f74ccount;

	bool bPortalMode;

	// Cache Tell ids
	std::map< std::string, DWORD >		m_treeNameIDCache;
};

