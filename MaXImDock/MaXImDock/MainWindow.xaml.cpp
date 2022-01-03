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
		auto items = gridIcons().Items(); // GridViewコントロールのItemCollectionを取得. Appendするとデータも見た目的にも追加される
		items.Clear();
		for (const auto& appIcon : MaXImDockModel::AppDataModel::GetAppIconList())
		{
			winrt::Button button{};
			/* クリックイベントのラムダ式を定義 */ // 簡単に書けるだけでなく, 処理の自由度も高い
			auto clickEventHandler = [&](winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
			{
				::ShellExecuteW(0, L"Open", L"explorer.exe", appIcon.m_exePath.c_str(), L"", SW_SHOW);
			};
			/* end */
			Image image{};
			image.Source(appIcon.m_appIcon); // 保存していたbitmapimageをソースとする. Imageはコントロールのため, sharedできないのでbitmapまでを作成し保存した
			image.MaxWidth(65);
			image.MaxHeight(65);
			button.Content(image);
			button.Click(clickEventHandler); // クリックイベントの登録
			button.Background(brush);
			button.CornerRadius(cr);
			items.Append(button); // GridViewに追加. これを忘れると変更が適用されない
		}
		/* end */

		/* フォルダリンク設定・設置 */
		items = folderLinks().Items();
		items.Clear();
		for (const auto& folderLink : MaXImDockModel::AppDataModel::GetFolderLinkList())
		{
			const auto& text = (folderLink.m_alias == L"") ? folderLink.m_linkPath : folderLink.m_alias;
			winrt::Button button{};
			auto clickEventHandler = [&](winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
			{
				::ShellExecuteW(0, L"Open", L"explorer.exe", folderLink.m_linkPath.c_str(), L"", SW_SHOW);
			};
			button.Content(box_value(text)); // box_valueはおそらくただの文字列をText系のコントロールに変換してくれると思われる. 
			button.Click(clickEventHandler);
			button.Background(brush);
			items.Append(button);
		}
		/* end */
	}

	winrt::IAsyncAction MainWindow::ClickOnReloadButton(winrt::IInspectable const& /*sender*/, winrt::RoutedEventArgs const& /*args*/)
	{
		co_await MaXImDockModel::AppDataModel::ReadSettingJson();
		
		InitViewControls();
	}
}