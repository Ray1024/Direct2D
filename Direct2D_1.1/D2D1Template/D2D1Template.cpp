/***********************************************************************
程序：Direct2D 1.1 程序模板，演示如何使用本程序框架
功能：实例化D2D1App子类，利用现成的程序框架，并在子类中实现自己的功能
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

#include "../Common/D2D1App.h"

//--------------------------------------------------------------------
// D2D1Template
//--------------------------------------------------------------------
class D2D1Template : public D2D1App
{
public:
	// 构造函数
	D2D1Template();
	// 析构函数
	~D2D1Template();
	// 重写：初始化
	virtual HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
	// 重写：渲染
	virtual void DrawScene();

private:
	// 单色画刷
	ID2D1SolidColorBrush*	m_pBrush = nullptr;
	// 字体格式
	IDWriteTextFormat*		m_pTextFormat = nullptr;
};


//-------------------------------------------------------------------
// 程序入口WinMain
//-------------------------------------------------------------------
int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, 
	int nCmdShow) 
{
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    if (SUCCEEDED(::CoInitialize(nullptr)))
    {
        {
			D2D1Template app;
            if (SUCCEEDED(app.Initialize(hInstance, nCmdShow)))
            {
                app.Run();
            }
        }
        ::CoUninitialize();
    }

    return 0;
}


//-------------------------------------------------------------------
// D2D1Template Implement
//-------------------------------------------------------------------

D2D1Template::D2D1Template()
{
	m_wndCaption = "Direct2D 1.1 程序模板";
}

D2D1Template::~D2D1Template()
{
	SafeRelease(m_pBrush);
	SafeRelease(m_pTextFormat);
}

HRESULT D2D1Template::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	if (!SUCCEEDED(D2D1App::Initialize(hInstance, nCmdShow)))
	{
		return S_FALSE;
	}

	HRESULT hr = S_OK;

	// 创建字体格式
	if (m_pDWriteFactory != NULL)
	{
		hr = m_pDWriteFactory->CreateTextFormat(
			L"Sitka Text",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			68.0,
			L"en-us",
			&m_pTextFormat);
	}

	// 创建画刷
	if (m_pD2DDeviceContext != NULL && SUCCEEDED(hr))
	{
		hr = m_pD2DDeviceContext->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::DarkBlue), &m_pBrush);
	}

	return true;
}

void D2D1Template::DrawScene()
{
	if (m_pD2DDeviceContext != nullptr)
	{
		// 开始渲染
		m_pD2DDeviceContext->BeginDraw();
		// 清屏
		m_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightBlue));

		m_pD2DDeviceContext->DrawText(
			L"Template project of Direct2D 1.1.",
			wcslen(L"Template project of Direct2D 1.1."),
			m_pTextFormat,
			D2D1::RectF(0, 0, 1080, 0),
			m_pBrush);

		// 结束渲染
		m_pD2DDeviceContext->EndDraw();
		// 呈现目标
		m_pSwapChain->Present(0, 0);
	}
}