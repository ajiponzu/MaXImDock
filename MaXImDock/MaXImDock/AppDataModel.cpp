#include "pch.h"
#include "AppDataModel.h"

static const winrt::hstring g_appfolderPath = L"MaXImDock";
static const winrt::hstring g_appSettingPath = L"app_setting.json";
static const winrt::hstring g_folderSettingPath = L"folder_setting.json";

namespace MaXImDockModel
{
	winrt::RectInt32 AppDataModel::s_windowRect{};
	::RECT AppDataModel::s_rcDispRect{};
	::HWND AppDataModel::s_hwnd{ nullptr };
	winrt::AppWindow AppDataModel::s_appWindow{ nullptr };
	std::vector<AppIconData> AppDataModel::s_appDataList{};
	std::vector<FolderLink> AppDataModel::s_folderLinkList{};

	winrt::IAsyncAction AppDataModel::ReadSettingJson()
	{
		/* Appフォルダ検索・初期化 */
		auto picturesfolder = winrt::Windows::Storage::KnownFolders::PicturesLibrary();
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

		/* 設定ファイル検索・初期化 */
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

		auto appSettingFile = co_await appfolder.GetFileAsync(g_appSettingPath);
		auto texts = co_await winrt::FileIO::ReadTextAsync(appSettingFile);
		auto appJson = winrt::JsonObject::Parse(texts);
		auto array = appJson.GetNamedArray(L"array");
		AppIconData appIconData{};
		for (const auto& elem : array)
		{
			auto object = elem.GetObjectW();
			appIconData.m_exePath = object.GetNamedString(L"exe");
			appIconData.m_iconPath = object.GetNamedString(L"icon");
			s_appDataList.push_back(appIconData);
		}

		auto folderSettingFile = co_await appfolder.GetFileAsync(g_folderSettingPath);
		array = appJson.GetNamedArray(L"array");
		FolderLink folderLink{};
		for (const auto& elem : array)
		{
			auto object = elem.GetObjectW();
			folderLink.m_linkPath = object.GetNamedString(L"link");
			folderLink.m_alias = object.GetNamedString(L"alias");
			s_folderLinkList.push_back(folderLink);
		}
	}
}