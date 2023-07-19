#include "pch.h"
#include "AppDataModel.h"

static const winrt::hstring g_APP_FOLDER_PATH = L"MaXImDock";
static const winrt::hstring g_APP_SETTING_PATH = L"app_setting.json";
static const winrt::hstring g_FOLDER_SETTING_PATH = L"folder_setting.json";

namespace MaXImDockModel
{
	std::vector<AppIconData> AppDataModel::s_appDataList{};
	std::vector<FolderLink> AppDataModel::s_folderLinkList{};
	winrt::hstring AppDataModel::s_userPictureFolderPath = L"";

	winrt::IAsyncAction AppDataModel::ReadAppJson(const winrt::Windows::Storage::StorageFolder& appfolder)
	{
		/* アプリアイコン設定ファイル読み込み */
		const auto& app_setting_file = co_await appfolder.GetFileAsync(g_APP_SETTING_PATH);
		const auto& texts = co_await winrt::FileIO::ReadTextAsync(app_setting_file);
		if (texts.empty())
			co_return;

		auto app_json = winrt::JsonObject::Parse(texts);
		auto json_array = app_json.GetNamedArray(L"array");
		for (const auto& elem : json_array)
		{
			AppIconData app_icon_data{};
			const auto object = elem.GetObjectW();
			app_icon_data.m_exePath = object.GetNamedString(L"exe");
			app_icon_data.m_iconPath = object.GetNamedString(L"icon");
			const auto& image_file = co_await appfolder.GetFileAsync(app_icon_data.m_iconPath);
			winrt::IRandomAccessStream stream{ co_await image_file.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read) };
			app_icon_data.m_appIcon.SetSource(stream);
			s_appDataList.push_back(app_icon_data);
			stream.Close();
		}
		/* end */
	}

	winrt::IAsyncAction AppDataModel::ReadFolderJson(const winrt::Windows::Storage::StorageFolder& appfolder)
	{
		/* フォルダリンク設定ファイル読み込み */
		const auto& folder_setting_file = co_await appfolder.GetFileAsync(g_FOLDER_SETTING_PATH);
		const auto& texts = co_await winrt::FileIO::ReadTextAsync(folder_setting_file);
		if (texts.empty())
			co_return;

		const auto& app_json = winrt::JsonObject::Parse(texts);
		const auto& json_array = app_json.GetNamedArray(L"array");
		for (const auto& elem : json_array)
		{
			FolderLink folder_link{};
			const auto object = elem.GetObjectW();
			folder_link.m_linkPath = object.GetNamedString(L"link");
			folder_link.m_alias = object.GetNamedString(L"alias");
			s_folderLinkList.push_back(folder_link);
		}
		/* end */
	}

	winrt::IAsyncAction AppDataModel::ReadSettingJson()
	{
		ClearData();

		/* Appフォルダ検索・初期化 */
		auto pictures_folder = winrt::Windows::Storage::KnownFolders::PicturesLibrary();
		s_userPictureFolderPath = pictures_folder.Path();

		const auto& folders_in_pictures = co_await pictures_folder.GetFoldersAsync();
		bool not_found = true;
		for (const auto& folder : folders_in_pictures)
		{
			if (folder.Name() == g_APP_FOLDER_PATH)
				not_found = false;
		}
		if (not_found)
			co_await pictures_folder.CreateFolderAsync(g_APP_FOLDER_PATH);
		/* end */

		/* 設定ファイル検索・新規作成 */
		const auto& app_folder = co_await pictures_folder.GetFolderAsync(g_APP_FOLDER_PATH);
		const auto& files_in_app = co_await app_folder.GetFilesAsync();
		auto not_found_app = true;
		auto not_found_folder = true;

		for (const auto& file : files_in_app)
		{
			if (file.Name() == g_APP_SETTING_PATH)
				not_found_app = false;
			if (file.Name() == g_FOLDER_SETTING_PATH)
				not_found_folder = false;
		}
		if (not_found_app)
			co_await app_folder.CreateFileAsync(g_APP_SETTING_PATH);
		if (not_found_folder)
			co_await app_folder.CreateFileAsync(g_FOLDER_SETTING_PATH);
		/* end */

		co_await ReadAppJson(app_folder);
		co_await ReadFolderJson(app_folder);
	}

	void AppDataModel::ClearData()
	{
		if (!s_appDataList.empty())
			s_appDataList.clear();
		if (!s_folderLinkList.empty())
			s_folderLinkList.clear();
	}
}