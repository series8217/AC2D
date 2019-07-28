#pragma once

#include "cCharInfo.h"
#include "cWorld.h"
#include "cInterface.h"
#include "cGraphics.h"
#include "cNetwork.h"

class cClient : public cThread {
public:
	cClient(HINSTANCE hInst, HWND hWnd);
	~cClient();
	void Resize();
	void Run();
	void WindowsMessage(UINT Message, WPARAM wParam, LPARAM lParam);
	bool Initted();

private:
	HWND m_hWnd;
	HINSTANCE m_hInst;

	bool m_bInit;

	cCharInfo *m_CharInfo;
	cInterface *m_Interface;
	cGraphics *m_Graphics;
	cNetwork *m_Network;
    cWorld *m_World;

};
