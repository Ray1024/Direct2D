#include "D2DFPS.h"


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
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

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

DemoApp::DemoApp() :
    m_hwnd(NULL),
    m_pD2DFactory(NULL),
	m_pWICFactory(NULL),
    m_pRT(NULL),
	m_bitmap(NULL)
{
}

DemoApp::~DemoApp()
{
	SafeRelease(&m_bitmap);
	SafeRelease(&m_pWICFactory);
    SafeRelease(&m_pD2DFactory);
    SafeRelease(&m_pRT);
}

HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    m_fRunning = FALSE;

    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
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

        // Create the application window.
        //
        // Because the CreateWindow function takes its size in pixels, we
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX;
        FLOAT dpiY;
        m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"D2D Demo App",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.0f * dpiX / 96.0f)),
            static_cast<UINT>(ceil(480.0f * dpiY / 96.0f)),
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            m_fRunning = TRUE;

            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    static const WCHAR msc_fontName[] = L"Gabriola";
    static const FLOAT msc_fontSize = 50;
    static const WCHAR sc_helloWorld[] = L"The quick brown fox jumped over the lazy dog!";
    static const UINT stringLength = ARRAYSIZE(sc_helloWorld) - 1;
    HRESULT hr;

    //create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

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

    if (!m_pRT)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
            &m_pRT
            );

		m_bitmap = LoadBitmapFromFile(L"bitmap.png");
    }

    return hr;
}


ID2D1Bitmap* DemoApp::LoadBitmapFromFile(LPCWSTR uri, UINT width, UINT height)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;
	ID2D1Bitmap*	bitmap = NULL;

	// ¼ÓÔØÎ»Í¼-------------------------------------------------
	HRESULT hr = m_pWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
		);

	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		if (width != 0 || height != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (width == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(height) / static_cast<FLOAT>(originalHeight);
					width = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (height == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(width) / static_cast<FLOAT>(originalWidth);
					height = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = m_pWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						width,
						height,
						WICBitmapInterpolationModeCubic
						);
				}
				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
						);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
				);
		}
	}
	if (SUCCEEDED(hr))
	{

		// Create a Direct2D bitmap from the WIC bitmap.
		hr = m_pRT->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&bitmap
			);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return bitmap;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
}

void DemoApp::RunMessageLoop()
{
    while (this->IsRunning())
    {
		OnRender();
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void DemoApp::UpdateWindowText()
{
    static LONGLONG sc_lastTimeStatusShown = 0;

    if (   m_times.GetCount() > 0
        && m_times.GetLast() > sc_lastTimeStatusShown + 1000000
       )
    {
        sc_lastTimeStatusShown = m_times.GetLast();

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        float fps = 0.0f;
        if (m_times.GetCount() > 0)
        {
            fps = (m_times.GetCount()-1) * frequency.QuadPart / static_cast<float>((m_times.GetLast() - m_times.GetFirst()));
        }

        wchar_t title[255];
        StringCchPrintf(
            title,
            ARRAYSIZE(title),
			L"fps:%f",
            fps
            );

        SetWindowText(m_hwnd, title);
		printf("FPS : %f\n", fps);
    }
}

HRESULT DemoApp::OnRender()
{
    HRESULT hr;
    LARGE_INTEGER time;

    QueryPerformanceCounter(&time);
    m_times.Add(time.QuadPart);

    hr = CreateDeviceResources();
	
    if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        m_pRT->BeginDraw();

        m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::Red));

		m_pRT->DrawBitmap(m_bitmap);

        hr = m_pRT->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }
    }


    UpdateWindowText();

	//InvalidateRect(m_hwnd, NULL, true);

    return hr;
}

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRT)
    {
        D2D1_SIZE_U size;
        size.width = width;
        size.height = height;

        m_pRT->Resize(size);
    }
}

void DemoApp::OnDestroy()
{
    m_fRunning = FALSE;
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

                    //pDemoApp->OnRender();
                    EndPaint(hwnd, &ps);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    pDemoApp->OnDestroy();
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
