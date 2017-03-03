 /***********************************************************************
 功能：Direct2D 1.1应用程序类，使用时可以继承这个类
 作者：Ray1024
 网址：http://www.cnblogs.com/Ray1024/
 ***********************************************************************/


#ifndef RAY1024_D2DAPP
#define RAY1024_D2DAPP

#include "Common.h"
#include "D2D1Timer.h"


// D2D1App
class D2D1App
{
public:
	// 构造函数
    D2D1App();
	// 析构函数
    virtual ~D2D1App();

	// 初始化
	virtual HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
	// 可以重写此函数来实现你想做的每帧执行的操作
	virtual void UpdateScene(float dt) {};
	// 渲染
	virtual void DrawScene() = 0; 

	// 消息循环
	void Run();
	// 处理消息
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:

	// 创建设备无关资源
	HRESULT CreateDeviceIndependentResources();
	// 创建设备有关资源
	HRESULT CreateDeviceResources();
	// 丢弃设备有关资源
	void DiscardDeviceResources();
	// 计算帧数信息
	void CalculateFrameStats();

	// 消息处理：鼠标
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
	virtual void OnMouseWheel(UINT nFlags, short zDelta, int x, int y) { }
	// 消息处理：窗口销毁
	virtual void OnDestroy();

protected:
	// 应用程序实例句柄
	HINSTANCE							m_hAppInst = nullptr;
	// 窗口句柄
	HWND								m_hWnd = nullptr;
	// 窗口标题
	std::string							m_wndCaption = "D2D1App";
	// 是否运行
	BOOL								m_fRunning = TRUE;
	// 用于记录deltatime和游戏时间
	D2D1Timer							m_timer;

#ifdef USE_D2D1_3
	// D2D 工厂
	ID2D1Factory4*                      m_pD2DFactory = nullptr;
	// D2D 设备
	ID2D1Device3*                       m_pD2DDevice = nullptr;
	// D2D 设备上下文
	ID2D1DeviceContext3*                m_pD2DDeviceContext = nullptr;
#else
	// D2D 工厂
	ID2D1Factory3*                      m_pD2DFactory = nullptr;
	// D2D 设备
	ID2D1Device2*                       m_pD2DDevice = nullptr;
	// D2D 设备上下文
	ID2D1DeviceContext2*                m_pD2DDeviceContext = nullptr;
#endif // USE_D2D1_3

	// WIC 工厂
	IWICImagingFactory2*                m_pWICFactory = nullptr;
	// DWrite工厂
	IDWriteFactory1*                    m_pDWriteFactory = nullptr;
	// DXGI 交换链
	IDXGISwapChain1*                    m_pSwapChain = nullptr;
	// D2D 位图 储存当前显示的位图
	ID2D1Bitmap1*                       m_pD2DTargetBimtap = nullptr;
	//  所创设备特性等级
	D3D_FEATURE_LEVEL                   m_featureLevel;
	// 手动交换链
	DXGI_PRESENT_PARAMETERS             m_parameters;
};

#endif