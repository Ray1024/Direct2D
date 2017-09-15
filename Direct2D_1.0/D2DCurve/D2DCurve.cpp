#include "D2DCurve.h"
#include <algorithm>
#include <time.h>

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

//////////////////////////////////////////////////////////////////////////

struct CvPoint 
{
	CvPoint(float _x = 0, float _y = 0) : x(_x), y(_y) {}

	bool operator==(const CvPoint& right) const
	{
		return this->x == right.x && this->y == right.y;
	}

	bool operator<(const CvPoint& right) const
	{
		//return this->x < right.x;
		return (x < right.x || (x == right.x && y < right.y));
	}

	float x;
	float y;
};

int Random(int a, int b)
{
	return (rand() % (b - a)) + a;
}

std::vector<CvPoint> g_originPoint;	// 输入的点集
std::vector<CvPoint> g_curvePoint;	// 输出的曲线的点集

//三次贝塞尔曲线
float bezier3funcX(float uu,CvPoint *controlP){
	float part0 = controlP[0].x * uu * uu * uu;
	float part1 = 3 * controlP[1].x * uu * uu * (1 - uu);
	float part2 = 3 * controlP[2].x * uu * (1 - uu) * (1 - uu);
	float part3 = controlP[3].x * (1 - uu) * (1 - uu) * (1 - uu);   
	return part0 + part1 + part2 + part3; 
}    
float bezier3funcY(float uu,CvPoint *controlP){
	float part0 = controlP[0].y * uu * uu * uu;
	float part1 = 3 * controlP[1].y * uu * uu * (1 - uu);
	float part2 = 3 * controlP[2].y * uu * (1 - uu) * (1 - uu);
	float part3 = controlP[3].y * (1 - uu) * (1 - uu) * (1 - uu);   
	return part0 + part1 + part2 + part3; 
} 

