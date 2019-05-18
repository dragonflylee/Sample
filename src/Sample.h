#pragma once

#define WINVER       0x0601
#define _WIN32_WINNT 0x0601

// ATL 头文件:
#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlcrack.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlribbon.h>
#include <atlcoll.h>

#define HR_CHECK(_hr_) hr = _hr_; if (FAILED(hr)) { ATLTRACE(TEXT("0x%.8x\n"), hr); goto exit; }
#define BOOL_CHECK(_hr_) if (!(_hr_)) { hr = HRESULT_FROM_WIN32(::GetLastError()); ATLTRACE(TEXT("0x%.8x\n"), hr); goto exit; }

#include "Resource.h"
#include "MainFrm.h"