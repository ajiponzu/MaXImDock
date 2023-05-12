#include "pch.h"
#include "AppDataModel.h"

static const winrt::hstring g_appfolderPath = L"MaXImDock";
static const winrt::hstring g_appSettingPath = L"app_setting.json";
static const winrt::hstring g_folderSettingPath = L"folder_setting.json";

namespace MaXImDockModel
{
	std::vector<AppIconData> AppDataModel::s_appDataList{};
	std::vector<FolderLink> AppDataModel::s_folderLinkList{};
	winrt::hstring AppDataModel::s_userPictureFolderPath = L"";

	winrt::IAsyncAction AppDataModel::ReadAppJson(const winrt::Windows::Storage::StorageFolder& appfolder)
	{
		/* アプリアイコン設定ファイル読み込み */
		if (!s_appDataList.empty())
			s_appDataList.clear();
		auto appSettingFile = co_await appfolder.GetFileAsync(g_appSettingPath);
		auto texts = co_await winrt::FileIO::ReadTextAsync(appSettingFile);
		if (texts.empty())
			co_return;

		auto appJson = winrt::JsonObject::Parse(texts);
		auto array = appJson.GetNamedArray(L"array");
		for (const auto& elem : array)
		{
			AppIconData appIconData{};
			auto object = elem.GetObjectW();
			appIconData.m_exePath = object.GetNamedString(L"exe");
			appIconData.m_iconPath = object.GetNamedString(L"icon");
			auto imagefile = co_await appfolder.GetFileAsync(appIconData.m_iconPath);
			winrt::IRandomAccessStream stream{ co_await imagefile.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read) };
			appIconData.m_appIcon.SetSource(stream);
			s_appDataList.push_back(appIconData);
		}
		/* end */
	}

	winrt::IAsyncAction AppDataModel::ReadFolderJson(const winrt::Windows::Storage::StorageFolder& appfolder)
	{
		/* フォルダリンク設定ファイル読み込み */
		if (!s_folderLinkList.empty())
			s_folderLinkList.clear();
		auto folderSettingFile = co_await appfolder.GetFileAsync(g_folderSettingPath);
		auto texts = co_await winrt::FileIO::ReadTextAsync(folderSettingFile);
		if (texts.empty())
			co_return;

		auto appJson = winrt::JsonObject::Parse(texts);
		auto array = appJson.GetNamedArray(L"array");
		for (const auto& elem : array)
		{
			FolderLink folderLink{};
			auto object = elem.GetObjectW();
			folderLink.m_linkPath = object.GetNamedString(L"link");
			folderLink.m_alias = object.GetNamedString(L"alias");
			s_folderLinkList.push_back(folderLink);
		}
		/* end */
	}

	winrt::IAsyncAction AppDataModel::ReadSettingJson()
	{
		/* Appフォルダ検索・初期化 */
		auto picturesfolder = winrt::Windows::Storage::KnownFolders::PicturesLibrary();
		s_userPictureFolderPath = picturesfolder.Path();
		auto foldersInPictures = co_await picturesfolder.GetFoldersAsync();
		bool notFound = true;
		for (const auto& folder : foldersInPictures)
		{
			if (folder.Name() == g_appfolderPath)
				notFound = false;
		}
		if (notFound)
			co_await picturesfolder.CreateFolderAsync(g_appfolderPath);
		/* end */

		/* 設定ファイル検索・新規作成 */
		auto appfolder = co_await picturesfolder.GetFolderAsync(g_appfolderPath);
		auto filesInApp = co_await appfolder.GetFilesAsync();
		auto notFoundApp = true;
		auto notFoundFolder = true;
		for (const auto& file : filesInApp)
		{
			if (file.Name() == g_appSettingPath)
				notFoundApp = false;
			if (file.Name() == g_folderSettingPath)
				notFoundFolder = false;
		}
		if (notFoundApp)
			co_await appfolder.CreateFileAsync(g_appSettingPath);
		if (notFoundFolder)
			co_await appfolder.CreateFileAsync(g_folderSettingPath);
		/* end */

		co_await ReadAppJson(appfolder);
		co_await ReadFolderJson(appfolder);
	}
}