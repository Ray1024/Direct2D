/***********************************************************************
程序：Direct2D1.1内置特效——离散传递效果
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

#include "../Common/D2D1App.h"
#include "../Common/D2D1Func.h"

//--------------------------------------------------------------------
// D2D1EffectGaussianBlur
//--------------------------------------------------------------------
class D2D1EffectGaussianBlur : public D2D1App
{
public:
	// 构造函数
	D2D1EffectGaussianBlur();
	// 析构函数
	~D2D1EffectGaussianBlur();
	// 重写：初始化
	virtual HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
	// 重写：渲染
	virtual void DrawScene();
	// 重写：鼠标滚轮消息处理
	virtual void OnMouseWheel(UINT nFlags, short zDelta, int x, int y);

private:
	// D2D位图
	ID2D1Bitmap1*						m_pBitmap = nullptr;
	// 测试特效
	ID2D1Effect*                        m_pEffectTest = nullptr;
	// 浮点数据
	float                               m_afData = 0.f;
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
			D2D1EffectGaussianBlur app;
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
// D2D1EffectGaussianBlur Implement
//-------------------------------------------------------------------

D2D1EffectGaussianBlur::D2D1EffectGaussianBlur()
{
	m_wndCaption = "D2D1内置特效之离散传递效果（鼠标滚轮调整效果）";
}

D2D1EffectGaussianBlur::~D2D1EffectGaussianBlur()
{
	SafeRelease(m_pBitmap);
	SafeRelease(m_pEffectTest);
}

HRESULT D2D1EffectGaussianBlur::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	if (!SUCCEEDED(D2D1App::Initialize(hInstance, nCmdShow)))
	{
		return S_FALSE;
	}

	HRESULT hr = S_OK;

	// 创建位图
	if (SUCCEEDED(hr))
	{
		hr = LoadBitmapFromFile(
			m_pD2DDeviceContext, m_pWICFactory, L"effect.png", 0, 0, &m_pBitmap);
	}

	// 创建特效
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DDeviceContext->CreateEffect(CLSID_D2D1DiscreteTransfer, &m_pEffectTest);
	}

	// 设置输入
	if (SUCCEEDED(hr))
	{
		m_pEffectTest->SetInput(0, m_pBitmap);
	}

	return true;
}

void D2D1EffectGaussianBlur::DrawScene()
{
	if (m_pD2DDeviceContext != nullptr)
	{
		// 开始渲染
		m_pD2DDeviceContext->BeginDraw();
		// 清屏
		m_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::LightBlue));

		// 设置模糊程度，并显示图片
		float table[3] = { 0, 0.5f, m_afData / 500.f };
		m_pEffectTest->SetValue(D2D1_DISCRETETRANSFER_PROP_RED_TABLE, table);
		m_pEffectTest->SetValue(D2D1_DISCRETETRANSFER_PROP_GREEN_TABLE, table);
		m_pEffectTest->SetValue(D2D1_DISCRETETRANSFER_PROP_BLUE_TABLE, table);

		m_pD2DDeviceContext->DrawImage(m_pEffectTest);

		// 结束渲染
		m_pD2DDeviceContext->EndDraw();
		// 呈现目标
		m_pSwapChain->Present(0, 0);
	}
}

void D2D1EffectGaussianBlur::OnMouseWheel(UINT nFlags, short zDelta, int x, int y)
{
	if (zDelta>0)
	{
		m_afData += 10;
	}
	else
	{
		m_afData -= 10;
		if (m_afData<0)
		{
			m_afData = 0;
		}
	}
}