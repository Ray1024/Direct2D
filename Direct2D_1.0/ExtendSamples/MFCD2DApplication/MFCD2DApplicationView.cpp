
// MFCD2DApplicationView.cpp : CMFCD2DApplicationView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFCD2DApplication.h"
#endif

#include "MFCD2DApplicationDoc.h"
#include "MFCD2DApplicationView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCD2DApplicationView

IMPLEMENT_DYNCREATE(CMFCD2DApplicationView, CView)

BEGIN_MESSAGE_MAP(CMFCD2DApplicationView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCD2DApplicationView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, &CMFCD2DApplicationView::OnAfxWmDraw2d)
END_MESSAGE_MAP()

// CMFCD2DApplicationView 构造/析构

CMFCD2DApplicationView::CMFCD2DApplicationView()
{
	// TODO: 在此处添加构造代码
	// Enable D2D support for this window:  
	EnableD2DSupport();

	// Initialize D2D resources:  
	m_pBlackBrush = new CD2DSolidColorBrush(GetRenderTarget(), D2D1::ColorF(D2D1::ColorF::Black));

	m_pTextFormat = new CD2DTextFormat(GetRenderTarget(), _T("Verdana"), 50);
	m_pTextFormat->Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	m_pTextFormat->Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	D2D1_GRADIENT_STOP gradientStops[2];

	gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::White);
	gradientStops[0].position = 0.f;
	gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Indigo);
	gradientStops[1].position = 1.f;

	m_pLinearGradientBrush = new CD2DLinearGradientBrush(GetRenderTarget(),
		gradientStops, ARRAYSIZE(gradientStops),
		D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(0, 0)));
}

CMFCD2DApplicationView::~CMFCD2DApplicationView()
{
}

BOOL CMFCD2DApplicationView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFCD2DApplicationView 绘制

void CMFCD2DApplicationView::OnDraw(CDC* /*pDC*/)
{
	CMFCD2DApplicationDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CMFCD2DApplicationView 打印


void CMFCD2DApplicationView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCD2DApplicationView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFCD2DApplicationView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMFCD2DApplicationView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CMFCD2DApplicationView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCD2DApplicationView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCD2DApplicationView 诊断

#ifdef _DEBUG
void CMFCD2DApplicationView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCD2DApplicationView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCD2DApplicationDoc* CMFCD2DApplicationView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCD2DApplicationDoc)));
	return (CMFCD2DApplicationDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCD2DApplicationView 消息处理程序


void CMFCD2DApplicationView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	m_pLinearGradientBrush->SetEndPoint(CPoint(cx, cy));
}


afx_msg LRESULT CMFCD2DApplicationView::OnAfxWmDraw2d(WPARAM wParam, LPARAM lParam)
{
	CHwndRenderTarget* pRenderTarget = (CHwndRenderTarget*)lParam;
	ASSERT_VALID(pRenderTarget);

	CRect rect;
	GetClientRect(rect);

	pRenderTarget->FillRectangle(rect, m_pLinearGradientBrush);
	pRenderTarget->DrawText(_T("Hello, World!"), rect, m_pBlackBrush, m_pTextFormat);

	return TRUE;
}
