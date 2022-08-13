#pragma once

#include "pch.h"


#define BEGIN_PROC() \
virtual LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override {

#define BEGIN_MAP() \
switch (msg) \
	{

#define BEGIN_PROC_MAP() BEGIN_PROC() BEGIN_MAP()

#define REGISTER_HANDLER(msg, handler) \
case msg: \
{ \
	return handler(hWnd, wParam, lParam); \
}

#define END_MAP() }
#define END_PROC() \
	return DefWindowProc(hWnd, msg, wParam, lParam); \
}
#define END_MAP_PROC() END_MAP() END_PROC()

namespace WindowWrapper
{
	class WindowClass;

	class WindowBase
	{
		friend class WindowClass;

		private:
		static LRESULT CALLBACK _WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		protected:
		HWND hWnd = NULL;

		virtual LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

		public:
		WindowBase() = default;
		static HWND Create(WindowBase* window, WindowClass windowClass, 
			const TCHAR* title, POINT position, SIZE size, 
			DWORD style = WS_OVERLAPPEDWINDOW, HWND parent = NULL, DWORD exStyle = NULL, HMENU menu = NULL);

		WindowBase(WindowBase& other) = delete;
		WindowBase& operator=(WindowBase& other) = delete;
		operator HWND() { return hWnd; }
		
		virtual ~WindowBase() = default;

		BOOL Show(bool show);
	};
}
