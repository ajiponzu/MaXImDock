#pragma once

#include "App.xaml.g.h"

namespace winrt::MaXImDock::implementation {
struct App : AppT<App> {
 private:
  winrt::RectInt32 m_windowRect{};  // ウィンドウの座標・サイズ
  ::RECT m_rcDispRect{};            // ディスプレイのDPI情報
  int m_activateBorderX = 0;        // ウィンドウ表示の際の座標境界
  winrt::Microsoft::UI::Xaml::Window m_window{nullptr};  // ウィンドウ
  winrt::IAsyncAction m_windowActivateHandlerStatus{winrt::IAsyncAction()};

 public:
  App();
  ~App();

  void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

 private:
  /// <summary>
  /// 非同期的にシステム起動処理. co_awaitにより, 実行順を制御
  /// </summary>
  winrt::IAsyncAction InitSystem();

  /// <summary>
  /// ウィンドウ初期化部
  /// </summary>
  void InitWindow();

  /// <summary>
  /// ウィンドウの表示位置・サイズを調整
  /// </summary>
  void SetWindowSizeAndPos();

  /// <summary>
  /// ウィンドウのスタイルを設定
  /// </summary>
  void SetWindowStyle();

  /// <summary>
  /// カーソル座標の侵入判定を行う
  /// </summary>
  bool CheckCursorEntered(const ::POINT& cursor_pos);

  /// <summary>
  /// カーソル座標とウィンドウとの当たり判定を行う
  /// </summary>
  bool CheckCursorOnWindow(const ::POINT& cursor_pos);

  /// <summary>
  /// ウィンドウ表示位置が正しいか検証する
  /// </summary>
  bool CheckWindowPosIncorrective();

  /// <summary>
  /// ウィンドウの表示・非表示処理を待機する
  /// </summary>
  winrt::IAsyncAction Async_WindowActivateHandler();
};
}  // namespace winrt::MaXImDock::implementation
