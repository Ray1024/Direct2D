#include "D2D1Func.h"

// 从文件读取位图
HRESULT LoadBitmapFromFile(
	ID2D1DeviceContext* IN pRenderTarget,
	IWICImagingFactory2* IN pIWICFactory,
	PCWSTR IN uri,
	UINT OPTIONAL width,
	UINT OPTIONAL height,
	ID2D1Bitmap1** OUT ppBitmap)
{
	IWICBitmapDecoder *pDecoder = nullptr;
	IWICBitmapFrameDecode *pSource = nullptr;
	IWICStream *pStream = nullptr;
	IWICFormatConverter *pConverter = nullptr;
	IWICBitmapScaler *pScaler = nullptr;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder);

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
						WICBitmapInterpolationModeCubic);
				}
				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						nullptr,
						0.f,
						WICBitmapPaletteTypeMedianCut);
				}
			}
		}
		else
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				nullptr,
				0.f,
				WICBitmapPaletteTypeMedianCut);
		}
	}
	if (SUCCEEDED(hr))
	{
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			nullptr,
			ppBitmap);
	}

	SafeRelease(pDecoder);
	SafeRelease(pSource);
	SafeRelease(pStream);
	SafeRelease(pConverter);
	SafeRelease(pScaler);

	return hr;
}

// 创建路径几何图形
ID2D1PathGeometry*	GetPathGeometry(ID2D1Factory4* pD2DFactory, D2D1_POINT_2F *points, UINT pointsCount)
{
	ID2D1PathGeometry* geometry = NULL;
	HRESULT hr = pD2DFactory->CreatePathGeometry(&geometry);

	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = geometry->Open(&pSink); // 获取Sink对象

		if (SUCCEEDED(hr))
		{
			pSink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_FILLED);

			pSink->AddLines(points, pointsCount);

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}

		pSink->Close(); // 关闭Sink对象

		return geometry;
	}

	return NULL;
}