// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "D2DBezierToWave.h"

#define M_PI 3.1415926535898f

void GetFirstControlPoints(  
	__in const std::vector<FLOAT>& rhs,
	__out std::vector<FLOAT>& x )  
{  
	assert(rhs.size()==x.size());  
	int n = rhs.size();  
	std::vector<FLOAT> tmp(n);    // Temp workspace.  

	FLOAT b = 2.0f;  
	x[0] = rhs[0] / b;  
	for (int i = 1; i < n; i++) // Decomposition and forward substitution.  
	{  
		tmp[i] = 1 / b;  
		b = (i < n-1 ? 4.0f : 3.5f) - tmp[i];  
		x[i] = (rhs[i] - x[i-1]) / b;  
	}  
	for (int i = 1; i < n; i++)  
	{  
		x[n-i-1] -= tmp[n-i] * x[n-i]; // Back substitution.  
	}  
}  

void GetCurveControlPoints(  
	__in const std::vector<D2D1_POINT_2F>& knots,  
	__out std::vector<D2D1_POINT_2F>& firstCtrlPt,   
	__out std::vector<D2D1_POINT_2F>& secondCtrlPt )  
{  
	assert( (firstCtrlPt.size()==secondCtrlPt.size()) && (knots.size()==firstCtrlPt.size()+1) );  

	int n = knots.size()-1;  
	assert(n>=1);  

	if (n == 1)  
	{   
		// Special case: Bezier curve should be a straight line.  
		// 3P1 = 2P0 + P3  
		firstCtrlPt[0].x = (2 * knots[0].x + knots[1].x) / 3.0f;  
		firstCtrlPt[0].y = (2 * knots[0].y + knots[1].y) / 3.0f;  

		// P2 = 2P1 – P0  
		secondCtrlPt[0].x = 2 * firstCtrlPt[0].x - knots[0].x;  
		secondCtrlPt[0].y = 2 * firstCtrlPt[0].y - knots[0].y;  
		return;  
	}  

	// Calculate first Bezier control points  
	// Right hand side vector  
	std::vector<FLOAT> rhs(n);  

	// Set right hand side X values  
	for (int i = 1; i < (n-1); ++i)  
	{  
		rhs[i] = 4 * knots[i].x + 2 * knots[i+1].x;  
	}  
	rhs[0] = knots[0].x + 2 * knots[1].x;  
	rhs[n-1] = (8 * knots[n-1].x + knots[n].x) / 2.0f;  
	// Get first control points X-values  
	std::vector<FLOAT> x(n);  
	GetFirstControlPoints(rhs,x);  

	// Set right hand side Y values  
	for (int i = 1; i < (n-1); ++i)  
	{  
		rhs[i] = 4 * knots[i].y + 2 * knots[i+1].y;  
	}  
	rhs[0] = knots[0].y + 2 * knots[1].y;  
	rhs[n-1] = (8 * knots[n-1].y + knots[n].y) / 2.0f;  
	// Get first control points Y-values  
	std::vector<FLOAT> y(n);  
	GetFirstControlPoints(rhs,y);  

	// Fill output arrays.  
	 for (int i = 0; i < n; ++i)  
	{  
		// First control point  
		firstCtrlPt[i] = D2D1::Point2F(x[i],y[i]);  
		// Second control point  
		if (i < (n-1))  
		{  
			secondCtrlPt[i] = D2D1::Point2F(2 * knots[i+1].x - x[i+1], 2*knots[i+1].y-y[i+1]);  
		}  
		else
		{  
			secondCtrlPt[i] = D2D1::Point2F((knots[n].x + x[n-1])/2, (knots[n].y+y[n-1])/2);  
		}  
	}  
}  

HRESULT CreateBezierSpline(  
	__in ID2D1Factory* pD2dFactory,   
	__in const std::vector<D2D1_POINT_2F>& points,  
	__out ID2D1PathGeometry** ppPathGeometry )  
{  
	assert(points.size()>1);  

	int n = points.size();  
	std::vector<D2D1_POINT_2F> firstCtrlPt(n-1);  
	std::vector<D2D1_POINT_2F> secondCtrlPt(n-1);  
	GetCurveControlPoints(points,firstCtrlPt,secondCtrlPt);  

	HRESULT hr = pD2dFactory->CreatePathGeometry(ppPathGeometry);  

	if (FAILED(hr))  
		return hr;  

	ID2D1GeometrySink* pSink;  
	hr = (*ppPathGeometry)->Open(&pSink);  

	if (SUCCEEDED(hr))  
	{  
		pSink->BeginFigure(points[0],D2D1_FIGURE_BEGIN_FILLED);  
		for (int i=1;i<n;i++)  
			pSink->AddBezier(     /*第一个控制点 第二个控制点 终点*/
			D2D1::BezierSegment(firstCtrlPt[i-1],secondCtrlPt[i-1],points[i]));
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		pSink->Close();  
	}  
	return hr;  
}  

