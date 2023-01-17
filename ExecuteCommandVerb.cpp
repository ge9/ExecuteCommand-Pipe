// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This demonstrates how implement a shell verb using the ExecuteCommand method
// this method is preferred for verb implementations as it provides the most flexibility,
// it is simple, and supports out of process activation.
//
// This sample implements a standalone local server COM object but
// it is expected that the verb implementation will be integrated
// into existing applications. to do that have your main application object
// register a class factory for itself and have that object implement IDropTarget
// for the verbs of your application. Note that COM will launch your application
// if it is not already running and will connect to an already running instance
// of your application if it is already running. These are features of the COM
// based verb implementation methods.
//
// It is also possible (but not recommended) to create in process implementations
// of this object. To do that follow this sample but replace the local
// server COM object with an in-proc server.

#include "ShellHelpers.h"
#include "VerbHelpers.h"
#include "RegisterExtension.h"
#include <strsafe.h>
#include <new>  // std::nothrow
#include <atlbase.h>
#include "myuuid.h"
// Each ExecuteCommand handler needs to have a unique COM object, run UUIDGEN.EXE to
// create new CLSID values for your handler. These handlers can implement multiple
// different verbs using the information provided via IInitializeCommand, for example the verb name.
// your code can switch off those different verb names or the properties provided
// in the property bag.

WCHAR const c_szVerbDisplayName[] = L"ExecuteCommand Verb Sample";
const int MAX_PATH_CHARS = 3000;//32767 in NTFS, but that's too much

class __declspec(uuid(MYUUID))
    CExecuteCommandVerb : public IExecuteCommand,
                          public IObjectWithSelection,
                          public IInitializeCommand,
                          public IObjectWithSite,
                          CAppMessageLoop
{
public:
    CExecuteCommandVerb(PWSTR pszCmdLine) : _cRef(1), _psia(NULL), _punkSite(NULL)
    {
        pszCmdLine_field = pszCmdLine;
    }

    HRESULT Run();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CExecuteCommandVerb, IExecuteCommand),        // required
            QITABENT(CExecuteCommandVerb, IObjectWithSelection),   // required
            QITABENT(CExecuteCommandVerb, IInitializeCommand),     // optional
            QITABENT(CExecuteCommandVerb, IObjectWithSite),        // optional
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IExecuteCommand
    IFACEMETHODIMP SetKeyState(DWORD grfKeyState)
    {
        _grfKeyState = grfKeyState;
        return S_OK;
    }

    IFACEMETHODIMP SetParameters(PCWSTR /* pszParameters */)
    { return S_OK; }

    IFACEMETHODIMP SetPosition(POINT pt)
    {
        _pt = pt;
        return S_OK;
    }

    IFACEMETHODIMP SetShowWindow(int nShow)
    {
        _nShow = nShow;
        return S_OK;
    }

    IFACEMETHODIMP SetNoShowUI(BOOL /* fNoShowUI */)
    { return S_OK; }

    IFACEMETHODIMP SetDirectory(PCWSTR /* pszDirectory */)
    { return S_OK; }

    IFACEMETHODIMP Execute();

    // IObjectWithSelection
    IFACEMETHODIMP SetSelection(IShellItemArray *psia)
    {
        SetInterface(&_psia, psia);
        return S_OK;
    }

    IFACEMETHODIMP GetSelection(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        return _psia ? _psia->QueryInterface(riid, ppv) : E_FAIL;
    }

    // IInitializeCommand
    IFACEMETHODIMP Initialize(PCWSTR /* pszCommandName */, IPropertyBag * /* ppb */)
    {
        // The verb name is in pszCommandName, this handler can varry its behavior
        // based on the command name (implementing different verbs) or the
        // data stored under that verb in the registry can be read via ppb
        return S_OK;
    }

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *punkSite)
    {
        SetInterface(&_punkSite, punkSite);
        return S_OK;
    }

    IFACEMETHODIMP GetSite(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;
    }

    void OnAppCallback()
    {
        DWORD count;
        _psia->GetCount(&count);
        CComPtr<IShellItem2> psi;

        PWSTR pszName = pszName0;
        const WCHAR LF = '\n';
        //HRESULT hr ;
        //HRESULT hr2;
        if (pszCmdLine_field[0] == '-' || pszCmdLine_field[0] == 'd') {//only "-Embedding" or "d" --> debug
            //debug; show dialog
            GetItemAt(_psia, 0, IID_PPV_ARGS(&psi));//the first item
            psi->GetDisplayName(SIGDN_FILESYSPATH, &pszName);

            CComPtr<IShellItem2> psi2;
            WCHAR* pszName2 = new WCHAR[MAX_PATH_CHARS];
            GetItemAt(_psia, count - 1, IID_PPV_ARGS(&psi2));//the last item
            psi2->GetDisplayName(SIGDN_FILESYSPATH, &pszName2);

            WCHAR uuid_wide[50];
            size_t converted;
            mbstowcs_s(&converted, uuid_wide, MYUUID, sizeof(MYUUID));
            WCHAR* szMsg = new WCHAR[MAX_PATH_CHARS * 2 + 400];
            StringCchPrintf(szMsg, MAX_PATH_CHARS * 2 + 400, L"UUID:%s, arg:[%s], %d item(s), first item: [%s], last item: [%s]", uuid_wide, pszCmdLine_field, count, pszName, pszName2);
            MessageBox(NULL, szMsg, c_szVerbDisplayName, MB_OK | MB_SETFOREGROUND);
            delete[] pszName2;
            delete[] szMsg;

        }
        else {//command specified
            pszCmdLine_field[wcslen(pszCmdLine_field) - 11] = 0;//ignore " -Embedding"

            //create pipe
            HANDLE hPipe1[2];
            HANDLE hChildRead;
            CreatePipe(&hPipe1[0], &hPipe1[1], NULL, 0);
            DuplicateHandle(GetCurrentProcess(), hPipe1[0], GetCurrentProcess(), &hChildRead, 0, TRUE, DUPLICATE_SAME_ACCESS);
            CloseHandle(hPipe1[0]);

            //prepare STARTUPINFO
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = hChildRead;
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            //if (pszCmdLine_field[0] == 'h') si.wShowWindow = SW_HIDE; // hide window

            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(pi));
            //ignore first 2 chars & optionally hide window
            CreateProcess(NULL, &pszCmdLine_field[2], NULL, NULL, TRUE, (pszCmdLine_field[0] == 'h') ? CREATE_NO_WINDOW : 0, NULL, NULL, &si, &pi);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(hChildRead);
            //write to stdin
            DWORD written = 0;
            for (DWORD i = 0; i < count; i++) {
                GetItemAt(_psia, i, IID_PPV_ARGS(&psi));
                psi->GetDisplayName(SIGDN_FILESYSPATH, &pszName);
                WideCharToMultiByte(CP_UTF8, 0, pszName, -1, mbfile, MAX_PATH_CHARS * 4, NULL, NULL);//to UTF-8
                WriteFile(hPipe1[1], mbfile, (DWORD)strlen(mbfile), &written, NULL);
                WriteFile(hPipe1[1], &LF, 1, &written, NULL);//newline
            }
            CloseHandle(hPipe1[1]);
        }
    }

