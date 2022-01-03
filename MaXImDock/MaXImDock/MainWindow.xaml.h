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
		/// xamlコントロールの動的追加
		/// </summary>
		void InitViewControls();

		/// <summary>
		/// リロードボタンをクリックしたときのイベント処理
		/// </summary>
		winrt::IAsyncAction ClickOnReloadButton(winrt::IInspectable const& sender, winrt::RoutedEventArgs const& args);
	};
}

namespace winrt::MaXImDock::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
