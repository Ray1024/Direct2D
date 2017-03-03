#include "D2DGeometryCollisionDetectionWithPt.h"

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
    m_pRT(NULL),
    m_pObjectGeometry(NULL),
    m_pBrush(NULL)
{
}

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pObjectGeometry);
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
            L"D2DGeometryCollisionDetectionWithPt",
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

    if (SUCCEEDED(hr))
    {
        // 创建路径几何图形
        hr = m_pD2DFactory->CreatePathGeometry(&m_pObjectGeometry);
    }
    if (SUCCEEDED(hr))
    {
        // 使用geometrysink为pathgeometry添加图像
        hr = m_pObjectGeometry->Open(&pSink);
    }
    if (SUCCEEDED(hr))
    {
		// 添加一个五角星的图形
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

// 检测点是否在几何图形内
bool PtInGeometry(ID2D1Geometry* pGeometry, D2D1_MATRIX_3X2_F& transMatrix, D2D1_POINT_2F& pt)
{
	BOOL contain = FALSE;
	HRESULT hr = S_OK;

	hr = pGeometry->FillContainsPoint(
		pt,             // 检测的点
		&transMatrix,   // 几何图形上应用的变换
		&contain);      // 用来接收 几何图形是否包含pt
	if (SUCCEEDED(hr) && contain)
	{
		return true;
	}

	return false;
}

// 检测点是否在几何图形边框上
bool PtInGeometryBorder(ID2D1Geometry* pGeometry, float strokeWidth, D2D1_MATRIX_3X2_F& transMatrix, D2D1_POINT_2F& pt)
{
	BOOL contain = FALSE;
	HRESULT hr = S_OK;

	hr = pGeometry->StrokeContainsPoint(
		pt,             // 检测的点
		strokeWidth,	// 边框宽度
		NULL,			// 线条
		&transMatrix,   // 几何图形上应用的变换
		&contain);      // 用来接收 几何图形是否包含pt
	if (SUCCEEDED(hr) && contain)
	{
		return true;
	}

	return false;
}

void DemoApp::RenderPoint(D2D1_POINT_2F& pt)
{
	// 绘制点（由于Direct2D中没有直接绘制点的方法，使用边长为0的矩形代替点）
	// 点不在五角星内，为绿色；点在五角星边上，为紫色；点在五角星内部，为红色
	if (PtInGeometry(m_pObjectGeometry, D2D1::Matrix3x2F::Identity(), pt))
	{
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
	} else if (PtInGeometryBorder(m_pObjectGeometry, 50, D2D1::Matrix3x2F::Identity(), pt))
	{
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Purple));
	} else
	{
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
	}

	m_pRT->DrawRectangle(D2D1::RectF(pt.x,pt.y,pt.x,pt.y), m_pBrush, 5);
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

		// 绘制五角星（路径几何图形）
		// 填充
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Silver));
        m_pRT->FillGeometry(m_pObjectGeometry, m_pBrush);
		// 描边细边
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		m_pRT->DrawGeometry(m_pObjectGeometry, m_pBrush);
		// 描边宽边
		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Pink, 0.5));
		m_pRT->DrawGeometry(m_pObjectGeometry, m_pBrush,50);
		
		// 绘制三个不同位置的点
		RenderPoint(D2D1::Point2F(250,200));
		RenderPoint(D2D1::Point2F(250,280));
		RenderPoint(D2D1::Point2F(250,350));

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