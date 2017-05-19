/***********************************************************************
功能：包含程序使用的头文件和库文件
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

#ifndef RAY1024_COMMON
#define RAY1024_COMMON

#include <Windows.h>

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cwchar>
#include <cstddef>
#include <atomic>
#include <thread>
#include <new>
#include <string>
#include <sstream>

#include <dxgi1_4.h>
#include <D3D11.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dwrite_2.h>
#include <wincodec.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib" )
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib" )
#pragma comment(lib, "windowscodecs.lib" )

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm.lib")

// 计算数组大小
#define lengthof(a) (sizeof(a)/sizeof(*(a)))

// 释放资源
template<class Interface>
inline void SafeRelease(Interface *&pInterfaceToRelease) {
	if (pInterfaceToRelease != nullptr) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = nullptr;
	}
}

#endif