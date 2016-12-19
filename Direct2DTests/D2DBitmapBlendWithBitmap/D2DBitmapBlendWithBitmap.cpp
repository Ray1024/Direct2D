//-----------------------------------------------------------------
// 功能：把两个图片混合再绘制，更多详细解释请参考：http://www.cnblogs.com/Ray1024/
// 作者：Ray1024
// 网址：http://www.cnblogs.com/Ray1024/
//-----------------------------------------------------------------

#include "D2DBitmapBlendWithBitmap.h"

HRESULT LoadBitmapFromFile(ID2D1RenderTarget *pRenderTarget,IWICImagingFactory *pIWICFactory,
	LPCWSTR uri,UINT width,UINT height,ID2D1Bitmap **ppBitmap);
IWICBitmap* GetWICBitmapFromFile(IWICImagingFactory *pIWICFactory,LPCWSTR uri,BYTE** srcData,
	UINT* srcSize);
ID2D1Bitmap* GetD2DBitmapFromWICBitmap(IWICImagingFactory *pIWICFactory,ID2D1RenderTarget *pRenderTarget,
	IWICBitmap* source,float d2dWidth,float d2dHeight,BYTE* srcData,UINT srcSize);
ID2D1Bitmap* GetBitmapFromFile(IWICImagingFactory *pIWICFactory,ID2D1RenderTarget *pRenderTarget,
	LPCWSTR uri,UINT width,UINT height);
ID2D1Bitmap* GetBlendedBitmapWithColor(IWICImagingFactory *pIWICFactory,ID2D1RenderTarget *pRenderTarget,
	IWICBitmap* source,const D2D1_COLOR_F &color,float d2dWidth,float d2dHeight,BYTE* srcData,UINT srcSize);
HRESULT BlendWithColor(IWICBitmap* source, const D2D1_COLOR_F &color, BYTE* srcData, UINT srcSize);

//-----------------------------------------------------------------
// 从资源文件加载D2D位图
//-----------------------------------------------------------------
HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	LPCWSTR uri,
	UINT width,
	UINT height,
	ID2D1Bitmap **ppBitmap)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	// 加载位图-------------------------------------------------
	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
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
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
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

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
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
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
			);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}

IWICBitmap* GetWICBitmapFromFile(
	IWICImagingFactory *pIWICFactory,
	LPCWSTR uri,
	BYTE** srcData,
	UINT* srcSize)
{
	IWICBitmapDecoder*		pDecoder	= NULL;
	IWICBitmapFrameDecode*	pSource		= NULL;
	IWICBitmap*				pWIC		= NULL;
	UINT					originalWidth = 0;
	UINT					originalHeight = 0;

	// 加载位图-------------------------------------------------
	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
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
		hr = pSource->GetSize(&originalWidth,&originalHeight);
	}

	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateBitmapFromSourceRect(
			pSource, 0,0,(UINT)originalWidth,(UINT)originalHeight, &pWIC);
	}

	if (SUCCEEDED(hr))
	{
		UINT uiWidth = 0;
		UINT uiHeight = 0;
		pWIC->GetSize(&uiWidth, &uiHeight);

		IWICBitmapLock *pILock = NULL;
		WICRect rcLock = { 0, 0, uiWidth, uiHeight };
		hr = pWIC->Lock(&rcLock, WICBitmapLockWrite, &pILock);

		if (SUCCEEDED(hr))
		{
			UINT cbBufferSize = 0;
			UINT cbStride = 0;
			BYTE *pv = NULL;

			// Retrieve the stride.
			hr = pILock->GetStride(&cbStride);
			WICPixelFormatGUID format;
			pILock->GetPixelFormat(&format);
			if (SUCCEEDED(hr))
			{
				hr = pILock->GetDataPointer(&cbBufferSize, &pv);
			}

			// 伴随IWICBitmap对象返回，保存图像像素数据，
			// 因为我们将重复利用IWICBitmap对象，所以要保存图片原始像素数据
			if (SUCCEEDED(hr))
			{
				*srcData = new BYTE[cbBufferSize];
				memcpy(*srcData, pv, cbBufferSize);
				*srcSize = cbBufferSize;
			}

			// Release the bitmap lock.
			pILock->Release();
		}
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);

	return pWIC;
}

ID2D1Bitmap* GetD2DBitmapFromWICBitmap(
	IWICImagingFactory *pIWICFactory,
	ID2D1RenderTarget *pRenderTarget,
	IWICBitmap* source, 
	float d2dWidth, 
	float d2dHeight,
	BYTE* srcData,
	UINT srcSize)
{
	IWICFormatConverter*	pConverter	= NULL;
	IWICBitmapScaler*		pScaler		= NULL;
	ID2D1Bitmap*			bitmap		= NULL;
	HRESULT					hr			= S_FALSE;

	Assert(source);	// source can't be null

	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateBitmapScaler(&pScaler);
	}
	if (SUCCEEDED(hr))
	{
		hr = pScaler->Initialize(source, (UINT)d2dWidth, (UINT)d2dHeight, WICBitmapInterpolationModeCubic);
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

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&bitmap
			);
	}

	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	if (SUCCEEDED(hr))
	{
		return bitmap;
	}

	return NULL;
}

