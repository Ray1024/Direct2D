
// MFCD2DApplicationView.h : CMFCD2DApplicationView 类的接口
//

#pragma once


class CMFCD2DApplicationView : public CView
{
protected: // 仅从序列化创建
	CMFCD2DApplicationView();
	DECLARE_DYNCREATE(CMFCD2DApplicationView)

// 特性
public:
	CMFCD2DApplicationDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CMFCD2DApplicationView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 成员
	CD2DTextFormat* m_pTextFormat;
	CD2DSolidColorBrush* m_pBlackBrush;
	CD2DLinearGradientBrush* m_pLinearGradientBrush;

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	afx_msg LRESULT OnAfxWmDraw2d(WPARAM wParam, LPARAM lParam);
};

#ifndef _DEBUG  // MFCD2DApplicationView.cpp 中的调试版本
inline CMFCD2DApplicationDoc* CMFCD2DApplicationView::GetDocument() const
   { return reinterpret_cast<CMFCD2DApplicationDoc*>(m_pDocument); }
#endif

