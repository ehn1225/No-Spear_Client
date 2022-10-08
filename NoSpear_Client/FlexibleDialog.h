#pragma once

enum E_DIALOG_STYLE
{
	DIALOG_STYLE_NONE = 0,
	DIALOG_STYLE_ELLIPTIC = 0x01 << 0,
};

class CFlexibleDialog : public CDialogEx
{
	DWORD m_dwStyle;
	CRect m_OriginalRect;
	std::map<CWnd*, CRect> m_mapWindowRect;

public:
	CFlexibleDialog(UINT nIDTemplate, CWnd* pParent = NULL);
	CFlexibleDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);

	void SetStyle(DWORD dwStyle = DIALOG_STYLE_NONE);
	void UpdateWindowRect(void);

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

