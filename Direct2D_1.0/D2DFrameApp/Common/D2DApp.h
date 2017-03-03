/**********************************************************************
 @FILE		D2DApp.h
 @BRIEF		Direct2D应用程序类，使用时可以继承这个类
 @AUTHOR	Ray1024
 @DATE		2016.11.29
 *********************************************************************/

#ifndef D2DAPP_H
#define D2DAPP_H

#include "D2DUtil.h"
#include "D2DTimer.h"

//------------------------------------------------------------------------------
// D2DApp
//------------------------------------------------------------------------------
class D2DApp
{
public:
    D2DApp(HINSTANCE hInstance);
    virtual ~D2DApp();

	HINSTANCE AppInst()const;
	HWND MainWnd()const;
	void Run();

	// 框架方法（可以重写来实现自己的渲染效果）
	virtual bool Init();
	virtual void OnResize();
	virtual void UpdateScene(float dt)=0;
	virtual void DrawScene()=0; 
	virtual LRESULT WndProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

	// 处理鼠标输入事件的便捷重载函数
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	HRESULT InitWindow();
	HRESULT InitDirect2D();

	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();

	void OnDestroy();

    BOOL IsRunning() { return m_fRunning; }

	void CalculateFrameStats();

protected:
	HINSTANCE	m_hAppInst;		// 应用程序实例句柄
    HWND		m_hWnd;			// 窗口句柄
	std::string	m_wndCaption;	// 窗口标题
	int			m_clientWidth;	// 宽度
	int			m_clientHeight;	// 高度
	BOOL		m_fRunning;		// 是否运行
	D2DTimer	m_timer;		// 用于记录deltatime和游戏时间

    ID2D1Factory*			m_pD2DFactory;		// D2D工厂
	IWICImagingFactory*		m_pWICFactory;		// WIC工厂
	IDWriteFactory*			m_pDWriteFactory;	// DWrite工厂
    ID2D1HwndRenderTarget*	m_pRT;				// 呈现器
};

#endif // D2DAPP_H