ID2D1Bitmap* GetBitmapFromFile(
	IWICImagingFactory *pIWICFactory, 
	ID2D1RenderTarget *pRenderTarget,
	LPCWSTR uri, 
	UINT width, 
	UINT height)
{
	BYTE*			srcData = NULL;
	UINT			srcSize = 0;
	IWICBitmap*		wicBitmap = NULL;
	ID2D1Bitmap*	bitmap = NULL;

	wicBitmap = GetWICBitmapFromFile(pIWICFactory, uri, &srcData, &srcSize);
	delete[] srcData;
	if (wicBitmap)
	{
		bitmap = GetD2DBitmapFromWICBitmap(pIWICFactory, pRenderTarget, wicBitmap, (float)width, (float)height, NULL, srcSize);
	}

	return bitmap;
}

ID2D1Bitmap* GetBlendedBitmapWithColor(
	IWICImagingFactory *pIWICFactory, 
	ID2D1RenderTarget *pRenderTarget,
	IWICBitmap* source,
	const D2D1_COLOR_F &color,
	float d2dWidth,
	float d2dHeight,
	BYTE* srcData,
	UINT srcSize)
{
	ID2D1Bitmap*	bitmap = NULL;

	HRESULT hr = BlendWithColor(source, color, srcData, srcSize);
	if (SUCCEEDED(hr))
	{
		bitmap = GetD2DBitmapFromWICBitmap(pIWICFactory, pRenderTarget, source, d2dWidth, d2dHeight, srcData, srcSize);
	}

	return bitmap;
}

HRESULT BlendWithColor(
	IWICBitmap* source, 
	const D2D1_COLOR_F &color,
	BYTE* srcData,
	UINT srcSize)
{
	Assert(source);	// source can't be null

	UINT uiWidth = 0;
	UINT uiHeight = 0;
	source->GetSize(&uiWidth, &uiHeight);

	IWICBitmapLock *pILock = NULL;
	WICRect rcLock = { 0, 0, uiWidth, uiHeight };
	HRESULT hr = source->Lock(&rcLock, WICBitmapLockWrite, &pILock);

	if (SUCCEEDED(hr))
	{
		UINT cbBufferSize = 0;
		UINT cbStride = 0;
		BYTE *pv = NULL;

		// Retrieve the stride.
		hr = pILock->GetStride(&cbStride);

		if (SUCCEEDED(hr))
		{
			hr = pILock->GetDataPointer(&cbBufferSize, &pv);
		}

		// reset the texture
		if (cbBufferSize==srcSize)
		{
			memcpy(pv, srcData, srcSize);
		}

		for (unsigned int i=0; i<cbBufferSize; i+=4)
		{
			if (pv[i+3] != 0)
			{
				pv[i]	*=color.b;
				pv[i+1]	*=color.g;
				pv[i+2]	*=color.r;
				pv[i+3] *=color.a;
			}
		}

		// Release the bitmap lock.
		pILock->Release();
	}

	return hr;
}

HRESULT BlendWithBitmap(
	IWICBitmap* source, 
	BYTE* srcData,
	UINT srcSize,
	IWICBitmap* source1, 
	BYTE* srcData1,
	UINT srcSize1,
	float proportion)
{
	Assert(source!=NULL && source1!=NULL);	// source can't be null

	HRESULT hr = S_OK;

	UINT uiWidth1 = 0;
	UINT uiHeight1 = 0;
	source1->GetSize(&uiWidth1, &uiHeight1);

	IWICBitmapLock *pILock1 = NULL;
	WICRect rcLock1 = { 0, 0, uiWidth1, uiHeight1 };
	hr = source1->Lock(&rcLock1, WICBitmapLockWrite, &pILock1);

	UINT cbBufferSize1 = 0;
	UINT cbStride1 = 0;
	BYTE *pv1 = NULL;

	if (SUCCEEDED(hr))
	{
		// Retrieve the stride.
		hr = pILock1->GetStride(&cbStride1);

		if (SUCCEEDED(hr))
		{
			hr = pILock1->GetDataPointer(&cbBufferSize1, &pv1);
		}

		// reset the texture
		if (cbBufferSize1==srcSize1)
		{
			memcpy(pv1, srcData1, srcSize1);
		}

// 		for (unsigned int i=0; i<cbBufferSize1; i+=4)
// 		{
// 			if (pv1[i+3] != 0)
// 			{
// 				pv1[i]	 *= pv1[i];
// 				pv1[i+1] *= pv1[i+1];
// 				pv1[i+2] *= pv1[i+2];
// 				pv1[i+3] *= pv1[i+3];
// 			}
// 		}

		pILock1->Release();
	}


	UINT uiWidth = 0;
	UINT uiHeight = 0;
	source->GetSize(&uiWidth, &uiHeight);

	IWICBitmapLock *pILock = NULL;
	WICRect rcLock = { 0, 0, uiWidth, uiHeight };
	hr = source->Lock(&rcLock, WICBitmapLockWrite, &pILock);

	if (SUCCEEDED(hr))
	{
		UINT cbBufferSize = 0;
		UINT cbStride = 0;
		BYTE *pv = NULL;

		// Retrieve the stride.
		hr = pILock->GetStride(&cbStride);

		if (SUCCEEDED(hr))
		{
			hr = pILock->GetDataPointer(&cbBufferSize, &pv);
		}

		// reset the texture
		if (cbBufferSize==srcSize)
		{
			memcpy(pv, srcData, srcSize);
		}

		for (unsigned int i=0; i<cbBufferSize; i+=4)
		{
			if (pv[i+3] != 0)
			{
				pv[i]	= pv[i]*(1-proportion)	 + pv1[i]*proportion;
				pv[i+1]	= pv[i+1]*(1-proportion) + pv1[i+1]*proportion;
				pv[i+2]	= pv[i+2]*(1-proportion) + pv1[i+2]*proportion;
				pv[i+3] = pv[i+3]*(1-proportion) + pv1[i+3]*proportion;				
			}
		}

		// Release the bitmap lock.
		pILock->Release();
	}

	return hr;
}

