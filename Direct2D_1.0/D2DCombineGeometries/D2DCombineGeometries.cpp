#include "D2DCombineGeometries.h"

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
	m_pPathGeometryUnion	= NULL;
	m_pPathGeometryIntersect= NULL;
	m_pPathGeometryXOR		= NULL;
	m_pPathGeometryExclude	= NULL;
	m_pCircleGeometry1		= NULL;
	m_pCircleGeometry2		= NULL;
	m_pStrokeBrush			= NULL;
	m_pFillBrush			= NULL;
}

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBrush);
	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pPathGeometryUnion);
	SafeRelease(&m_pPathGeometryIntersect);
	SafeRelease(&m_pPathGeometryXOR);
	SafeRelease(&m_pPathGeometryExclude);
	SafeRelease(&m_pCircleGeometry1);
	SafeRelease(&m_pCircleGeometry2);
	SafeRelease(&m_pStrokeBrush);
	SafeRelease(&m_pFillBrush);
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
            L"D2DCombineGeometries",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(860.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
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

        // 创建render target
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );

		// 创建画刷
        if (SUCCEEDED(hr))
        {
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pBrush
                );
        }

		// 创建画刷
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Blue),
				&m_pStrokeBrush
				);
		}

		// 创建画刷
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue, 0.5),
				&m_pFillBrush
				);
		}

		// 创建字体格式
		if (SUCCEEDED(hr))
		{
			hr = m_pDWriteFactory->CreateTextFormat( 
				L"宋体",
				NULL, 
				DWRITE_FONT_WEIGHT_BOLD, 
				DWRITE_FONT_STYLE_NORMAL, 
				DWRITE_FONT_STRETCH_NORMAL, 
				13.0, 
				L"zh-cn",
				&m_pTextFormat);

			m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			m_pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		}
    }

    return CreateGeometryResources();
}

