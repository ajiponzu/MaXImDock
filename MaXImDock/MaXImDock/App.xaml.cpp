#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "AppDataModel.h"

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
	InitSystem();
}

winrt::IAsyncAction winrt::MaXImDock::implementation::App::InitSystem()
{
	co_await MaXImDockModel::AppDataModel::ReadSettingJson();

	m_window = make<MainWindow>();
	GetAppWindowForCurrentWindow();
	SetWindowStyle();
	SetWindowSizeAndPos();

	Async_WaitAccident();
	Async_WaitActivateWindow();
}

void winrt::MaXImDock::implementation::App::GetAppWindowForCurrentWindow()
{
	winrt::com_ptr<IWindowNative> windowNative = m_window.as<IWindowNative>();

	windowNative->get_WindowHandle(&m_hwnd);
	winrt::WindowId windowId;
	windowId = winrt::GetWindowIdFromWindow(m_hwnd);
	m_appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);
}

void winrt::MaXImDock::implementation::App::SetWindowSizeAndPos()
{
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &m_rcDispRect, NULL);
	m_activateBorderX = m_rcDispRect.right - 1;
	m_windowRect.Width = static_cast<int32_t>(m_rcDispRect.right * 0.045);
	m_windowRect.Height = static_cast<int32_t>(m_rcDispRect.bottom * 0.6);
	m_windowRect.X = static_cast<int32_t>(m_rcDispRect.right - m_windowRect.Width * 2);
	m_windowRect.Y = static_cast<int32_t>(m_rcDispRect.bottom - m_windowRect.Height);
	m_appWindow.MoveAndResize(m_windowRect);
}

void winrt::MaXImDock::implementation::App::SetWindowStyle()
{
	OverlappedPresenter customOverlappedPresenter(0);
	customOverlappedPresenter = OverlappedPresenter::CreateForContextMenu();
	customOverlappedPresenter.IsAlwaysOnTop(true);
	m_appWindow.SetPresenter(customOverlappedPresenter);
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitActivateWindow()
{
	m_isRunningWaitActivate = true;
	co_await winrt::resume_background();

	POINT mouse_p;
	::GetCursorPos(&mouse_p);
	while (!(mouse_p.x >= m_activateBorderX && mouse_p.y >= m_windowRect.Y))
	{
		Sleep(gSleepTime);
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(m_window.DispatcherQueue());

	m_window.Activate();
	Async_WaitHideWindow();
	m_isRunningWaitActivate = true;
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitHideWindow()
{
	m_isRunningWaitHide = true;
	co_await winrt::resume_background();

	POINT mouse_p;
	::GetCursorPos(&mouse_p);
	while (mouse_p.x >= m_windowRect.X && mouse_p.y >= m_windowRect.Y)
	{
		Sleep(gSleepTime);
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(m_window.DispatcherQueue());

	m_appWindow.Hide();
	Async_WaitActivateWindow();
	m_isRunningWaitHide = true;
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
		restartFlag = (mouse_p.x >= m_activateBorderX && mouse_p.y >= m_windowRect.Y) && !m_appWindow.IsVisible();
		if (restartFlag)
			break;
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(m_window.DispatcherQueue());

	if (!m_isRunningWaitActivate)
		m_window.Activate();
	if (!m_isRunningWaitHide)
		Async_WaitHideWindow();
	Async_WaitAccident();
}