ID2D1Bitmap* GetBlendedBitmapWithBitmap(
	IWICImagingFactory *pIWICFactory, 
	ID2D1RenderTarget *pRenderTarget,
	IWICBitmap* source,
	BYTE* srcData,
	UINT srcSize,
	IWICBitmap* source1,
	BYTE* srcData1,
	UINT srcSize1,
	float d2dWidth,
	float d2dHeight)
{
	ID2D1Bitmap*	bitmap = NULL;

	HRESULT hr = BlendWithBitmap(source, srcData, srcSize, source1, srcData1, srcSize1, 0.7);
	if (SUCCEEDED(hr))
	{
		bitmap = GetD2DBitmapFromWICBitmap(pIWICFactory, pRenderTarget, source, d2dWidth, d2dHeight, srcData, srcSize);
	}

	return bitmap;
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
	m_pBitmap(NULL),
	m_pBitmap1(NULL),
	m_pBitmapBlended(NULL)
{
}

DemoApp::~DemoApp()
{
	SafeRelease(&m_pWICFactory);
    SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBitmap);
	SafeRelease(&m_pBitmap1);
	SafeRelease(&m_pBitmapBlended);
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
            L"D2DBitmapBlendWithBitmap",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(960.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(640.f * dpiY / 96.f)),
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

        // 创建render target
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRT
            );

		// 创建位图
		if (SUCCEEDED(hr))
		{
			LoadBitmapFromFile(m_pRT,m_pWICFactory, L"bitmap.png",0,0, &m_pBitmap);
			LoadBitmapFromFile(m_pRT,m_pWICFactory, L"bitmapBlend.png",0,0, &m_pBitmap1);
		}

		// 创建位图，并进行颜色混合
		if (SUCCEEDED(hr))
		{
			IWICBitmap*			pWIC = NULL;
			BYTE*				srcData = NULL;
			UINT				srcSize = 0;

			IWICBitmap*			pWIC1 = NULL;
			BYTE*				srcData1 = NULL;
			UINT				srcSize1 = 0;

			pWIC = GetWICBitmapFromFile(m_pWICFactory,L"bitmap.png", &srcData, &srcSize);
			pWIC1 = GetWICBitmapFromFile(m_pWICFactory,L"bitmapBlend.png", &srcData1, &srcSize1);

			UINT bitmapWidth, bitmapHeight;
			pWIC->GetSize(&bitmapWidth, &bitmapHeight);
			m_pBitmapBlended = GetBlendedBitmapWithBitmap(m_pWICFactory, m_pRT, 
				pWIC, srcData, srcSize, pWIC1, srcData1, srcSize1, bitmapWidth, bitmapHeight);
		}
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRT);
    SafeRelease(&m_pBitmap);
	SafeRelease(&m_pBitmap1);
	SafeRelease(&m_pBitmapBlended);
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

		// 绘制位图
        m_pRT->DrawBitmap(
			m_pBitmap,
			D2D1::RectF(0,0,
			m_pBitmap->GetSize().width,
			m_pBitmap->GetSize().height));

		m_pRT->DrawBitmap(
			m_pBitmap1,
			D2D1::RectF(
			0,
			0 + m_pBitmap->GetSize().height,
			m_pBitmap->GetSize().width,
			m_pBitmap->GetSize().height + m_pBitmap1->GetSize().height));

		// 绘制颜色混合后的位图
		m_pRT->DrawBitmap(
			m_pBitmapBlended,
			D2D1::RectF(
			0 + m_pBitmap->GetSize().width,
			0,
			m_pBitmap->GetSize().width + m_pBitmapBlended->GetSize().width,
			0 + m_pBitmapBlended->GetSize().height));
		
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