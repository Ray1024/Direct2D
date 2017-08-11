//-----------------------------------------------------------------
// 功能：将Direct2D位图保存为图片文件，更多详细解释请参考：http://www.cnblogs.com/Ray1024/
// 作者：Ray1024
// 网址：http://www.cnblogs.com/Ray1024/
//-----------------------------------------------------------------

#include "D2DSaveBitmapToFile.h"
#include "resource.h"

HRESULT SaveBitmapToFile(ID2D1Factory* pD2DFactory, IWICImagingFactory* pWICFactory, ID2D1Bitmap* bitmap, LPCWSTR filename)
{
	HRESULT hr = S_FALSE;
	IWICStream*	pStream = NULL;
	IWICBitmapEncoder* pEncoder = NULL;
	IWICBitmapFrameEncode* pFrameEncode = NULL;
	IWICBitmap*	pWicBitmap = NULL;
	ID2D1RenderTarget* pD2DRenderTarget = NULL;

	D2D1_SIZE_F sizeToSave = bitmap->GetSize();

	//////////////////////////////////////////////////////////////////////////
	// 在WIC位图上绘制内容

	if (SUCCEEDED(hr))
	{
		hr = pWICFactory->CreateBitmap(
			sizeToSave.width,
			sizeToSave.height,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad,
			&pWicBitmap);
	}

	if (SUCCEEDED(hr))
	{
		hr = pD2DFactory->CreateWicBitmapRenderTarget(
			pWicBitmap,
			&D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE),
			&pD2DRenderTarget);
	}

	if (SUCCEEDED(hr))
	{
		pD2DRenderTarget->BeginDraw();

		pD2DRenderTarget->DrawBitmap(
			bitmap,
			D2D1::RectF(0, 0, sizeToSave.width, sizeToSave.height));

		hr = pD2DRenderTarget->EndDraw();
	}

	//
	if (SUCCEEDED(hr))
	{
		hr = pWICFactory->CreateStream(&pStream);
	}

	if (SUCCEEDED(hr))
	{
		hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);
	}

	if (SUCCEEDED(hr))
	{
		hr = pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder);
	}

	if (SUCCEEDED(hr))
	{
		hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);
	}

	if (SUCCEEDED(hr))
	{
		hr = pEncoder->CreateNewFrame(&pFrameEncode, NULL);
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->Initialize(NULL);
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->SetSize(sizeToSave.width, sizeToSave.height);
	}

	if (SUCCEEDED(hr))
	{
		WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;  
		hr = pFrameEncode->SetPixelFormat(&format);
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->WriteSource(pWicBitmap, NULL);
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameEncode->Commit();
	}

	if (SUCCEEDED(hr))
	{
		hr = pEncoder->Commit();
	}

	return hr;
}

/******************************************************************
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
	m_pWICFactory(NULL),
	m_pCRT(NULL),
	m_pBrush(NULL)
{
}

DemoApp::~DemoApp()
{
	SafeRelease(&m_pBrush);
	SafeRelease(&m_pCRT);
	SafeRelease(&m_pRT);
	SafeRelease(&m_pWICFactory);
    SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
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
	wcex.hIcon		   = NULL;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
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

        m_hwnd = CreateWindowEx(
			WS_EX_ACCEPTFILES, 
            L"D2DDemoApp",
            L"RIMGPacker",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(480.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(320.f * dpiY / 96.f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			hr = CreateDeviceResources();
		}

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

	if (m_pWICFactory == NULL && SUCCEEDED(hr))
	{
		if (!SUCCEEDED(
			CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_pWICFactory)
			)
			))
			return FALSE;
	}

    return hr;
}

HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

	ID2D1Bitmap* pCopyBitmap = NULL;

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
            D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );

		// 创建CompatibleRT
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateCompatibleRenderTarget(
				D2D1::SizeF(size.width, size.height),
				&m_pCRT);
		}

		// 创建画布位图
		if (SUCCEEDED(hr))
		{
			m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Purple), &m_pBrush);
		}
    }

    return hr;
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

void DemoApp::OnRender()
{
    if (m_pRT != NULL)
    {
		// 在CompatibleRT上绘制

		m_pCRT->BeginDraw();
		m_pCRT->Clear(D2D1::ColorF(D2D1::ColorF::LimeGreen));

		// 绘制
		m_pCRT->DrawRectangle(
			&D2D1::RectF(200,0,250,300),
			m_pBrush);

		D2D1_ROUNDED_RECT roundRect = D2D1::RoundedRect(D2D1::RectF(100,50,250,100), 5, 5);

		m_pCRT->DrawRoundedRectangle(
			&roundRect,
			m_pBrush);

		m_pCRT->EndDraw();

		// 从CompatibleRT上取出内容
		ID2D1Bitmap* bitmap = NULL;
		m_pCRT->GetBitmap(&bitmap);

        // 开始绘制
        m_pRT->BeginDraw();
        m_pRT->Clear();

		// 绘制
        m_pRT->DrawBitmap(
			bitmap,
			D2D1::RectF(0,0,
			bitmap->GetSize().width,
			bitmap->GetSize().height));
		
        // 结束绘制
        m_pRT->EndDraw();

		SafeRelease(&bitmap);
    }
}

void DemoApp::OnCommand(UINT id)
{
	switch(id)
	{
	case IDM_OUT:
		{
			ID2D1Bitmap* pBitmapToSave = NULL;

			WCHAR path[MAX_PATH] = {0};
			WCHAR filename[MAX_PATH] = {0};

			// 获取可执行文件路径
			::GetModuleFileName(0, path, MAX_PATH);
			::PathRemoveFileSpec(path);

			// 将字体文件转换成字体文件路径
			wsprintf(filename, L"%s\\%s", path, L"output.png");

			//////////////////////////////////////////////////////////////////////////
			// 获取要保存的内容

			HRESULT hr = m_pCRT->GetBitmap(&pBitmapToSave);

			hr = SaveBitmapToFile(m_pD2DFactory, m_pWICFactory, pBitmapToSave, filename);

			SafeRelease(&pBitmapToSave);
		}
		break;
	}
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
        HRESULT hr = m_pRT->Resize(size);
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
			case WM_COMMAND:
				{
					UINT id = LOWORD(wParam);
					pDemoApp->OnCommand(id);
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