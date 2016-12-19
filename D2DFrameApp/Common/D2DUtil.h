/**********************************************************************
 @FILE		D2DUtil.h
 @BRIEF		The file contains some useful tool codes.
 @AUTHOR	Ray1024
 @DATE		2016.11.22
 *********************************************************************/

#ifndef D2DUTIL_H
#define D2DUTIL_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
 
#include <Windows.h>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

// Direct2D需要的头文件
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dwmapi.h>

#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "DWrite.lib")

/******************************************************************
需要用到的宏定义
******************************************************************/

template<class Interface>
inline void
	SafeRelease(
	Interface **ppInterfaceToRelease
	)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#endif // D2DUTIL_H