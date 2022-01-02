#pragma once

namespace MaXImDockModel
{
	/// <summary>
	/// appIconのデータ
	/// </summary>
	struct AppIconData {
	public:
		winrt::hstring m_exePath = L""; // 実行ファイルの絶対パス
		winrt::hstring m_iconPath = L""; // 規定フォルダからの相対パス
		winrt::BitmapImage m_appIcon{}; // アイコンイメージコントロール
	};

	/// <summary>
	/// folderへのリンクデータ
	/// </summary>
	struct FolderLink {
	public:
		winrt::hstring m_linkPath = L""; // フォルダの絶対パス
		winrt::hstring m_alias = L""; // 表示名. ""ならパスをそのまま表示
	};

	/// <summary>
	/// staticクラス. アプリデータの管理
	/// </summary>
	class AppDataModel
	{
	public:
		AppDataModel() = delete;
		~AppDataModel() = delete;
		AppDataModel operator=(const AppDataModel& other) = delete;
		AppDataModel operator=(AppDataModel&& other) = delete;

	private:
		static std::vector<AppIconData> s_appDataList; // appIconデータリスト
		static std::vector<FolderLink> s_folderLinkList; // folderリンクデータリスト
	public:
		static std::vector<AppIconData>& GetAppIconList() { return s_appDataList; }
		static std::vector<FolderLink>& GetFolderLinkList() { return s_folderLinkList; }

		/// <summary>
		/// 非同期でjsonファイルから設定を読みこむ
		/// </summary>
		static winrt::IAsyncAction ReadSettingJson();
	};
}
