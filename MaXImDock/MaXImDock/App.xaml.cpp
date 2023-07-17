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

	InitWindow();
}

void winrt::MaXImDock::implementation::App::InitWindow()
{
	m_window = make<MainWindow>();

	SetWindowStyle();
	SetWindowSizeAndPos();
	m_window.Activate();
	Async_WaitHideWindow();
}

void winrt::MaXImDock::implementation::App::SetWindowSizeAndPos()
{
	::HWND hwnd = nullptr;
	winrt::com_ptr<IWindowNative> window_native = m_window.as<IWindowNative>(); // win32ネイティブウィンドウの取得
	window_native->get_WindowHandle(&hwnd); // ウィンドウハンドル取得
	const auto hdc = ::GetDC(hwnd);

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
	m_window.AppWindow().MoveAndResize(m_windowRect); // ウィンドウへの変更を適用
}

void winrt::MaXImDock::implementation::App::SetWindowStyle()
{
	OverlappedPresenter overlapped_presenter(0);
	overlapped_presenter = OverlappedPresenter::CreateForContextMenu();
	overlapped_presenter.IsAlwaysOnTop(true); // 常に最前面に表示
	m_window.AppWindow().SetPresenter(overlapped_presenter); // ウィンドウスタイル適用
	m_window.AppWindow().IsShownInSwitchers(false);
}

bool winrt::MaXImDock::implementation::App::CheckCursorEntered(const ::POINT& cursor_pos)
{
	const bool was_entered =
		cursor_pos.x >= m_activateBorderX
		&& cursor_pos.x <= (m_windowRect.X + m_windowRect.Width * 3)
		&& cursor_pos.y >= m_windowRect.Y;

	return was_entered;
}

bool winrt::MaXImDock::implementation::App::CheckCursorOnWindow(const::POINT& cursor_pos)
{
	const bool is_on_window =
		cursor_pos.x >= m_windowRect.X
		&& cursor_pos.x <= (m_windowRect.X + m_windowRect.Width * 3)
		&& cursor_pos.y >= m_windowRect.Y;

	return is_on_window;
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitActivateWindow()
{
	co_await winrt::resume_background(); // これとforegroundの間の処理はバックグラウンドで処理される.

	::POINT cursor_pos;
	::GetCursorPos(&cursor_pos);

	/* ビジーウェイト */
	while (true)
	{
		if (CheckCursorEntered(cursor_pos))
			break;

		Sleep(g_SLEEP_TIME); // スリープタイムは遅延だけでなく, CPUを休ませる意味もある
		::GetCursorPos(&cursor_pos);
	}
	/* end */

	co_await wil::resume_foreground(m_window.DispatcherQueue()); // バックグラウンド処理を終了. UIを操作可能に

	// メインディスプレイ変更時の表示位置を修正する処理
	if (m_window.AppWindow().Position().X < 0)
	{
		InitWindow();
		co_return;
	}

	m_window.Activate(); // 条件を満たしたためウィンドウを表示
	co_await Async_WaitHideWindow(); // ウィンドウ非表示待ち処理を起動

	co_return;
}

winrt::Windows::Foundation::IAsyncAction winrt::MaXImDock::implementation::App::Async_WaitHideWindow()
{
	co_await winrt::resume_background();

	::POINT cursor_pos;
	::GetCursorPos(&cursor_pos);

	while (CheckCursorOnWindow(cursor_pos)) // カーソルが範囲内にある限り非表示にしない
	{
		Sleep(g_SLEEP_TIME);
		::GetCursorPos(&cursor_pos);
	}

	co_await wil::resume_foreground(m_window.DispatcherQueue());

	m_window.AppWindow().Hide();
	co_await Async_WaitActivateWindow();

	co_return;
}