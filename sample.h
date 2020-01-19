#pragma once

#define STRICT              // 型チェックを厳密に行なう
#define WIN32_LEAN_AND_MEAN // 从 Windows 头文件中排除极少使用的信息
#define WINVER       0x0601 // Windows Vista以降対応アプリを指定
#define _WIN32_WINNT 0x0601 // 同上

// Windows 头文件: 
#include <windows.h>
#include <wrl.h>

// C 运行时头文件
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define HR_CHECK(_hr_) hr = _hr_; if (FAILED(hr)) { goto exit; }
#define BOOL_CHECK(_hr_) if (!(_hr_)) { hr = HRESULT_FROM_WIN32(::GetLastError()); goto exit; }

using namespace Microsoft::WRL;

#include "Shader.h"
#include "Model.h"
#include "Graphic.h"
#include "Frame.h"
#include "Resource.h"