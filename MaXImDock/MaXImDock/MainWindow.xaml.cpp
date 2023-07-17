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
		/* ボタンカラー設定 */
		winrt::Media::AcrylicBrush brush{};
		auto color = winrt::Windows::UI::ColorHelper::FromArgb(200, 230, 230, 250);
		brush.TintColor(color);
		brush.TintOpacity(0.85);
		brush.Opacity(0.75);
		/* end */

		/* ボタン角の丸みを設定 */
		winrt::CornerRadius cr{};
		cr.BottomLeft = 20;
		cr.BottomRight = 20;
		cr.TopLeft = 20;
		cr.TopRight = 20;
		/* end */

		/* アプリアイコンの設定・設置 */
		auto app_items = gridIcons().Items(); // GridViewコントロールのItemCollectionを取得. Appendするとデータも見た目的にも追加される
		app_items.Clear();
		for (const auto& app_icon : MaXImDockModel::AppDataModel::GetAppIconList())
		{
			winrt::Button button{};
			/* クリックイベントのラムダ式を定義 */ // 簡単に書けるだけでなく, 処理の自由度も高い
			auto click_handler = [&](winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
			{
				::ShellExecuteW(0, L"Open", L"explorer.exe", app_icon.m_exePath.c_str(), L"", SW_SHOW);
			};
			/* end */
			Image image{};
			image.Source(app_icon.m_appIcon); // 保存していたbitmapimageをソースとする. Imageはコントロールのため, sharedできないのでbitmapまでを作成し保存した
			image.MaxWidth(65);
			image.MaxHeight(65);
			button.Content(image);
			button.Click(click_handler); // クリックイベントの登録
			button.Background(brush);
			button.CornerRadius(cr);
			app_items.Append(button); // GridViewに追加. これを忘れると変更が適用されない
		}
		/* end */

		/* フォルダリンク設定・設置 */
		auto folder_items = folderLinks().Items();
		folder_items.Clear();
		for (const auto& folder_link : MaXImDockModel::AppDataModel::GetFolderLinkList())
		{
			const auto& text = (folder_link.m_alias == L"") ? folder_link.m_linkPath : folder_link.m_alias;
			winrt::Button button{};
			auto click_handler = [&](winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
			{
				::ShellExecuteW(0, L"Open", L"explorer.exe", folder_link.m_linkPath.c_str(), L"", SW_SHOW);
			};
			button.Content(box_value(text)); // box_valueはおそらくただの文字列をText系のコントロールに変換してくれると思われる.
			button.Click(click_handler);
			button.Background(brush);
			folder_items.Append(button);
		}
		/* end */
	}

	winrt::IAsyncAction MainWindow::ClickOnReloadButton(winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
	{
		co_await MaXImDockModel::AppDataModel::ReadSettingJson();

		InitViewControls();
	}
}