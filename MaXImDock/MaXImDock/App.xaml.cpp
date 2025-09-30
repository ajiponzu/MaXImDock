#include "App.xaml.h"

#include <WinUser.h>
#include <Windows.h>

#include "AppDataModel.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace MaXImDock;
using namespace MaXImDock::implementation;
using namespace std::chrono_literals;

static constexpr int g_BASE_DISP_WID = 1920;
static constexpr int g_BASE_DISP_HIGH = 1200;
static constexpr int g_SLEEP_TIME = 400;
static constexpr double g_INCH_PER_MILLIMETER = 25.4;

static bool g_is_resized = false;
static bool g_is_resizing = false;

HANDLE g_hMutex = nullptr;

/// <summary>
/// Initializes the singleton application object.  This is the first line of
/// authored code executed, and as such is the logical equivalent of main() or
/// WinMain().
/// </summary>
App::App() {
  InitializeComponent();

#if defined _DEBUG && \
    !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
  UnhandledException(
      [this](IInspectable const&, UnhandledExceptionEventArgs const& e) {
        if (IsDebuggerPresent()) {
          auto errorMessage = e.Message();
          __debugbreak();
        }
      });
#endif
}

App::~App() {
  if (g_hMutex) {
    ::CloseHandle(g_hMutex);
  }
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other
/// entry points will be used such as when the application is launched to open a
/// specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const&) {
  g_hMutex = ::CreateMutexW(nullptr, FALSE, L"MaXImDock_Mutex");
  if (!g_hMutex) {
    ::MessageBoxW(nullptr, L"Mutexの作成に失敗しました", L"エラー", MB_OK);
    exit(1);
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    ::CloseHandle(g_hMutex);
    exit(0);
  }

  InitWindow();
}

winrt::IAsyncAction winrt::MaXImDock::implementation::App::InitSystem() {
  co_await MaXImDockModel::AppDataModel::ReadSettingJson();

  InitWindow();
}

void winrt::MaXImDock::implementation::App::InitWindow() {
  if (g_is_resizing && !g_is_resized) return;

  m_window = make<MainWindow>();

  SetWindowStyle();
  SetWindowSizeAndPos();
  m_window.AppWindow().MoveAndResize(m_windowRect);

  const auto size_changed_handler =
      [&](winrt::IInspectable const& /*sender*/,
          winrt::Microsoft::UI::Xaml::
              WindowSizeChangedEventArgs const& /*args*/) {
        g_is_resizing = true;

        if (!g_is_resized)
          g_is_resized = true;
        else
          g_is_resized = false;

        m_windowActivateHandlerStatus.Close();
        ::Sleep(g_SLEEP_TIME);
        InitWindow();
        g_is_resizing = false;
      };
  m_window.SizeChanged(size_changed_handler);

  m_windowActivateHandlerStatus = IAsyncAction{Async_WindowActivateHandler()};
}

void winrt::MaXImDock::implementation::App::SetWindowSizeAndPos() {
  ::HWND hwnd = nullptr;
  winrt::com_ptr<IWindowNative> window_native =
      m_window.as<IWindowNative>();        // win32ネイティブウィンドウの取得
  window_native->get_WindowHandle(&hwnd);  // ウィンドウハンドル取得

  // 取得するdpiはhwndのパラメータなので，同一hwndでのdpi変化は感知しない.
  // だからwindowを作り直す
  const auto dpi_rate =
      (double)::GetDpiForWindow(hwnd) / USER_DEFAULT_SCREEN_DPI;

  ::SystemParametersInfo(SPI_GETWORKAREA, NULL, &m_rcDispRect, NULL);
  const auto x_disp_rate = (double)m_rcDispRect.right / g_BASE_DISP_WID;
  const auto y_disp_rate = (double)m_rcDispRect.bottom / g_BASE_DISP_HIGH;

  m_activateBorderX = m_rcDispRect.right - 1;
  m_windowRect.Width = (int32_t)((double)m_rcDispRect.right * 0.075 * dpi_rate /
                                 x_disp_rate);  // display比率はなぜか割る
  m_windowRect.Height =
      (int32_t)((double)m_rcDispRect.bottom * 0.45 * dpi_rate / y_disp_rate);
  m_windowRect.X = (int32_t)(m_activateBorderX - m_windowRect.Width);
  m_windowRect.Y = (int32_t)(m_rcDispRect.bottom - m_windowRect.Height);
}

void winrt::MaXImDock::implementation::App::SetWindowStyle() {
  OverlappedPresenter overlapped_presenter(0);
  overlapped_presenter = OverlappedPresenter::CreateForContextMenu();
  overlapped_presenter.IsAlwaysOnTop(true);  // 常に最前面に表示
  m_window.AppWindow().SetPresenter(
      overlapped_presenter);  // ウィンドウスタイル適用
  m_window.AppWindow().IsShownInSwitchers(false);
}

bool winrt::MaXImDock::implementation::App::CheckCursorEntered(
    const ::POINT& cursor_pos) {
  const bool was_entered =
      cursor_pos.x >= m_activateBorderX &&
      cursor_pos.x <= (m_windowRect.X + m_windowRect.Width * 3) &&
      cursor_pos.y >= m_windowRect.Y;

  return was_entered;
}

bool winrt::MaXImDock::implementation::App::CheckCursorOnWindow(
    const ::POINT& cursor_pos) {
  const bool is_on_window =
      cursor_pos.x >= m_windowRect.X &&
      cursor_pos.x <= (m_windowRect.X + m_windowRect.Width * 3) &&
      cursor_pos.y >= m_windowRect.Y;

  return is_on_window;
}

bool winrt::MaXImDock::implementation::App::CheckWindowPosIncorrective() {
  const auto app_window_pos = m_window.AppWindow().Position();
  const auto is_incorrect = app_window_pos.X < m_rcDispRect.left ||
                            app_window_pos.X > m_rcDispRect.right ||
                            app_window_pos.Y < m_rcDispRect.top ||
                            app_window_pos.Y > m_rcDispRect.bottom;

  return is_incorrect;
}

winrt::IAsyncAction
winrt::MaXImDock::implementation::App::Async_WindowActivateHandler() {
  co_await winrt::resume_background();

  bool is_waited_activate = false;

  ::POINT cursor_pos;
  ::GetCursorPos(&cursor_pos);

  while (true) {
    ::Sleep(g_SLEEP_TIME);
    ::GetCursorPos(&cursor_pos);
    SetWindowSizeAndPos();

    if (is_waited_activate && CheckCursorEntered(cursor_pos)) {
      co_await wil::resume_foreground(m_window.DispatcherQueue());
      if (CheckWindowPosIncorrective())
        m_window.AppWindow().MoveAndResize(winrt::RectInt32{1, 1, 1, 1});
      else
        m_window.Activate();
      co_await winrt::resume_background();
      is_waited_activate = false;
    } else if (!CheckCursorOnWindow(cursor_pos)) {
      co_await wil::resume_foreground(m_window.DispatcherQueue());
      m_window.AppWindow().Hide();
      co_await winrt::resume_background();
      is_waited_activate = true;
    }
  }
}