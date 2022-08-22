#pragma once
#include <afxwin.h>
class CColorListBox :public CListBox
{
	DECLARE_DYNAMIC(CColorListBox)
public:
	CColorListBox();
	~CColorListBox();

	//�ؼ� ����  ���� ��Ե ��ɫ �Լ����±�Ե����
	void SetSkin(COLORREF normal, COLORREF down, COLORREF edge = 0, int edgespace = 0);
	void SetItemHeight(int cy);//���õ�����Ŀ�߶�
private:
	COLORREF m_NormalColor;
	COLORREF m_DownColor;
	COLORREF m_EdgeColor;
	int m_EdgeSpace;
	int m_ItemHeigh;
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};


