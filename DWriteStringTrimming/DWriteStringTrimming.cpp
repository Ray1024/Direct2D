#include "DWriteStringTrimming.h"

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

DemoApp::DemoApp()
	: m_hwnd(NULL)
	, m_pD2DFactory(NULL)
	, m_pDWriteFactory(NULL)
	, m_pRT(NULL)
	, m_pFontBrush(NULL)
	, m_pRectBrush(NULL)
	, m_pTF1(NULL)
	, m_pTF2(NULL)
	, m_pTF3(NULL)
	, m_pTF4(NULL)
	, m_pTF5(NULL)
{
}

DemoApp::~DemoApp()
{
	// 释放资源
	SafeRelease(&m_pFontBrush);
	SafeRelease(&m_pRectBrush);
	SafeRelease(&m_pTF1);
	SafeRelease(&m_pTF2);
	SafeRelease(&m_pTF3);
	SafeRelease(&m_pTF4);
	SafeRelease(&m_pTF5);

	SafeRelease(&m_pRT);
	SafeRelease(&m_pDWriteFactory);
	SafeRelease(&m_pD2DFactory);
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
            L"D2DCollisionDetectionBetweenGeometrys",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(1000 * dpiX / 96.f)),
            static_cast<UINT>(ceil(600 * dpiY / 96.f)),
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
    // 创建D2D工厂
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

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

        // 创建 render target
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );

        if (SUCCEEDED(hr))
        {
			//创建渲染文字的资源--------------------------------------------------------------------------------
			if (SUCCEEDED(hr))
			{
				hr = m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red),
					&m_pFontBrush
					);
			}

			if (SUCCEEDED(hr))
			{
				hr = m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green),
					&m_pRectBrush
					);
			}

			// 字体格式1：不使用去尾
			if (SUCCEEDED(hr))
			{
				DWRITE_TRIMMING trim1;
				IDWriteInlineObject* trim2 = NULL;
				hr = m_pDWriteFactory->CreateTextFormat( L"Arial Black", NULL, DWRITE_FONT_WEIGHT_NORMAL, 
					DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0, L"en-us",&m_pTF1);

				if (SUCCEEDED(hr))
				{	// 换行模式（DWRITE_WORD_WRAPPING）设置为不换行
					hr = m_pTF1->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				}

				if (SUCCEEDED(hr))
				{
					trim1.granularity = DWRITE_TRIMMING_GRANULARITY_NONE;
					trim1.delimiter = 1;
					trim1.delimiterCount = 10;
					hr = m_pTF1->SetTrimming(&trim1,trim2);
				}
			}

			// 字体格式2：以字符为单位去尾
			if (SUCCEEDED(hr))
			{
				DWRITE_TRIMMING trim1;
				IDWriteInlineObject* trim2 = NULL;
				hr = m_pDWriteFactory->CreateTextFormat( L"Arial Black", NULL, DWRITE_FONT_WEIGHT_NORMAL, 
					DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0, L"en-us",&m_pTF2);

				if (SUCCEEDED(hr))
				{	// 换行模式（DWRITE_WORD_WRAPPING）设置为不换行
					hr = m_pTF2->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				}

				if (SUCCEEDED(hr))
				{
					trim1.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
					trim1.delimiter = 1;
					trim1.delimiterCount = 10;
					hr = m_pTF2->SetTrimming(&trim1,trim2);
				}
			}

			// 字体格式3：以单词为单位去尾
			if (SUCCEEDED(hr))
			{
				DWRITE_TRIMMING trim1;
				IDWriteInlineObject* trim2 = NULL;
				hr = m_pDWriteFactory->CreateTextFormat( L"Arial Black", NULL, DWRITE_FONT_WEIGHT_NORMAL, 
					DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0, L"en-us",&m_pTF3);

				if (SUCCEEDED(hr))
				{	// 换行模式（DWRITE_WORD_WRAPPING）设置为不换行
					hr = m_pTF3->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				}

				if (SUCCEEDED(hr))
				{
					trim1.granularity = DWRITE_TRIMMING_GRANULARITY_WORD;
					trim1.delimiter = 1;
					trim1.delimiterCount = 10;
					hr = m_pTF3->SetTrimming(&trim1,trim2);
				}
			}

			// 字体格式4：以字符为单位去尾，用省略号代替略去部分
			if (SUCCEEDED(hr))
			{
				DWRITE_TRIMMING trim1;
				IDWriteInlineObject* trim2 = NULL;
				hr = m_pDWriteFactory->CreateTextFormat( L"Arial Black", NULL, DWRITE_FONT_WEIGHT_NORMAL, 
					DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0, L"en-us",&m_pTF4);

				if (SUCCEEDED(hr))
				{	// 换行模式（DWRITE_WORD_WRAPPING）设置为不换行
					hr = m_pTF4->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				}

				if (SUCCEEDED(hr))
				{
					m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTF4,&trim2);
				}

				if (SUCCEEDED(hr))
				{
					trim1.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
					trim1.delimiter = 1;
					trim1.delimiterCount = 10;
					hr = m_pTF4->SetTrimming(&trim1,trim2);
				}
			}

			// 字体格式5：以单词为单位去尾，用省略号代替略去部分
			if (SUCCEEDED(hr))
			{
				DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_NONE, 0, 0 };
				DWRITE_TRIMMING trim1;
				IDWriteInlineObject* trim2 = NULL;
				hr = m_pDWriteFactory->CreateTextFormat( L"Arial Black", NULL, DWRITE_FONT_WEIGHT_NORMAL, 
					DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0, L"en-us",&m_pTF5);

				if (SUCCEEDED(hr))
				{	// 换行模式（DWRITE_WORD_WRAPPING）设置为不换行
					hr = m_pTF5->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				}

				if (SUCCEEDED(hr))
				{
					m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTF5,&trim2);
				}

				if (SUCCEEDED(hr))
				{
					trim1.granularity = DWRITE_TRIMMING_GRANULARITY_WORD;
					trim1.delimiter = 1;
					trim1.delimiterCount = 10;
					hr = m_pTF5->SetTrimming(&trim1,trim2);
				}
			}

			return FALSE;

        }
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
	// 释放资源
	SafeRelease(&m_pFontBrush);
	SafeRelease(&m_pRectBrush);
	SafeRelease(&m_pTF1);
	SafeRelease(&m_pTF2);
	SafeRelease(&m_pTF3);
	SafeRelease(&m_pTF4);
	SafeRelease(&m_pTF5);

	SafeRelease(&m_pRT);
	SafeRelease(&m_pDWriteFactory);
	SafeRelease(&m_pD2DFactory);
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

