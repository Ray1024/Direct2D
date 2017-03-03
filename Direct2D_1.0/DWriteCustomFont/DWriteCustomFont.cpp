#include "DWriteCustomFont.h"

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
    m_pSolidBrushOutline(NULL),
	m_pLinearGradientBrush(NULL),
	m_pGeometrySink(NULL),
	m_pPathGeometry(NULL)
{
}

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pSolidBrushOutline);
	SafeRelease(&m_pLinearGradientBrush);
	SafeRelease(&m_pGeometrySink);
	SafeRelease(&m_pPathGeometry);
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
            static_cast<UINT>(ceil(800.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(300.f * dpiY / 96.f)),
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

	IDWriteFontFace* pFontFace = NULL;
	IDWriteFontFile* pFontFiles = NULL;

	if (SUCCEEDED(hr))
	{
		WCHAR strPath[MAX_PATH] = {0};
		WCHAR fontPath[MAX_PATH] = {0};

		// 获取可执行文件路径
		::GetModuleFileName(0, strPath, MAX_PATH);
		::PathRemoveFileSpec(strPath);

		// 将字体文件转换成字体文件路径
		wsprintf(fontPath, L"%s\\%s", strPath, L"Ruzicka TypeK.ttf");

		hr = m_pDWriteFactory->CreateFontFileReference(
			fontPath,
			NULL,
			&pFontFiles);
	}

	// 创建FontFace
	IDWriteFontFile* fontFileArray[] = {pFontFiles};
	m_pDWriteFactory->CreateFontFace(
		DWRITE_FONT_FACE_TYPE_TRUETYPE,
		1, // file count
		fontFileArray,
		0,
		DWRITE_FONT_SIMULATIONS_NONE,
		&pFontFace
		);

	char* text = "DriteWrite custom font";
	UINT lengthText = strlen(text);

	// 用FontFace创建点集合
	UINT* pCodePoints = new UINT[lengthText];
	UINT16* pGlyphIndices = new UINT16[lengthText];
	ZeroMemory(pCodePoints, sizeof(UINT) * lengthText);
	ZeroMemory(pGlyphIndices, sizeof(UINT16) * lengthText);
	for(int i=0; i<lengthText; ++i)
	{
		pCodePoints[i] = text[i];
	}
	pFontFace->GetGlyphIndicesW(pCodePoints, lengthText, pGlyphIndices);

	// 用文本的点集合创建路径图形
	m_pD2DFactory->CreatePathGeometry(&m_pPathGeometry);

	m_pPathGeometry->Open((ID2D1GeometrySink**)&m_pGeometrySink);

	pFontFace->GetGlyphRunOutline(
		(48.0f/72.0f)*96.0f, 
		pGlyphIndices, 
		NULL,
		NULL,
		lengthText,
		FALSE,
		FALSE,
		m_pGeometrySink);

	m_pGeometrySink->Close();

	if(pCodePoints)
	{
		delete [] pCodePoints;
		pCodePoints = NULL;
	}

	if(pGlyphIndices)
	{
		delete [] pGlyphIndices;
		pGlyphIndices = NULL;
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

        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

        // 创建呈现器
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );

		ID2D1GradientStopCollection* pGradientStops = NULL;

		D2D1_GRADIENT_STOP gradientStops[3];
		gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Blue, 1);
		gradientStops[0].position = 0.0f;
		gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Purple, 1);
		gradientStops[1].position = 0.5f;
		gradientStops[2].color = D2D1::ColorF(D2D1::ColorF::Red, 1);
		gradientStops[2].position = 1.0f;

		m_pRT->CreateGradientStopCollection(
			gradientStops,
			3,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&pGradientStops
			);

		m_pRT->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
			D2D1::Point2F(0.0, -30.0),
			D2D1::Point2F(0.0, 0.0)),
			pGradientStops,
			&m_pLinearGradientBrush
			);

		m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Plum),
			&m_pSolidBrushOutline
			);
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pSolidBrushOutline);
	SafeRelease(&m_pLinearGradientBrush);
	SafeRelease(&m_pGeometrySink);
	SafeRelease(&m_pPathGeometry);
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

		// 转换文本在窗口的显示位置
		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(10.0f,60.0f));

		// 绘制文本外框
		m_pRT->DrawGeometry(m_pPathGeometry, m_pSolidBrushOutline, 3.0f);
		// 填充文本
		m_pRT->FillGeometry(m_pPathGeometry, m_pLinearGradientBrush);
		
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