#pragma once
#include "MainWindow.g.h"

namespace winrt::MaXImDock::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		MainWindow();

		int32_t MyProperty();
		void MyProperty(int32_t value);
		void InitViewControls();
	};
}

namespace winrt::MaXImDock::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
