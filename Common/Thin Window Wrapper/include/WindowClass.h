#pragma once

#include "pch.h"

#include <string>

namespace WindowWrapper
{
	#ifdef UNICODE
	#define string wstring
	#else
	#define string string
	#endif

	class WindowBase;

	class WindowClass
	{
		friend class WindowBase;

		private:
		WNDCLASSEX wndClass{ };
		std::string className;
		bool isRegistered = false;

		public:
		WindowClass(WNDCLASSEX wndClass);
		WindowClass(const TCHAR* windowClassName, UINT style, HCURSOR cursor = NULL, HICON icon = NULL, HICON smallIcon = NULL);
		~WindowClass();

		WindowClass operator=(WNDCLASSEX wndClass);

		ATOM Register();
		void UnRegister();

		const std::string& GetName();
		bool IsRegistered();
	};
}
