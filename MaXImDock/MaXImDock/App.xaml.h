#pragma once

#include "App.xaml.g.h"

namespace winrt::MaXImDock::implementation
{
    struct App : AppT<App>
    {
    private:
        winrt::RectInt32 m_windowRect{};
        ::RECT m_rcDispRect{};
        bool m_isRunningWaitActivate = false;
        bool m_isRunningWaitHide = false;
        int m_activateBorderX = 0;
        winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
        winrt::AppWindow m_appWindow{ nullptr };
        ::HWND m_hwnd{ nullptr };
    public:
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        void GetAppWindowForCurrentWindow();
        void SetWindowSizeAndPos();
        void SetWindowStyle();
        winrt::Windows::Foundation::IAsyncAction Async_WaitActivateWindow();
        winrt::Windows::Foundation::IAsyncAction Async_WaitHideWindow();
        winrt::Windows::Foundation::IAsyncAction Async_WaitAccident();
    };
}