private:
    ~CExecuteCommandVerb()
    {
    }
    WCHAR* pszCmdLine_field;
    long _cRef;
    CComPtr<IShellItemArray> _psia;
    CComPtr<IUnknown> _punkSite;
    HWND _hwnd;
    POINT _pt;
    int _nShow;
    DWORD _grfKeyState;

    CHAR mbfile[MAX_PATH_CHARS * 4];
    WCHAR pszName0[MAX_PATH_CHARS];
};

// this is called to invoke the verb but this call must not block the caller. to accomidate that
// this function captures the state it needs to invoke the verb and queues a callback via
// the message queue. if your application has a message queue of its own this can be accomilished
// using PostMessage() or setting a timer of zero seconds

IFACEMETHODIMP CExecuteCommandVerb::Execute()
{
    // capture state from the site needed to invoke the verb here
    // note the HWND can be retrieved here but it should not be used for modal UI as
    // all shell verbs should be modeless as well as not block the caller
    IUnknown_GetWindow(_punkSite, &_hwnd);

    // queue the execution of the verb via the message pump
    QueueAppCallback();

    return S_OK;
}

HRESULT CExecuteCommandVerb::Run()
{
    CStaticClassFactory<CExecuteCommandVerb> classFactory(static_cast<IObjectWithSite*>(this));

    HRESULT hr = classFactory.Register(CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE);
    if (SUCCEEDED(hr))
    {
        MessageLoop();
    }
    return S_OK;
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR pszCmdLine, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        DisableComExceptionHandling();
        if (StrStrI(pszCmdLine, L"-Embedding"))
        {
            CExecuteCommandVerb* pAppDrop = new (std::nothrow) CExecuteCommandVerb(pszCmdLine);
            if (pAppDrop)
            {
                pAppDrop->Run();
                pAppDrop->Release();
            }
        }
        else
        {
            CRegisterExtension re(__uuidof(CExecuteCommandVerb));

            hr = re.RegisterAppAsLocalServer(c_szVerbDisplayName);
            MessageBoxA(NULL,
                "Installed ExecuteCommand Verb Sample as UUID:{" MYUUID "}",
                "ExecuteCommand-Pipe", MB_OK);
        }

        CoUninitialize();
    }

    return 0;
}