void createCurve(std::vector<CvPoint> &originPoint, std::vector<CvPoint> &curvePoint)
{
	//控制点收缩系数 ，经调试0.6较好，CvPoint是opencv的，可自行定义结构体(x,y)
	float scale = 0.6;

	int originCount = originPoint.size();

	std::vector<CvPoint> midpoints;
	for(int i = 0 ;i < originCount ; i++)
	{    
		midpoints.push_back(CvPoint());
	}    
	//生成中点     
	for(int i = 0 ;i < originCount ; i++)
	{    
		int nexti = (i + 1) % originCount;
		midpoints[i].x = (originPoint[i].x + originPoint[nexti].x)/2.0;
		midpoints[i].y = (originPoint[i].y + originPoint[nexti].y)/2.0;
	}

	//平移中点
	std::vector<CvPoint> extrapoints;

	for(int i = 0 ;i < originCount*2 ; i++)
	{    
		extrapoints.push_back(CvPoint());
	}
	for(int i = 0 ;i < originCount ; i++)
	{
		int nexti = (i + 1) % originCount;
		int backi = (i + originCount - 1) % originCount;
		CvPoint midinmid;
		midinmid.x = (midpoints[i].x + midpoints[backi].x)/2.0;
		midinmid.y = (midpoints[i].y + midpoints[backi].y)/2.0;
		int offsetx = originPoint[i].x - midinmid.x;
		int offsety = originPoint[i].y - midinmid.y;
		int extraindex = 2 * i;
		extrapoints[extraindex].x = midpoints[backi].x + offsetx;
		extrapoints[extraindex].y = midpoints[backi].y + offsety;
		//朝 originPoint[i]方向收缩 
		int addx = (extrapoints[extraindex].x - originPoint[i].x) * scale;
		int addy = (extrapoints[extraindex].y - originPoint[i].y) * scale;
		extrapoints[extraindex].x = originPoint[i].x + addx;
		extrapoints[extraindex].y = originPoint[i].y + addy;

		int extranexti = (extraindex + 1)%(2 * originCount);
		extrapoints[extranexti].x = midpoints[i].x + offsetx;
		extrapoints[extranexti].y = midpoints[i].y + offsety;
		//朝 originPoint[i]方向收缩 
		addx = (extrapoints[extranexti].x - originPoint[i].x) * scale;
		addy = (extrapoints[extranexti].y - originPoint[i].y) * scale;
		extrapoints[extranexti].x = originPoint[i].x + addx;
		extrapoints[extranexti].y = originPoint[i].y + addy;

	}    

	CvPoint controlPoint[4];
	//生成4控制点，产生贝塞尔曲线
	for(int i = 0 ;i < originCount ; i++){
		controlPoint[0] = originPoint[i];
		int extraindex = 2 * i;
		controlPoint[1] = extrapoints[extraindex + 1];
		int extranexti = (extraindex + 2) % (2 * originCount);
		controlPoint[2] = extrapoints[extranexti];
		int nexti = (i + 1) % originCount;
		controlPoint[3] = originPoint[nexti];    
		float u = 1;
		while(u >= 0){
			int px = bezier3funcX(u,controlPoint);
			int py = bezier3funcY(u,controlPoint);
			//u的步长决定曲线的疏密
			u -= 0.0005;
			CvPoint tempP = CvPoint(px,py);
			//存入曲线点 
			curvePoint.push_back(tempP);
		}    
	}

	// 删除结束点到开始点之间的点集，让曲线不闭合

	CvPoint pt = originPoint.back();
	std::vector <CvPoint>::iterator itor = curvePoint.end() - 1;
	while(itor != curvePoint.begin())	
	{
		if (((*itor).x == pt.x && (*itor).y == pt.y))
		{
			break;
		}

		itor--;
	}

	int itor_index = 0;
	while(itor != curvePoint.end())
	{
		itor_index = itor - curvePoint.begin(); 
		curvePoint.erase(itor); 
		itor = curvePoint.begin() + itor_index; 
	}

	// 删除附近重复的点
	std::sort(curvePoint.begin(), curvePoint.end());

	curvePoint.erase(unique(curvePoint.begin(), curvePoint.end()), curvePoint.end());  
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
            L"D2DCurve | 按任意键盘随机生成新的曲线",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(960.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(640+50.f * dpiY / 96.f)),
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

	srand((unsigned int)time(0));

// 	for (int i=0; i<10; i++)
// 	{
// 		g_originPoint.push_back(CvPoint(rand()%300, rand()%300));
// 	}
// 
// 	createCurve(g_originPoint, g_curvePoint);

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
                D2D1::ColorF(D2D1::ColorF::Red),
                &m_pBrush
                );
        }

		// 创建字体格式
		if (SUCCEEDED(hr))
		{
			hr = m_pDWriteFactory->CreateTextFormat( 
				L"Edwardian Script ITC",
				NULL, 
				DWRITE_FONT_WEIGHT_BOLD, 
				DWRITE_FONT_STYLE_NORMAL, 
				DWRITE_FONT_STRETCH_NORMAL, 
				60.0, 
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
		// 生成随机新点
		g_originPoint.clear();
		g_curvePoint.clear();

		for (int i=0; i<10; i++)
		{
			g_originPoint.push_back(CvPoint(Random(100, 860), Random(100, 640-100)));
		}
		createCurve(g_originPoint, g_curvePoint);

        // 开始绘制
        m_pRT->BeginDraw();

        m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRT->Clear();

		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::LawnGreen));
		for(int i=0; i<g_curvePoint.size(); i++)
		{
			m_pRT->DrawRectangle(D2D1::RectF(g_curvePoint[i].x, g_curvePoint[i].y, g_curvePoint[i].x, g_curvePoint[i].y),m_pBrush, 2);
		}

		m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		for(int i=0; i<g_originPoint.size(); i++)
		{
			m_pRT->DrawRectangle(D2D1::RectF(g_originPoint[i].x, g_originPoint[i].y, g_originPoint[i].x, g_originPoint[i].y),m_pBrush, 8);
		}
		
        hr = m_pRT->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
    }

    //InvalidateRect(m_hwnd, NULL, FALSE);

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
			case WM_KEYUP:
				{
					InvalidateRect(hwnd, NULL, FALSE);
				}
				result = 0;
				wasHandled = true;
				break;

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