// 绘制信息
wchar_t g_txt[128] = L"What damn are u looking at?";
int height = 100;
int width = 200;
int deltaY = 250;
int deltaX = 330;

HRESULT DemoApp::OnRender()
{
    HRESULT hr;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
		m_pRT->BeginDraw();

		m_pRT->SetTransform(D2D1::IdentityMatrix());
		m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(70.0f,60.0f));

		{
			m_pRT->DrawTextW(
				g_txt, 
				wcslen(g_txt), 
				m_pTF1,
				D2D1::RectF(0,0,width,height),
				m_pFontBrush);
			m_pRT->DrawRectangle(
				D2D1::RectF(0,0,width,height),
				m_pRectBrush
				);
		}

		{
			m_pRT->DrawTextW(
				g_txt, 
				wcslen(g_txt),
				m_pTF2,
				D2D1::RectF(deltaX,0,deltaX+width,height),
				m_pFontBrush);
			m_pRT->DrawRectangle(
				D2D1::RectF(deltaX,0,deltaX+width,height),
				m_pRectBrush
				);
		}

		{
			m_pRT->DrawTextW(
				g_txt, 
				wcslen(g_txt),
				m_pTF3,
				D2D1::RectF(deltaX*2,0,deltaX*2+width,height),
				m_pFontBrush);
			m_pRT->DrawRectangle(
				D2D1::RectF(deltaX*2,0,deltaX*2+width,height),
				m_pRectBrush
				);
		}

		{
			m_pRT->DrawTextW(
				g_txt, 
				wcslen(g_txt),
				m_pTF4,
				D2D1::RectF(0,deltaY,width,deltaY+height),
				m_pFontBrush);
			m_pRT->DrawRectangle(
				D2D1::RectF(0,deltaY,width,deltaY+height),
				m_pRectBrush
				);
		}

		{
			m_pRT->DrawTextW(
				g_txt, 
				wcslen(g_txt),
				m_pTF5,
				D2D1::RectF(deltaX,deltaY,deltaX+width,deltaY+height),
				m_pFontBrush);
			m_pRT->DrawRectangle(
				D2D1::RectF(deltaX,deltaY,deltaX+width,deltaY+height),
				m_pRectBrush
				);
		}

		HRESULT hr = m_pRT->EndDraw();

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