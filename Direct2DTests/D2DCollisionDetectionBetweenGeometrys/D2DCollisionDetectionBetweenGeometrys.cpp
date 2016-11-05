#include "D2DCollisionDetectionBetweenGeometrys.h"

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
	, m_pRT(NULL)
	, m_pPentagram(NULL)
	, m_pRectangle(NULL)
	, m_pCircle(NULL)
	, m_pRoundedRectangle(NULL)
	, m_pTriangle(NULL)
	, m_pBrush(NULL)
{
}

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pPentagram);
	SafeRelease(&m_pRectangle);
	SafeRelease(&m_pCircle);
	SafeRelease(&m_pRoundedRectangle);
	SafeRelease(&m_pTriangle);
    SafeRelease(&m_pBrush);
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
            static_cast<UINT>(ceil(480 * dpiX / 96.f)),
            static_cast<UINT>(ceil(480 * dpiY / 96.f)),
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

	// 创建矩形
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreateRectangleGeometry(
			D2D1::RectF(60,160,150,200),
			&m_pRectangle);
	}

	// 创建圆形
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreateEllipseGeometry(
			D2D1::Ellipse(D2D1::Point2F(250,200), 50, 50),
			&m_pCircle);
	}

	// 创建圆角矩形
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreateRoundedRectangleGeometry(
			D2D1::RoundedRect(D2D1::RectF(50,20,450,400), 10, 10),
			&m_pRoundedRectangle);
	}

	// 创建三角形
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreatePathGeometry(&m_pTriangle);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pTriangle->Open(&pSink);
	}
	if (SUCCEEDED(hr))
	{
		pSink->BeginFigure(
			D2D1::Point2F(450, 300),
			D2D1_FIGURE_BEGIN_FILLED
			);

		const D2D1_POINT_2F ptTriangle[] = {
			{  350,	250	}, 
			{  400,	200	}, 
			{  450, 300 }
		};

		pSink->AddLines(ptTriangle, 3);

		pSink->EndFigure(D2D1_FIGURE_END_OPEN);

		hr = pSink->Close();
	}

	// 创建五角星
    if (SUCCEEDED(hr))
    {
        hr = m_pD2DFactory->CreatePathGeometry(&m_pPentagram);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pPentagram->Open(&pSink);
    }
    if (SUCCEEDED(hr))
    {
        pSink->BeginFigure(
            D2D1::Point2F(0.0f+250, -200.0f+250),
            D2D1_FIGURE_BEGIN_FILLED
            );

        const D2D1_POINT_2F ptPentagram[] = {
			{  50.0f+250,	-100.0f+250	}, 
			{ 170.0f+250,	-100.0f+250	}, 
			{  70.0f+250,	 -30.0f+250 },
			{ 130.0f+250,	  90.0f+250	},
			{   0.0f+250,	  15.0f+250	},
			{-130.0f+250,	  90.0f+250	},
			{ -70.0f+250,	 -30.0f+250	},
			{-170.0f+250,	-100.0f+250	},
			{ -50.0f+250,	-100.0f+250	},
			{   0.0f+250,	-200.0f+250	}
		};

        pSink->AddLines(ptPentagram, 10);

        pSink->EndFigure(D2D1_FIGURE_END_OPEN);

        hr = pSink->Close();
    }

    SafeRelease(&pSink);

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
        if (SUCCEEDED(hr))
        {
            // Create a yellow brush.
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::White),
                &m_pBrush
                );
        }
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBrush);
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

// 检测两个几何图形是否碰撞
D2D1_GEOMETRY_RELATION IntersectsWithGeometry(ID2D1Factory* pD2DFacytory,
	ID2D1Geometry* pGeometry1, D2D1_MATRIX_3X2_F& transMatrix1,
	ID2D1Geometry* pGeometry2, D2D1_MATRIX_3X2_F& transMatrix2)
{
	D2D1_GEOMETRY_RELATION relation;
	ID2D1TransformedGeometry* transGeometry = NULL;

	HRESULT hr = pD2DFacytory->CreateTransformedGeometry(
		pGeometry1,
		transMatrix1,
		&transGeometry
		);

	if (SUCCEEDED(hr))
	{
		hr = transGeometry->CompareWithGeometry(
			pGeometry2,
			transMatrix2,
			&relation
			);

		if (SUCCEEDED(hr))
		{
			SafeRelease(&transGeometry);
			return relation;

		}
		SafeRelease(&transGeometry);
	}

	return D2D1_GEOMETRY_RELATION_UNKNOWN;
}

void DemoApp::SelectColorForGeometry(ID2D1Geometry* pGeometry, D2D1_MATRIX_3X2_F& transMatrix)
{
	Assert(m_pBrush!=NULL);	// 画刷不能为NULL

	D2D1_GEOMETRY_RELATION relation = IntersectsWithGeometry(
		m_pD2DFactory, m_pPentagram, D2D1::Matrix3x2F::Identity(),
		pGeometry, transMatrix);
	//relation = D2D1_GEOMETRY_RELATION_UNKNOWN;
	switch(relation)
	{
	case D2D1_GEOMETRY_RELATION_DISJOINT:		// 不相交	红
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		break;
	case D2D1_GEOMETRY_RELATION_IS_CONTAINED:	// 属于		橙
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Orange));
		break;
	case D2D1_GEOMETRY_RELATION_CONTAINS:		// 包含		黄
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Yellow));
		break;
	case D2D1_GEOMETRY_RELATION_OVERLAP:		// 相交		绿
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
		break;
	case D2D1_GEOMETRY_RELATION_UNKNOWN:		// 未知
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
		break;
	}
}

HRESULT DemoApp::OnRender()
{
    HRESULT hr;

    hr = CreateDeviceResources();
    if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
		D2D1_SIZE_F rtSize = m_pRT->GetSize();
		float minWidthHeightScale = min(rtSize.width, rtSize.height) / 512;

		D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(
			minWidthHeightScale,
			minWidthHeightScale
			);

		D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(
			rtSize.width / 2,
			rtSize.height / 2
			);

        // 开始绘制
        m_pRT->BeginDraw();

        // 重置呈现器的变换
        m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

        // 清除呈现器上的内容
        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

		// 绘制圆角矩形
		SelectColorForGeometry(m_pRoundedRectangle, D2D1::Matrix3x2F::Identity());
		m_pRT->FillGeometry(m_pRoundedRectangle, m_pBrush);
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		m_pRT->DrawGeometry(m_pRoundedRectangle, m_pBrush);

		// 绘制五角星
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::DeepSkyBlue,0.6));
        m_pRT->FillGeometry(m_pPentagram, m_pBrush);
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		m_pRT->DrawGeometry(m_pPentagram, m_pBrush);

		// 绘制三角形
		SelectColorForGeometry(m_pTriangle, D2D1::Matrix3x2F::Identity());
		m_pRT->FillGeometry(m_pTriangle, m_pBrush);
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		m_pRT->DrawGeometry(m_pTriangle, m_pBrush);

		// 绘制圆形
		SelectColorForGeometry(m_pCircle, D2D1::Matrix3x2F::Identity());
		m_pRT->FillGeometry(m_pCircle, m_pBrush);
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		m_pRT->DrawGeometry(m_pCircle, m_pBrush);

		// 绘制矩形
		SelectColorForGeometry(m_pRectangle, D2D1::Matrix3x2F::Identity());
		m_pRT->FillGeometry(m_pRectangle, m_pBrush);
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		m_pRT->DrawGeometry(m_pRectangle, m_pBrush);

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