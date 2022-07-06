#pragma once

#include "pch.h"

#include "Direct2DResources.h"
#include "DirectWriteResources.h"


namespace TitleBarLibrary
{
	HRESULT InitializeTitleBarLibrary();

	namespace TitleBarDefaults
	{
		// Auto sizez buttons height to titlebars height
		constexpr LONG AUTO_BUTTON_HEIGHT = -1;

		constexpr LONG DEFAULT_TITLEBAR_HEIGHT = 32;
		constexpr LONG DEFAULT_BUTTON_WIDTH = 46;
		constexpr LONG DEFAULT_BUTTON_HEIGHT = AUTO_BUTTON_HEIGHT;
		constexpr LONG DEFAULT_FRAME_THICKNESS = 3;
		constexpr LONG DEFAULT_SIZINGBORDER_THICKNESS = DEFAULT_FRAME_THICKNESS;

		constexpr bool DEFAULT_SHOWICON = true;

		constexpr SIZE DEFAULT_BUTTON_SIZE = SIZE{ DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT };
		constexpr MARGINS DEFAULT_FRAME_MARGINS = MARGINS{ DEFAULT_FRAME_THICKNESS, DEFAULT_FRAME_THICKNESS, DEFAULT_FRAME_THICKNESS, DEFAULT_FRAME_THICKNESS };
		constexpr MARGINS DEFAULT_SIZINGBORDER_MARGINS = DEFAULT_FRAME_MARGINS;
	}

	constexpr bool _SCREEN_COORDS = true;
	constexpr bool _CLIENT_COORDS = false;

	constexpr bool HANDLED = true;
	constexpr bool UNHANDLED = false;

	constexpr int CloseButton = 1;
	constexpr int MaximizeButton = 2;
	constexpr int MinimizeButton = 4;
	constexpr int AllButtons = CloseButton | MaximizeButton | MinimizeButton;

	constexpr int _BS_Default = 0;
	constexpr int _BS_Hover = 1;
	constexpr int _BS_Pressed = 2;
	constexpr int _BS_Disabled = 3;

	typedef struct _buttonColors
	{
		COLORREF defaultForegroundColor;
		COLORREF defaultBackgroundColor;

		COLORREF hoverForegroundColor;
		COLORREF hoverBackgroundColor;

		COLORREF disabledForegroundColor;
		COLORREF disabledBackgroundColor;

		COLORREF pressedForegroundColor;
		COLORREF pressedBackgroundColor;

		COLORREF noFocusForegroundColor;
		COLORREF noFocusBackgroundColor;

		COLORREF noFocusHoverForegroundColor;
		COLORREF noFocusHoverBackgroundColor;

		COLORREF noFocusDisabledForegroundColor;
		COLORREF noFocusDisabledBackgroundColor;
	} ButtonColors;
	typedef struct _titleBarColors
	{
		COLORREF defaultFrameColor;
		COLORREF defaultBackgroundColor;
		COLORREF defaultTextColor;

		COLORREF noFocusFrameColor;
		COLORREF noFocusBackgroundColor;
		COLORREF noFocusTextColor;

		ButtonColors closeButtonColors;
		ButtonColors maximizeButtonColors;
		ButtonColors minimizeButtonColors;
	} TitleBarColors;

	typedef struct _titleBarMetrics
	{
		LONG height = TitleBarDefaults::DEFAULT_TITLEBAR_HEIGHT;

		SIZE closeButtonSize = TitleBarDefaults::DEFAULT_BUTTON_SIZE;
		SIZE maximizeButtonSize = TitleBarDefaults::DEFAULT_BUTTON_SIZE;
		SIZE minimizeButtonSize = TitleBarDefaults::DEFAULT_BUTTON_SIZE;

		// Visual frame
		MARGINS frameMargins = TitleBarDefaults::DEFAULT_FRAME_MARGINS;
		// Sizing borders
		MARGINS sizingMargins = TitleBarDefaults::DEFAULT_SIZINGBORDER_MARGINS;
	} TitleBarMetrics;

	typedef struct _titleBarParams
	{
		TitleBarMetrics metrics;

		int enabledButtons = AllButtons;

		bool showIcon = true;

		const WCHAR* fontFamilyName = L"Segoe UI";
	} TitleBarParams;

	//TODO Implement menus
	class TitleBar
	{
		friend bool TitleBarHandleMessage(HWND hWnd, UINT msg, WPARAM& wParam, LPARAM& lParam, TitleBar& titleBar, LRESULT& lResult);

		private:
		Direct2DResources direct2dResource;
		DirectWriteResources directWriteResource;

		TitleBarColors colors;

		TitleBarMetrics metrics;

		HWND _hWnd = NULL;

		bool showIcon;
		bool hasFocus = true;

		int closeButtonState = _BS_Default;
		int maximizeButtonState = _BS_Default;
		int minimizeButtonState = _BS_Default;

		RECT CalculateTitleBarRect(HWND hWnd, bool screenCoords);
		RECT CalculateCloseButtonRect(HWND hWnd, bool screenCoords);
		RECT CalculateMaximizeButtonRect(HWND hWnd, bool screenCoords);
		RECT CalculateMinimizeButtonRect(HWND hWnd, bool screenCoords);
		RECT CalculateIconRect(HWND hWnd, bool screenCoords);
		RECT CalculateTextRect(HWND hWnd, bool screenCoords);

		bool UpdateButtonState(WPARAM wParam, int state);

		// Helper for NCPaint
		void DrawCloseButton(HWND hWnd);
		// Helper for NCPaint
		void DrawMaximizeButton(HWND hWnd);
		// Helper for NCPaint
		void DrawMinimizeButton(HWND hWnd);
		// Helper for NCPaint
		void DrawIcon(HWND hWnd);
		// Helper for NCPaint
		void DrawWindowText(HWND hWnd);

		LRESULT OnNCPaint(HWND hWnd, HRGN hrgn);
		LRESULT OnNCHittest(HWND hWnd, POINT p);

		public:
		TitleBar(const TitleBarParams& titleBarParams);
		TitleBar(const TitleBarColors& colors, const TitleBarParams& titleBarParams);

		~TitleBar();

		void EnableButtons(int enabledButtons);
		void SetShowIcon(bool showIcon);

		void SetTitleBarColors(const TitleBarColors& titleBarColors);
		
		void SetHeight(LONG height);

		void SetFrameMargins(MARGINS margins);
		void SetSizingMargins(MARGINS margins);

		void SetCloseButtonSize(SIZE size);
		void SetMaximizeButtonSize(SIZE size);
		void SetMinimizeButtonSize(SIZE size);

		void SetTitleBarMetrics(const TitleBarMetrics& titleBarMetrics);

		bool GetShowIcon() const;

		TitleBarColors& GetTitleBarColors();
		const TitleBarColors& GetTitleBarColors() const;

		TitleBarMetrics& GetTitleBarMetrics();
		const TitleBarMetrics& GetTitleBarMetrics() const;

		LONG GetHeight() const;

		MARGINS GetFrameMargins() const;
		MARGINS GetSizingMargins() const;

		SIZE GetCloseButtonSize() const;
		SIZE GetMaximizeButtonSize() const;
		SIZE GetMinimizeButtonSize() const;
	};

	TitleBarColors GetDefaultTitleBarColors();

	BOOL UpdateNCArea(HWND hWnd);

	bool TitleBarHandleMessage(HWND hWnd, UINT msg, WPARAM& wParam, LPARAM& lParam, TitleBar& titleBar, LRESULT& lResult);
}
