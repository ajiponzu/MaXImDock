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

static constexpr int g_BASE_DISP_WID = 1920;
static constexpr int g_BASE_DISP_HIGH = 1200;
static constexpr int g_SLEEP_TIME = 400;

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

	InitWindow(true);
}

void winrt::MaXImDock::implementation::App::InitWindow(const bool& is_waited_activate)
{
	m_window = make<MainWindow>();
	const auto closed_handler = [&](winrt::IInspectable const& /*sender*/, winrt::WindowEventArgs const& /*args*/)
	{
		InitWindow(false);
	};
	m_window.Closed(closed_handler);

	GetAppWindowForCurrentWindow();
	SetWindowStyle();
	SetWindowSizeAndPos();

	if (is_waited_activate)
		Async_WaitActivateWindow();
	else
	{
		m_window.Activate();
		Async_WaitHideWindow();
	}
}

void winrt::MaXImDock::implementation::App::GetAppWindowForCurrentWindow()
{
	winrt::com_ptr<IWindowNative> window_native = m_window.as<IWindowNative>(); // win32ネイティブウィンドウの取得

	window_native->get_WindowHandle(&m_hwnd); // ウィンドウハンドル取得
	const auto window_id = winrt::GetWindowIdFromWindow(m_hwnd);
	m_appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(window_id); // ウィンドウハンドルとWinUIとの橋渡し?を構築
}

void winrt::MaXImDock::implementation::App::SetWindowSizeAndPos()
{
	const auto hdc = ::GetDC(m_hwnd);

	const auto x_dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
	const auto y_dpi = ::GetDeviceCaps(hdc, LOGPIXELSY);
	const auto x_dpi_rate = (double)x_dpi / USER_DEFAULT_SCREEN_DPI;
	const auto y_dpi_rate = (double)y_dpi / USER_DEFAULT_SCREEN_DPI;

	const auto x_disp = ::GetDeviceCaps(hdc, HORZRES);
	const auto y_disp = ::GetDeviceCaps(hdc, VERTRES);
	const auto x_disp_rate = (double)x_disp / g_BASE_DISP_WID;
	const auto y_disp_rate = (double)y_disp / g_BASE_DISP_HIGH;

	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &m_rcDispRect, NULL);
	m_activateBorderX = m_rcDispRect.right - 1;
	m_windowRect.Width = (int32_t)(m_rcDispRect.right * 0.05 * x_dpi_rate / x_disp_rate); // display比率はなぜか割る
	m_windowRect.Height = (int32_t)(m_rcDispRect.bottom * 0.45 * y_dpi_rate / y_disp_rate);
	m_windowRect.X = (int32_t)(m_rcDispRect.right - m_windowRect.Width * 1.5);
	m_windowRect.Y = (int32_t)(m_rcDispRect.bottom - m_windowRect.Height);
	m_appWindow.MoveAndResize(m_windowRect); // ウィンドウへの変更を適用
}

void winrt::MaXImDock::implementation::App::SetWindowStyle()
{
	OverlappedPresenter overlapped_presenter(0);
	overlapped_presenter = OverlappedPresenter::CreateForContextMenu();
	overlapped_presenter.IsAlwaysOnTop(true); // 常に最前面に表示
	m_appWindow.SetPresenter(overlapped_presenter); // ウィンドウスタイル適用
	m_appWindow.IsShownInSwitchers(false);
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitActivateWindow()
{
	m_isRunningWaitActivate = true;
	co_await winrt::resume_background(); // これとforegroundの間の処理はバックグラウンドで処理される.

	POINT mouse_p;
	::GetCursorPos(&mouse_p);

	const auto check_wait_flag = [&]()
	{
		return !(mouse_p.x >= m_activateBorderX
			&& mouse_p.y >= m_windowRect.Y);
	};
	/* ビジーウェイト */
	while (check_wait_flag()) // カーソルが範囲内にない限り表示しない
	{
		Sleep(g_SLEEP_TIME); // スリープタイムは, ただの遅延というだけでなく, CPUを休ませる意味もある
		::GetCursorPos(&mouse_p);
	}
	/* end */

	co_await wil::resume_foreground(m_window.DispatcherQueue()); // バックグラウンド処理を終了. UIを操作可能に

	// メインディスプレイ変更時の表示位置を修正する処理
	if (m_appWindow.Position().X < 0)
		m_window.Close();

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

	const auto check_wait_flag = [&]()
	{
		return mouse_p.x >= m_windowRect.X
			&& mouse_p.y >= m_windowRect.Y;
	};
	while (check_wait_flag()) // カーソルが範囲内にある限り非表示にしない
	{
		Sleep(g_SLEEP_TIME);
		::GetCursorPos(&mouse_p);
	}

	co_await wil::resume_foreground(m_window.DispatcherQueue());

	// メインディスプレイ変更時の表示位置を修正する処理
	if (m_appWindow.Position().X < 0)
		m_window.Close();

	m_appWindow.Hide();
	Async_WaitActivateWindow();
	m_isRunningWaitHide = false;
}