#pragma once

#include "App.xaml.g.h"

namespace winrt::MaXImDock::implementation
{
    struct App : AppT<App>
    {
    private:
        RECT rcDispRect{};
        winrt::Windows::Graphics::RectInt32 windowRect{};
        bool isRunningWaitActivate = false;
        bool isRunningWaitHide = false;
        int activateBorderX = 0;
    public:
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };
        winrt::AppWindow appWindow{ nullptr };
        void GetAppWindowForCurrentWindow();
        void SetWindowSizeAndPos();
        void SetWindowStyle();
        winrt::Windows::Foundation::IAsyncAction Async_WaitActivateWindow();
        winrt::Windows::Foundation::IAsyncAction Async_WaitHideWindow();
        winrt::Windows::Foundation::IAsyncAction Async_WaitAccident();
    };
}
