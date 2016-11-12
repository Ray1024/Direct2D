#include "D2D_TMPL.h"

/******************************************************************
WinMain
程序入口
******************************************************************/
int WINAPI WinMain(
    HINSTANCE	/* hInstance */		,
    HINSTANCE	/* hPrevInstance */	,
    LPSTR		/* lpCmdLine */		,
    int			/* nCmdShow */		)
{
    // Ignoring the return value because we want to continue running even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            DemoApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}

/******************************************************************
DemoApp实现
******************************************************************/

DemoApp::DemoApp() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
	m_pDWriteFactory(NULL),
    m_pRT(NULL),
    m_pBrush(NULL),
	m_pTextFormat(NULL)
{
}

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBrush);
	SafeRelease(&m_pTextFormat);
}

HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    //register window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = DemoApp::WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = HINST_THISCOMPONENT;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = L"D2DDemoApp";

    RegisterClassEx(&wcex);

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Create the application window.
        //
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"D2D_TMPL",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(512.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(512.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
			ShowWindow(m_hwnd, SW_SHOWNORMAL);

			UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr;
    ID2D1GeometrySink *pSink = NULL;

    // 创建D2D工厂
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	if (m_pDWriteFactory == NULL && SUCCEEDED(hr))
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
	}

    return hr;
}

HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRT)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        // Create a Direct2D render target
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );

		// 创建画刷
        if (SUCCEEDED(hr))
        {
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Red),
                &m_pBrush
                );
        }

		// 创建字体格式
		if (SUCCEEDED(hr))
		{
			hr = m_pDWriteFactory->CreateTextFormat( 
				L"Arial Black", 
				NULL, 
				DWRITE_FONT_WEIGHT_NORMAL, 
				DWRITE_FONT_STYLE_NORMAL, 
				DWRITE_FONT_STRETCH_NORMAL, 
				20.0, 
				L"en-us",
				&m_pTextFormat);
		}
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBrush);
	SafeRelease(&m_pTextFormat);
}

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT DemoApp::OnRender()
{
    HRESULT hr;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        // 开始绘制
        m_pRT->BeginDraw();

        // 重置呈现器的变换
        m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

        // 清除呈现器上的内容
        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

		// 绘制五角星（路径几何图形）
        m_pRT->DrawText(
			L"Ray1024_D2D_Template",
			wcslen(L"Ray1024_D2D_Template"),
			m_pTextFormat,
			D2D1::RectF(0,0,300,0),
			m_pBrush);
		
        // 结束绘制
        hr = m_pRT->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
    }

    InvalidateRect(m_hwnd, NULL, FALSE);

    return hr;
}

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRT)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;

        // Note: This method can fail, but it's okay to ignore the
        // error here -- it will be repeated on the next call to
        // EndDraw.
        m_pRT->Resize(size);
    }
}

LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DemoApp *pDemoApp = (DemoApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pDemoApp)
            );

        result = 1;
    }
    else
    {
        DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                )));

        bool wasHandled = false;

        if (pDemoApp)
        {
            switch(message)
            {
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pDemoApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
            case WM_DISPLAYCHANGE:
                {
                    PAINTSTRUCT ps;
                    BeginPaint(hwnd, &ps);

                    pDemoApp->OnRender();
                    EndPaint(hwnd, &ps);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}