/**********************************************************************
 @FILE		D2DFrameApp.h
 @BRIEF		此工程演示了Direct2D程序框架的使用方法
 @AUTHOR	Ray1024
 @DATE		2016.11.29
 *********************************************************************/

#include "../Common/D2DApp.h"

//------------------------------------------------------------------------------
// D2DFrameApp
//------------------------------------------------------------------------------
class D2DFrameApp : public D2DApp
{
public:
	D2DFrameApp(HINSTANCE hInstance);
	~D2DFrameApp();

	bool Init();
	void UpdateScene(float dt);
	void DrawScene();

private:

	ID2D1SolidColorBrush*	m_pBrush;
	IDWriteTextFormat*		m_pTextFormat;
};


//------------------------------------------------------------------------------
// 程序入口WinMain
//------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			D2DFrameApp app(hInstance);

			if (app.Init())
			{
				app.Run();
			}
		}
		CoUninitialize();
	}

	return 0;
}

//------------------------------------------------------------------------------
// D2DFrameApp Implement
//------------------------------------------------------------------------------

D2DFrameApp::D2DFrameApp(HINSTANCE hInstance)
	: D2DApp(hInstance)
	, m_pBrush(NULL)
	, m_pTextFormat(NULL)
{
	m_wndCaption = "D2DFrameApp";
}

D2DFrameApp::~D2DFrameApp()
{
	SafeRelease(&m_pBrush);
	SafeRelease(&m_pTextFormat);
}

bool D2DFrameApp::Init()
{
	if (!D2DApp::Init())
	{
		return false;
	}

	HRESULT hr = S_OK;

	// 创建字体格式
	if (m_pDWriteFactory != NULL)
	{
		hr = m_pDWriteFactory->CreateTextFormat( 
			L"Edwardian Script ITC", 
			NULL, 
			DWRITE_FONT_WEIGHT_NORMAL, 
			DWRITE_FONT_STYLE_NORMAL, 
			DWRITE_FONT_STRETCH_NORMAL, 
			68.0, 
			L"en-us",
			&m_pTextFormat);
	}

	// 创建画刷
	if (m_pRT != NULL && SUCCEEDED(hr))
	{
		hr = m_pRT->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::DarkBlue),
			&m_pBrush
			);
	}

	return true;
}

void D2DFrameApp::UpdateScene(float dt)
{
}

void D2DFrameApp::DrawScene()
{
	HRESULT hr;

	if (!(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
	{
		m_pRT->BeginDraw();

		m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::LightBlue));

		m_pRT->DrawText(
			L"Direct2D Frame Application",
			27,
			m_pTextFormat,
			D2D1::RectF(0,160, 640, 300),
			m_pBrush);

		hr = m_pRT->EndDraw();
	}
}