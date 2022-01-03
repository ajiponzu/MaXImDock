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
	co_await MaXImDockModel::AppDataModel::ReadSettingJson(); // これがそのまま非同期実行されると, UI表示のタイミングとかぶるなどの危険があるため待つ

	m_window = make<MainWindow>();
	GetAppWindowForCurrentWindow();
	SetWindowStyle();
	SetWindowSizeAndPos();

	Async_WaitActivateWindow();
}

void winrt::MaXImDock::implementation::App::GetAppWindowForCurrentWindow()
{
	winrt::com_ptr<IWindowNative> windowNative = m_window.as<IWindowNative>(); // win32ネイティブウィンドウの取得

	windowNative->get_WindowHandle(&m_hwnd); // ウィンドウハンドル取得
	winrt::WindowId windowId;
	windowId = winrt::GetWindowIdFromWindow(m_hwnd);
	m_appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId); // ウィンドウハンドルとWinUIとの橋渡し?を構築
	auto visibilityEventHandler = [&](winrt::IInspectable const& /*sender*/, winrt::WindowVisibilityChangedEventArgs const& /*args*/)
	{
		if (m_window.Visible())
			return;
		else
		{
			if (m_isRunningWaitActivate)
				m_window.Activate();
		}
	};
	m_window.VisibilityChanged(visibilityEventHandler);
}

void winrt::MaXImDock::implementation::App::SetWindowSizeAndPos()
{
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &m_rcDispRect, NULL); // DPI取得
	m_activateBorderX = m_rcDispRect.right - 1;
	m_windowRect.Width = static_cast<int32_t>(m_rcDispRect.right * 0.045);
	m_windowRect.Height = static_cast<int32_t>(m_rcDispRect.bottom * 0.6);
	m_windowRect.X = static_cast<int32_t>(m_rcDispRect.right - m_windowRect.Width * 2);
	m_windowRect.Y = static_cast<int32_t>(m_rcDispRect.bottom - m_windowRect.Height);
	m_appWindow.MoveAndResize(m_windowRect); // ウィンドウへの変更を適用
}

void winrt::MaXImDock::implementation::App::SetWindowStyle()
{
	OverlappedPresenter overlappedPresenter(0); 
	overlappedPresenter = OverlappedPresenter::CreateForContextMenu();
	overlappedPresenter.IsAlwaysOnTop(true); // 常に最前面に表示
	m_appWindow.SetPresenter(overlappedPresenter); // ウィンドウスタイル適用
	m_appWindow.IsShownInSwitchers(false);
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitActivateWindow()
{
	m_isRunningWaitActivate = true;
	co_await winrt::resume_background(); // これとforegroundの間の処理はバックグラウンドで処理される.

	POINT mouse_p;
	::GetCursorPos(&mouse_p);
	/* ビジーウェイト */
	while (!(mouse_p.x >= m_activateBorderX && mouse_p.y >= m_windowRect.Y))
	{
		Sleep(gSleepTime); // スリープタイムは, ただの遅延というだけでなく, CPUを休ませる意味もある
		::GetCursorPos(&mouse_p); 
	}
	/* end */

	co_await wil::resume_foreground(m_window.DispatcherQueue()); // バックグラウンド処理を終了. UIを操作可能に

	m_window.Activate(); // 条件を満たしたためウィンドウを表示
	Async_WaitHideWindow(); // ウィンドウ非表示待ち処理を起動
	m_isRunningWaitActivate = false;
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
	m_isRunningWaitHide = false;
}
