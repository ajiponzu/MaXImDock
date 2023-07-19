#pragma once
#include "MainWindow.g.h"

namespace winrt::MaXImDock::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		MainWindow();

		int32_t MyProperty();
		void MyProperty(int32_t value);

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns></returns>
		winrt::IAsyncAction Init();

		/// <summary>
		/// xamlコントロールの動的追加
		/// </summary>
		void InitViewControls();

		/// <summary>
		/// リロードボタンをクリックしたときのイベント処理
		/// </summary>
		winrt::IAsyncAction ClickOnReloadButton(winrt::IInspectable const& sender, winrt::RoutedEventArgs const& args);

		/// <summary>
		/// 自己終了の仕組み
		/// </summary>
		static uint8_t s_idGenerator;
		uint8_t m_id = 0;
		winrt::IAsyncAction SelfClose();
	};
}

namespace winrt::MaXImDock::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