HRESULT DemoApp::CreateGeometryResources()
{
	HRESULT hr = S_OK;
	ID2D1GeometrySink *pGeometrySink = NULL;

	// Create the first ellipse geometry to merge.
	const D2D1_ELLIPSE circle1 = D2D1::Ellipse(
		D2D1::Point2F(75.0f, 75.0f),
		50.0f,
		50.0f
		);

	hr = m_pD2DFactory->CreateEllipseGeometry(
		circle1,
		&m_pCircleGeometry1
		);

	if (SUCCEEDED(hr))
	{
		// Create the second ellipse geometry to merge.
		const D2D1_ELLIPSE circle2 = D2D1::Ellipse(
			D2D1::Point2F(125.0f, 75.0f),
			50.0f,
			50.0f
			);

		hr = m_pD2DFactory->CreateEllipseGeometry(circle2, &m_pCircleGeometry2);
	}


	if (SUCCEEDED(hr))
	{
		//
		// Use D2D1_COMBINE_MODE_UNION to combine the geometries.
		//
		hr = m_pD2DFactory->CreatePathGeometry(&m_pPathGeometryUnion);

		if (SUCCEEDED(hr))
		{
			hr = m_pPathGeometryUnion->Open(&pGeometrySink);

			if (SUCCEEDED(hr))
			{
				hr = m_pCircleGeometry1->CombineWithGeometry(
					m_pCircleGeometry2,
					D2D1_COMBINE_MODE_UNION,
					NULL,
					NULL,
					pGeometrySink
					);
			}

			if (SUCCEEDED(hr))
			{
				hr = pGeometrySink->Close();
			}

			SafeRelease(&pGeometrySink);
		}
	}

	if (SUCCEEDED(hr))
	{
		//
		// Use D2D1_COMBINE_MODE_INTERSECT to combine the geometries.
		//
		hr = m_pD2DFactory->CreatePathGeometry(&m_pPathGeometryIntersect);

		if (SUCCEEDED(hr))
		{
			hr = m_pPathGeometryIntersect->Open(&pGeometrySink);

			if (SUCCEEDED(hr))
			{
				hr = m_pCircleGeometry1->CombineWithGeometry(
					m_pCircleGeometry2,
					D2D1_COMBINE_MODE_INTERSECT,
					NULL,
					NULL,
					pGeometrySink
					);
			}

			if (SUCCEEDED(hr))
			{
				hr = pGeometrySink->Close();
			}

			SafeRelease(&pGeometrySink);
		}
	}

	if (SUCCEEDED(hr))
	{
		//
		// Use D2D1_COMBINE_MODE_XOR to combine the geometries.
		//
		hr = m_pD2DFactory->CreatePathGeometry(&m_pPathGeometryXOR);

		if (SUCCEEDED(hr))
		{
			hr = m_pPathGeometryXOR->Open(&pGeometrySink);

			if (SUCCEEDED(hr))
			{
				hr = m_pCircleGeometry1->CombineWithGeometry(
					m_pCircleGeometry2,
					D2D1_COMBINE_MODE_XOR,
					NULL,
					NULL,
					pGeometrySink
					);
			}

			if (SUCCEEDED(hr))
			{
				hr = pGeometrySink->Close();
			}

			SafeRelease(&pGeometrySink);
		}
	}

	if (SUCCEEDED(hr))
	{
		//
		// Use D2D1_COMBINE_MODE_EXCLUDE to combine the geometries.
		//
		hr = m_pD2DFactory->CreatePathGeometry(&m_pPathGeometryExclude);

		if (SUCCEEDED(hr))
		{
			hr = m_pPathGeometryExclude->Open(&pGeometrySink);

			if (SUCCEEDED(hr))
			{
				hr = m_pCircleGeometry1->CombineWithGeometry(
					m_pCircleGeometry2,
					D2D1_COMBINE_MODE_EXCLUDE,
					NULL,
					NULL,
					pGeometrySink
					);
			}

			if (SUCCEEDED(hr))
			{
				hr = pGeometrySink->Close();
			}

			SafeRelease(&pGeometrySink);
		}
	}

	return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBrush);
	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pCircleGeometry1);
	SafeRelease(&m_pCircleGeometry2);
	SafeRelease(&m_pPathGeometryUnion);
	SafeRelease(&m_pPathGeometryIntersect);
	SafeRelease(&m_pPathGeometryXOR);
	SafeRelease(&m_pPathGeometryExclude);
	SafeRelease(&m_pStrokeBrush);
	SafeRelease(&m_pFillBrush);
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

        m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(20, 100));
		m_pRT->DrawTextW(L"The geometries before combining", wcslen(L"The geometries before combining"), m_pTextFormat, D2D1::RectF(0,130+0,200,130+0), m_pBrush);
		m_pRT->DrawTextW(L"组合之前的几何图形", wcslen(L"组合之前的几何图形"), m_pTextFormat, D2D1::RectF(0,130+20,200,130+20), m_pBrush);
		m_pRT->FillGeometry(m_pCircleGeometry1, m_pFillBrush);
		m_pRT->DrawGeometry(m_pCircleGeometry1, m_pStrokeBrush);
		m_pRT->FillGeometry(m_pCircleGeometry2, m_pFillBrush);
		m_pRT->DrawGeometry(m_pCircleGeometry2, m_pStrokeBrush);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

		// 绘制
		// D2D1_COMBINE_MODE_UNION
		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(300, 0));
		m_pRT->DrawTextW(L"D2D1_COMBINE_MODE_UNION", wcslen(L"D2D1_COMBINE_MODE_UNION"), m_pTextFormat, D2D1::RectF(0,130+0,200,130+0), m_pBrush);
		m_pRT->DrawTextW(L"并集合并A+B", wcslen(L"并集合并A+B"), m_pTextFormat, D2D1::RectF(0,130+20,200,130+20), m_pBrush);
		m_pRT->FillGeometry(m_pPathGeometryUnion, m_pFillBrush);
		m_pRT->DrawGeometry(m_pPathGeometryUnion, m_pStrokeBrush);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

		// D2D1_COMBINE_MODE_INTERSECT
		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(550, 0));
		m_pRT->DrawTextW(L"D2D1_COMBINE_MODE_INTERSECT", wcslen(L"D2D1_COMBINE_MODE_INTERSECT"), m_pTextFormat, D2D1::RectF(0,130+0,200,130+0), m_pBrush);
		m_pRT->DrawTextW(L"交集合并", wcslen(L"交集合并"), m_pTextFormat, D2D1::RectF(0,130+20,200,130+20), m_pBrush);
		m_pRT->FillGeometry(m_pPathGeometryIntersect, m_pFillBrush);
		m_pRT->DrawGeometry(m_pPathGeometryIntersect, m_pStrokeBrush);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

		// D2D1_COMBINE_MODE_XOR
		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(300, 200));
		m_pRT->DrawTextW(L"D2D1_COMBINE_MODE_XOR", wcslen(L"D2D1_COMBINE_MODE_XOR"), m_pTextFormat, D2D1::RectF(0,130+0,200,130+0), m_pBrush);
		m_pRT->DrawTextW(L"对称差(A-B) + (B-A)", wcslen(L"对称差(A-B) + (B-A)"), m_pTextFormat, D2D1::RectF(0,130+20,200,130+20), m_pBrush);
		m_pRT->FillGeometry(m_pPathGeometryXOR, m_pFillBrush);
		m_pRT->DrawGeometry(m_pPathGeometryXOR, m_pStrokeBrush);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

		// D2D1_COMBINE_MODE_EXCLUDE
		m_pRT->SetTransform(D2D1::Matrix3x2F::Translation(550, 200));
		m_pRT->DrawTextW(L"D2D1_COMBINE_MODE_EXCLUDE", wcslen(L"D2D1_COMBINE_MODE_EXCLUDE"), m_pTextFormat, D2D1::RectF(0,130+0,200,130+0), m_pBrush);
		m_pRT->DrawTextW(L"差集A-B", wcslen(L"差集A-B"), m_pTextFormat, D2D1::RectF(0,130+20,200,130+20), m_pBrush);
		m_pRT->FillGeometry(m_pPathGeometryExclude, m_pFillBrush);
		m_pRT->DrawGeometry(m_pPathGeometryExclude, m_pStrokeBrush);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

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