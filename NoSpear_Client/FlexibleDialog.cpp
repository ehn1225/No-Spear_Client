#include "pch.h"
#include "FlexibleDialog.h"

CFlexibleDialog::CFlexibleDialog(UINT nIDTemplate, CWnd* pParent)
	: CDialogEx(nIDTemplate, pParent)
	, m_dwStyle(DIALOG_STYLE_NONE)
	, m_OriginalRect()
	, m_mapWindowRect()
{
}

CFlexibleDialog::CFlexibleDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
	: CDialogEx(lpszTemplateName, pParentWnd)
{
}

void CFlexibleDialog::SetStyle(DWORD dwStyle)
{
	m_dwStyle = dwStyle;
}

void CFlexibleDialog::UpdateWindowRect(void)
{
	GetClientRect(&m_OriginalRect);

	m_mapWindowRect.clear();
	CWnd* pWnd = this->GetWindow(GW_CHILD);
	while (pWnd)
	{
		CRect rtWindow;
		pWnd->GetClientRect(&rtWindow);
		pWnd->MapWindowPoints(this, &rtWindow);
		pWnd = pWnd->GetNextWindow(GW_HWNDNEXT);
	}
}

BOOL CFlexibleDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateWindowRect();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
BEGIN_MESSAGE_MAP(CFlexibleDialog, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


void CFlexibleDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_mapWindowRect.empty())
		return;

	double dRatioX = (double)cx / m_OriginalRect.Width();
	double dRatioY = (double)cy / m_OriginalRect.Height();
	for (auto iter = m_mapWindowRect.begin(); iter != m_mapWindowRect.end(); iter++){
		double dLeft = (double)iter->second.left * dRatioX;
		double dRight = (double)iter->second.right * dRatioX;
		double dTop = (double)iter->second.top * dRatioY;
		double dBottom = (double)iter->second.bottom * dRatioY;
		iter->first->SetWindowPos(NULL, (int)dLeft, (int)dTop, (int)(dRight - dLeft), (int)(dBottom - dTop), SWP_NOZORDER);
	}

	if (m_dwStyle & DIALOG_STYLE_ELLIPTIC)
		//MakeEllipticWindow(this);
	Invalidate(false);
}
