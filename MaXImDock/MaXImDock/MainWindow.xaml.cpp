#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include "AppDataModel.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::MaXImDock::implementation
{
	MainWindow::MainWindow()
	{
		InitializeComponent();
		InitViewControls();
	}

	int32_t MainWindow::MyProperty()
	{
		throw hresult_not_implemented();
	}

	void MainWindow::MyProperty(int32_t /* value */)
	{
		throw hresult_not_implemented();
	}

	void MainWindow::InitViewControls()
	{
		auto items = gridIcons().Items();
		for (const auto& appIcon : MaXImDockModel::AppDataModel::GetAppIconList())
		{
			winrt::Button button{};
			auto clickEventHandler = [&](winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
			{
				::ShellExecuteW(0, L"Open", L"explorer.exe", appIcon.m_exePath.c_str(), L"", SW_SHOW);
			};
			Image image{};
			image.Source(appIcon.m_appIcon);
			image.MaxWidth(65);
			image.MaxHeight(65);
			button.Content(image);
			button.Click(clickEventHandler);
			items.Append(button);
		}

		items = folderLinks().Items();
		for (const auto& folderLink : MaXImDockModel::AppDataModel::GetFolderLinkList())
		{
			const auto& text = (folderLink.m_alias == L"") ? folderLink.m_linkPath : folderLink.m_alias;
			winrt::Button button{};
			auto clickEventHandler = [&](winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
			{
				::ShellExecuteW(0, L"Open", L"explorer.exe", folderLink.m_linkPath.c_str(), L"", SW_SHOW);
			};
			button.Content(box_value(text));
			button.Click(clickEventHandler);
			items.Append(button);
		}
	}
}