#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include <winrt/Microsoft.UI.Interop.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace MaXImDock;
using namespace MaXImDock::implementation;

static constexpr int gSleepTime = 400;
static constexpr int gSleepTimeForAccident = 2000;

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
	InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
	UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
		{
			if (IsDebuggerPresent())
			{
				auto errorMessage = e.Message();
				__debugbreak();
			}
		});
#endif
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const&)
{
	window = make<MainWindow>();

	GetAppWindowForCurrentWindow();
	SetWindowStyle();
	SetWindowSizeAndPos();
	appWindow.MoveAndResize(windowRect);

	Async_WaitAccident();
	Async_WaitActivateWindow();
}

void winrt::MaXImDock::implementation::App::GetAppWindowForCurrentWindow()
{
	winrt::com_ptr<IWindowNative> windowNative = window.as<IWindowNative>();

	HWND hWnd;
	windowNative->get_WindowHandle(&hWnd);
	winrt::WindowId windowId;
	windowId = winrt::GetWindowIdFromWindow(hWnd);
	appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);
}

void winrt::MaXImDock::implementation::App::SetWindowSizeAndPos()
{
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcDispRect, NULL);
	activateBorderX = rcDispRect.right - 1;
	windowRect.Width = static_cast<int32_t>(rcDispRect.right * 0.03);
	windowRect.Height = static_cast<int32_t>(rcDispRect.bottom * 0.6);
	windowRect.X = static_cast<int32_t>(rcDispRect.right - windowRect.Width * 2);
	windowRect.Y = static_cast<int32_t>(rcDispRect.bottom - windowRect.Height);
}

void winrt::MaXImDock::implementation::App::SetWindowStyle()
{
	OverlappedPresenter customOverlappedPresenter(0);
	customOverlappedPresenter = OverlappedPresenter::CreateForContextMenu();
	customOverlappedPresenter.IsAlwaysOnTop(true);
	appWindow.SetPresenter(customOverlappedPresenter);
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitActivateWindow()
{
	isRunningWaitActivate = true;
	co_await winrt::resume_background();

	POINT mouse_p;
	::GetCursorPos(&mouse_p);
	//while (!(mouse_p.x >= windowRect.X && mouse_p.y >= windowRect.Y))
	while (!(mouse_p.x >= activateBorderX && mouse_p.y >= windowRect.Y))
	{
		Sleep(gSleepTime);
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(window.DispatcherQueue());

	window.Activate();
	Async_WaitHideWindow();
	isRunningWaitActivate = true;
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitHideWindow()
{
	isRunningWaitHide = true;
	co_await winrt::resume_background();

	POINT mouse_p;
	::GetCursorPos(&mouse_p);
	while (mouse_p.x >= windowRect.X && mouse_p.y >= windowRect.Y)
	{
		Sleep(gSleepTime);
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(window.DispatcherQueue());

	appWindow.Hide();
	Async_WaitActivateWindow();
	isRunningWaitHide = true;
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitAccident()
{
	co_await winrt::resume_background();

	POINT mouse_p;
	bool restartFlag = false;
	::GetCursorPos(&mouse_p);
	while (true)
	{
		Sleep(gSleepTimeForAccident);
		restartFlag = (mouse_p.x >= activateBorderX && mouse_p.y >= windowRect.Y) && !appWindow.IsVisible();
		if (restartFlag)
			break;
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(window.DispatcherQueue());

	if (!isRunningWaitActivate)
		window.Activate();
	if (!isRunningWaitHide)
		Async_WaitHideWindow();
	Async_WaitAccident();
}
