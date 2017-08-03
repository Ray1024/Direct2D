/***********************************************************************
功能：包含D2D1需要的函数的简单封装
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

#ifndef RAY1024_D2D1FUNC
#define RAY1024_D2D1FUNC

#include "D2DUtil.h"

// 从文件读取位图
HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget* IN pRenderTarget,
	IWICImagingFactory* IN pIWICFactory,
	PCWSTR IN uri,
	UINT OPTIONAL width,
	UINT OPTIONAL height,
	ID2D1Bitmap** OUT ppBitmap);

#endif