/******************************************************************
*                                                                 *
*  WinMain                                                        *
*                                                                 *
*  Application entrypoint                                         *
*                                                                 *
******************************************************************/

int WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
    )
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
*                                                                 *
*  DemoApp::DemoApp constructor                                   *
*                                                                 *
*  Initialize member data                                         *
*                                                                 *
******************************************************************/

DemoApp::DemoApp() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
    m_pRT(NULL),
    m_pPathGeometry(NULL),
    m_pRedBrush(NULL)
{
}

/******************************************************************
*                                                                 *
*  DemoApp::~DemoApp destructor                                   *
*                                                                 *
*  Release resources.                                             *
*                                                                 *
******************************************************************/

DemoApp::~DemoApp()
{
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pPathGeometry);
    SafeRelease(&m_pRedBrush);
}

/******************************************************************
*                                                                 *
*  DemoApp::Initialize                                            *
*                                                                 *
*  Create application window and device-independent resources     *
*                                                                 *
******************************************************************/

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
            L"D2D Simple Path Animation Sample",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            512.f * dpiX / 96.f,
            512.f * dpiY / 96.f,
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
			hr = CreateMyResources();
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceIndependentResources                      *
*                                                                 *
*  This method is used to create resources which are not bound    *
*  to any device. Their lifetime effectively extends for the      *
*  duration of the app.                                           *
*                                                                 *
******************************************************************/

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::CreateMyResources									  *
*                                                                 *
******************************************************************/
HRESULT DemoApp::CreateMyResources()
{
	HRESULT hr = S_OK;

	if (m_pD2DFactory)
	{
		const int ptCount = 3;  
		FLOAT width = 400;
		FLOAT height = 200;  

		FLOAT factor = static_cast<FLOAT>(4.0f*M_PI/width);  
		FLOAT x = -width*0.5f, y = 0, dx = width/ptCount;  
		for (int i=0;i<ptCount+1;i++)  
		{  
			y = height*sin(factor*x);  
			m_Points.push_back(D2D1::Point2F(x,y));  
			x += dx;  
		}

		// create Bezier spline  
		CreateBezierSpline(m_pD2DFactory,m_Points,&m_pPathGeometry); 
	}

	return hr;
}


/******************************************************************
*                                                                 *
*  DemoApp::CreateDeviceResources                                 *
*                                                                 *
*  This method creates resources which are bound to a particular  *
*  D3D device. It's all centralized here, in case the resources   *
*  need to be recreated in case of D3D device loss (eg. display   *
*  change, remoting, removal of video card, etc).                 *
*                                                                 *
******************************************************************/

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
            // Create a red brush.
            hr = m_pRT->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LawnGreen),
                &m_pRedBrush
                );
        }
    }

    return hr;
}

/******************************************************************
*                                                                 *
*  DemoApp::DiscardDeviceResources                                *
*                                                                 *
*  Discard device-specific resources which need to be recreated   *
*  when a D3D device is lost                                      *
*                                                                 *
******************************************************************/

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pRedBrush);
}

/******************************************************************
*                                                                 *
*  DemoApp::RunMessageLoop                                        *
*                                                                 *
*  Main window message loop                                       *
*                                                                 *
******************************************************************/

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/******************************************************************
*                                                                 *
*  DemoApp::OnRender                                              *
*                                                                 *
*  Called whenever the application needs to display the client    *
*  window. This method draws a single frame of animated content   *
*                                                                 *
*  Note that this function will not render anything if the window *
*  is occluded (e.g. when the screen is locked).                  *
*  Also, this function will automatically discard device-specific *
*  resources if the D3D device disappears during function         *
*  invocation, and will recreate the resources the next time it's *
*  invoked.                                                       *
*                                                                 *
******************************************************************/

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

        // Prepare to draw.
        m_pRT->BeginDraw();

        // Reset to identity transform
        m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

        //clear the render target contents
        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        //center the path
        m_pRT->SetTransform(scale * translation);

		m_pRedBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Green));  
        //draw the path in red
        m_pRT->DrawGeometry(m_pPathGeometry, m_pRedBrush, 5);

		// point mark
// 		m_pRedBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));  
// 		for (auto p=m_Points.cbegin();p!=m_Points.cend();p++)  
// 		{  
// 			m_pRT->FillRectangle(D2D1::RectF((*p).x-1,(*p).y-1,(*p).x+1,(*p).y+1), m_pRedBrush);
// 		}

        // Commit the drawing operations.
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

/******************************************************************
*                                                                 *
*  DemoApp::OnResize                                              *
*                                                                 *
*  If the application receives a WM_SIZE message, this method     *
*  resize the render target appropriately.                        *
*                                                                 *
******************************************************************/

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

/******************************************************************
*                                                                 *
*  DemoApp::WndProc                                               *
*                                                                 *
*  Window message handler                                         *
*                                                                 *
******************************************************************/

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
