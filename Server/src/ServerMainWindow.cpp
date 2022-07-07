#include "ServerMainWindow.h"

#include "utils.h"


static constexpr COLORREF terminalGreenColor = RGB(0x13, 0xa1, 0x0e);
static constexpr COLORREF terminalRedColor = RGB(0xc5, 0x0f, 0x1f);


ServerMainWindow::ServerMainWindow(PCSTR ip, PCSTR port) : 
	server{ ip, port }
{

}

void ServerMainWindow::Send()
{
	LRESULT length = SendMessage(hWndMessageEnter, WM_GETTEXTLENGTH, NULL, NULL) + 1;

	wchar_t* message = new wchar_t[length];

	SendMessage(hWndMessageEnter, WM_GETTEXT, length, (LPARAM)message);

	DisplayOnScreen(Format(L"SERVER", message).c_str());

	HEADER header = ConstructHeader(HeaderType::BROADCAST,
		static_cast<uint32_t>(7),
		static_cast<uint32_t>(length), true);

	server.BroadCastMessage(INVALID_SOCKET, header, L"SERVER", message);

	delete[] message;

	SendMessage(hWndMessageEnter, WM_SETTEXT, NULL, (LPARAM)TEXT(""));
}

LRESULT ServerMainWindow::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	server.SetHwnd(hWnd);
	server.StartListening();

	return MainWindow::OnCreate(hWnd, wParam, lParam);
}

LRESULT ServerMainWindow::OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	server.StopServer();
	PostQuitMessage(0);
	return 0;
}

LRESULT ServerMainWindow::OnMessagePack(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MESSAGEPACK* pack = reinterpret_cast<MESSAGEPACK*>(lParam);

	COLORREF color = NO_COLORING;

	if (pack->type == HeaderType::USER_CONNECTED)
	{
		color = terminalGreenColor;
	}
	else if (pack->type == HeaderType::USER_DISCONNECTED)
	{
		color = terminalRedColor;
	}

	DisplayOnScreen(pack->message.c_str(), color);

	delete pack;

	return 0;
}

LRESULT ServerMainWindow::OnClientCountChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	auto clientCount = server.GetClientCount();

	std::wstring name{ L"SERVER | " };
	std::wstring countStr = std::to_wstring(clientCount);

	SendMessage(hWnd, WM_SETTEXT, NULL, (LPARAM)(name + countStr + L" CLIENT(S) CONNECTED").c_str());
	TitleBarLibrary::UpdateNCArea(hWnd);

	return 0;
}
