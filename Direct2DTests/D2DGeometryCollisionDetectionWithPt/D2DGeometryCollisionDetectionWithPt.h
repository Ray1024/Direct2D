#pragma once

// 如果你需要在以下指定的平台之前定位平台，请修改以下定义。
// 参考MSDN上最新的平台的相应的值
#ifndef WINVER              // 适用于Win7及Win7以上版本
#define WINVER 0x0700       // 修改这个值来确定不同的windows版本
#endif

#ifndef _WIN32_WINNT        // 适用于Win7及Win7以上版本
#define _WIN32_WINNT 0x0700 // 修改这个值来确定不同的windows版本
#endif

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN     // 从windows头文件中去除很少使用的内容

// windows头文件
#include <windows.h>

// c运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>

// Direct2D需要的头文件
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dwmapi.h>

/******************************************************************
需要用到的宏定义
******************************************************************/

template<class Interface>
inline void
SafeRelease(
    Interface **ppInterfaceToRelease
    )
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


#include <math.h>

/******************************************************************
DemoApp
******************************************************************/
class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

    HRESULT Initialize();

    void RunMessageLoop();

private:
    HRESULT CreateDeviceIndependentResources();
    HRESULT CreateDeviceResources();
    void DiscardDeviceResources();

    HRESULT OnRender();

    void OnResize(
        UINT width,
        UINT height
        );

	// 绘制点
	void RenderPoint(D2D1_POINT_2F& pt);

    void OnTimer();

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
    HWND m_hwnd;
    ID2D1Factory *m_pD2DFactory;
    ID2D1HwndRenderTarget *m_pRT;
    ID2D1PathGeometry *m_pObjectGeometry;
    ID2D1SolidColorBrush *m_pBrush;
};


