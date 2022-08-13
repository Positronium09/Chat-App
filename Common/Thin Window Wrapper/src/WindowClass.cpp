#include "pch.h"
#include "WindowClass.h"
#include "Window.h"

#include <string>


WindowWrapper::WindowClass::WindowClass(WNDCLASSEX p_wndClass) : 
	className(p_wndClass.lpszClassName)
{
	wndClass = p_wndClass;
	wndClass.lpfnWndProc = nullptr;
}

WindowWrapper::WindowClass::WindowClass(const TCHAR* windowClassName, UINT style, HCURSOR cursor, HICON icon, HICON smallIcon) : 
	className(windowClassName)
{
	if (cursor == NULL)
	{
		cursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
	}

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.style = style;
	wndClass.hIcon = icon;
	wndClass.hIconSm = smallIcon;
	wndClass.hCursor = cursor;
	wndClass.hbrBackground = GetStockBrush(BLACK_BRUSH);
}

WindowWrapper::WindowClass::~WindowClass()
{
	if (isRegistered)
	{
		UnRegister();
	}
}

WindowWrapper::WindowClass WindowWrapper::WindowClass::operator=(WNDCLASSEX p_wndClass)
{
	return WindowClass(p_wndClass);
}

ATOM WindowWrapper::WindowClass::Register()
{
	wndClass.lpfnWndProc = WindowBase::_WindowProcedure;
	wndClass.lpszClassName = className.c_str();
	return RegisterClassEx(&wndClass);
}

void WindowWrapper::WindowClass::UnRegister()
{
	UnregisterClass(className.c_str(), GetModuleHandle(NULL));
}

const std::string& WindowWrapper::WindowClass::GetName()
{
	return className;
}

bool WindowWrapper::WindowClass::IsRegistered()
{
	return isRegistered;
}
