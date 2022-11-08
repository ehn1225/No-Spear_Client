/*
Source File : AutoUpdate.h
Created for the purpose of demonstration for http://www.codeproject.com

Copyright 2017 Michael Haephrati, Secured Globe Inc.
See also: https://www.codeproject.com/script/Membership/View.aspx?mid=5956881

Secured Globe, Inc.
http://www.securedglobe.com

*/

#pragma once
typedef struct{
	int Major;
	int Minor;
	int Revision;
	int SubRevision;
} SG_Version;

class AutoUpdate 
{
public:
	AutoUpdate();
	~AutoUpdate();
	BOOL SG_GetVersion(LPWSTR ExeFile, SG_Version *ver);
	void AddNextVersionToFileName(CString& ExeFile, SG_Version ver);
	CString GetSelfFullPath();
	BOOL ReplaceTempVersion();
	BOOL SG_VersionMatch(CString ExeFile, SG_Version *ver);
	BOOL CheckForUpdates(void);
	void SetSelfFullPath(CString Path);
	CString GetSelfFileName();
	void SetSelfFileName(CString FileName);
	BOOL AutoUpdate::ApplyAgentUpdate(CString Agent, BOOL ReplaceSelf);

private:
	bool tempversion;			// indicate this is a temp version
	CString m_SelfFullPath;
	CString m_SelfFileName;
	CString m_DownloadLink = _T("http://4nul.org:3000/");
	CString m_NextVersion;
	BOOL _downloading = FALSE;
};


using namespace std;

class MyCallback : public IBindStatusCallback
{
public:
	MyCallback() {}

	~MyCallback() { }

	// This one is called by URLDownloadToFile
	STDMETHOD(OnProgress)(/* [in] */ ULONG ulProgress, /* [in] */ ULONG ulProgressMax, /* [in] */ ULONG ulStatusCode, /* [in] */ LPCWSTR wszStatusText)
	{
		// You can use your own logging function here
		wprintf(L"Downloaded %d of %d. Status code %d\n", ulProgress, ulProgressMax, ulStatusCode);
		return S_OK;
	}

	STDMETHOD(OnStartBinding)(/* [in] */ DWORD dwReserved, /* [in] */ IBinding __RPC_FAR *pib)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetPriority)(/* [out] */ LONG __RPC_FAR *pnPriority)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnLowResource)(/* [in] */ DWORD reserved)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnStopBinding)(/* [in] */ HRESULT hresult, /* [unique][in] */ LPCWSTR szError)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetBindInfo)(/* [out] */ DWORD __RPC_FAR *grfBINDF, /* [unique][out][in] */ BINDINFO __RPC_FAR *pbindinfo)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnDataAvailable)(/* [in] */ DWORD grfBSCF, /* [in] */ DWORD dwSize, /* [in] */ FORMATETC __RPC_FAR *pformatetc, /* [in] */ STGMEDIUM __RPC_FAR *pstgmed)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnObjectAvailable)(/* [in] */ REFIID riid, /* [iid_is][in] */ IUnknown __RPC_FAR *punk)
	{
		return E_NOTIMPL;
	}

	// IUnknown stuff
	STDMETHOD_(ULONG, AddRef)()
	{
		return 0;
	}

	STDMETHOD_(ULONG, Release)()
	{
		return 0;
	}

	STDMETHOD(QueryInterface)(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		return E_NOTIMPL;
	}
};


