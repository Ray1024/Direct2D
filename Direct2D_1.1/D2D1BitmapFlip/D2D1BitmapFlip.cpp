/***********************************************************************
程序：[Direct2D1.1教程] 打破二维的限制，实现图片的翻转
功能：实例化D2D1App子类，利用现成的程序框架，并在子类中实现自己的功能
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

#include "../Common/D2D1App.h"
#include "../Common/D2D1Func.h"

static float s_angle = 0.f;

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
	// 重写：此函数来实现你想做的每帧执行的操作
	virtual void UpdateScene(float dt);

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
	m_wndCaption = "[Direct2D1.1教程] 打破二维的限制，实现图片的翻转";
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
			m_pD2DDeviceContext, m_pWICFactory, L"flower.png", 0, 0, &m_pBitmap);
	}

	// 创建特效
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pEffectTest);
	}

	// 设置输入
	if (SUCCEEDED(hr))
	{
		m_pEffectTest->SetInput(0, m_pBitmap);
	}

	return true;
}

void D2D1EffectGaussianBlur::UpdateScene(float dt)
{
	s_angle += 0.1;
}

void D2D1EffectGaussianBlur::DrawScene()
{
	if (m_pD2DDeviceContext != nullptr)
	{
		// 开始渲染
		m_pD2DDeviceContext->BeginDraw();
		// 清屏
		m_pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Purple));

		float bitmapW = m_pBitmap->GetSize().width;
		float bitmapH = m_pBitmap->GetSize().height;

		m_pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(500, 300));

		// 4*4的矩阵
		D2D1::Matrix4x4F m;

		// 平移，决定旋转轴的位置
		m = m * D2D1::Matrix4x4F::Translation(-bitmapW / 2, -bitmapH / 2, 0);
		// 向量 vector {x, y, z}为翻转轴，可以指定为任意轴（注意：向量只是轴的方向，并没有指定轴的位置，向量+平移才能确定轴的方向和位置）
		m = m * D2D1::Matrix4x4F::RotationX(s_angle);
		m = m * D2D1::Matrix4x4F::RotationY(0);
		m = m * D2D1::Matrix4x4F::RotationZ(0);
		//m = m * D2D1::Matrix4x4F::RotationArbitraryAxis(0, 1, 0, s_angle);
		// 深度
		m = m * D2D1::Matrix4x4F::PerspectiveProjection(bitmapH*10);

		m_pD2DDeviceContext->DrawBitmap(
			m_pBitmap,
			&D2D1::RectF(0, 0, bitmapW, bitmapH),
			1,
			D2D1_INTERPOLATION_MODE_LINEAR,
			&D2D1::RectF(0, 0, bitmapW, bitmapH),
			&m);


		// 结束渲染
		m_pD2DDeviceContext->EndDraw();

		m_pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		// 呈现目标
		m_pSwapChain->Present(0, 0);
	}
}