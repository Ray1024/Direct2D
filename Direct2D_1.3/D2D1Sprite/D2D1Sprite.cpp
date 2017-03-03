/***********************************************************************
程序：Direct2D 1.3 批精灵示例
功能：实例化D2D1App子类，利用现成的程序框架，并在子类中实现自己的功能
作者：Ray1024
网址：http://www.cnblogs.com/Ray1024/
***********************************************************************/

// 如果使用Direct2D1.3的API，需要定义下面这个宏
#define USE_D2D1_3

#include "../Common/D2D1App.h"
#include "../Common/D2D1Func.h"

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
	// 重写：鼠标弹起消息处理
	virtual void OnMouseUp(WPARAM btnState, int x, int y);

private:
	// D2D位图
	ID2D1Bitmap1*						m_pMapAsset = nullptr;
	// D2D精灵集
	ID2D1SpriteBatch*                   m_pSpriteBatch = nullptr;
	// 渲染状态：按下鼠标按键切换渲染方式 批精灵/位图
	bool								m_bState = true;
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
// 一些全局变量和函数
//-------------------------------------------------------------------
struct MapBase {
	uint32_t    width;
	uint32_t    height;
	uint32_t    unit_width;
	uint32_t    unit_height;
};

MapBase*    g_mapData = nullptr;
HRESULT CreateSprites(const MapBase* map, ID2D1SpriteBatch* sprites)
{
	assert(map && sprites && "bad arguments");
	sprites->Clear();
	HRESULT hr = S_OK;
	// 添加精灵
	const auto count = map->width * map->height;
	{
		D2D1_RECT_F rect = { 0.f };
		hr = sprites->AddSprites(count, &rect, nullptr, nullptr, nullptr, 0);
	}
	// 成功
	if (SUCCEEDED(hr))
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			// 计算源矩形
			D2D1_RECT_U src_rect = D2D1::RectU(0, 0, 200, 200);
			// 计算目标矩形
			uint32_t x = i % map->width;
			uint32_t y = i / map->width;
			D2D1_RECT_F des_rect;
			des_rect.left = x * map->unit_width;
			des_rect.top = y * map->unit_height;
			des_rect.right = des_rect.left + static_cast<float>(map->unit_width);
			des_rect.bottom = des_rect.top + static_cast<float>(map->unit_height);
			// 设置
			hr = sprites->SetSprites(i, 1, &des_rect, &src_rect);
			assert(SUCCEEDED(hr));
		}
	}
	return hr;
}


//-------------------------------------------------------------------
// D2D1Template Implement
//-------------------------------------------------------------------

D2D1Template::D2D1Template()
{
	m_wndCaption = "Direct2D 1.3 批精灵示例(通过鼠标左键切换渲染方式)";
}

D2D1Template::~D2D1Template()
{
	SafeRelease(m_pMapAsset);
	SafeRelease(m_pSpriteBatch);
}

HRESULT D2D1Template::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	if (!SUCCEEDED(D2D1App::Initialize(hInstance, nCmdShow)))
	{
		return S_FALSE;
	}

	HRESULT hr = S_OK;

	static const MapBase s_map = { 200, 200, 32, 32 };
	g_mapData = reinterpret_cast<MapBase*>(std::malloc(sizeof(MapBase)));
	if (g_mapData)
	{
		std::memcpy(g_mapData, &s_map, sizeof(s_map));
	}

	// 创建精灵集
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DDeviceContext->CreateSpriteBatch(&m_pSpriteBatch);
	}
	// 创建精灵
	if (SUCCEEDED(hr))
	{
		hr = CreateSprites(g_mapData, m_pSpriteBatch);
	}
	// 载入地图资源集
	if (SUCCEEDED(hr))
	{
		hr = LoadBitmapFromFile(
			m_pD2DDeviceContext, m_pWICFactory, L"leopard.png", 0, 0, &m_pMapAsset);
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

		D2D1_MATRIX_3X2_F matrix;
		m_pD2DDeviceContext->GetTransform(&matrix);
// 		const float zoom = 0.5;
// 		m_pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(zoom, zoom)));
		m_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

		// 渲染方式：批精灵
		if (m_bState)
		{
			m_pD2DDeviceContext->DrawSpriteBatch(
				m_pSpriteBatch, m_pMapAsset, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		}
		// 渲染方式：位图
		else
		{
			for (uint32_t i = 0; i < g_mapData->width * g_mapData->height; ++i)
			{
				// 计算源矩形
				D2D1_RECT_F src_rect = D2D1::RectF(0, 0, 200, 200);
				// 计算目标矩形
				uint32_t x = i % g_mapData->width;
				uint32_t y = i / g_mapData->width;
				D2D1_RECT_F des_rect;
				des_rect.left = x * g_mapData->unit_width;
				des_rect.top = y * g_mapData->unit_height;
				des_rect.right = des_rect.left + g_mapData->unit_width;
				des_rect.bottom = des_rect.top + g_mapData->unit_height;
				// 设置
				m_pD2DDeviceContext->DrawBitmap(
					m_pMapAsset,
					&des_rect, 1.f,
					D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
					&src_rect);
			}
		}

		m_pD2DDeviceContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		m_pD2DDeviceContext->SetTransform(&matrix);

		// 结束渲染
		m_pD2DDeviceContext->EndDraw();
		// 呈现目标
		m_pSwapChain->Present(0, 0);
	}
}

void D2D1Template::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_bState = !m_bState;
	if (m_bState)
	{
		m_wndCaption = "Direct2D 1.3 批精灵示例 | 渲染方式：批精灵";
	}
	else
	{
		m_wndCaption = "Direct2D 1.3 批精灵示例 | 渲染方式：位图";
	}
}