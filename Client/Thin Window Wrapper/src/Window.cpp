#include "pch.h"

#include "Window.h"
#include "WindowClass.h"


WindowWrapper::WindowBase* GetWindowInstanceFromHandle(HWND hWnd)
{
	return reinterpret_cast<WindowWrapper::WindowBase*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

LRESULT WindowWrapper::WindowBase::_WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		auto createParams = reinterpret_cast<LPCREATESTRUCT>(lParam);
		WindowBase* window = reinterpret_cast<WindowBase*>(createParams->lpCreateParams);

		window->hWnd = hWnd;

		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}

	auto window = GetWindowInstanceFromHandle(hWnd);
	if (window)
	{
		return window->WindowProcedure(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND WindowWrapper::WindowBase::Create(WindowBase* window, WindowClass windowClass, 
	const TCHAR* title, POINT position, SIZE size, 
	DWORD style, HWND parent, DWORD exStyle, HMENU menu)
{
	if (!windowClass.IsRegistered())
	{
		windowClass.Register();
	}

	return CreateWindowEx(exStyle, windowClass.GetName().c_str(), title, style, 
		position.x, position.y, size.cx, size.cy, 
		parent, menu, GetModuleHandle(NULL), window);
}

BOOL WindowWrapper::WindowBase::Show(bool show)
{
	return ShowWindow(hWnd, show ? SW_SHOW : SW_HIDE);
}
