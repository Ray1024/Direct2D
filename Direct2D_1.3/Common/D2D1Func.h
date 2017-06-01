/***********************************************************************
功能：包含D2D1需要的函数的简单封装
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

#pragma once

#include "Common.h"

// 从文件读取位图
HRESULT LoadBitmapFromFile(
	ID2D1DeviceContext* IN pRenderTarget, IWICImagingFactory2* IN pIWICFactory,
	PCWSTR IN uri, UINT OPTIONAL width, UINT OPTIONAL height, ID2D1Bitmap1** OUT ppBitmap);

// 创建路径几何图形
ID2D1PathGeometry*	GetPathGeometry(
	ID2D1Factory4* pD2DFactory, D2D1_POINT_2F *points, UINT pointsCount);
