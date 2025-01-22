#ifndef UNICODE
#define UNICODE
#endif
#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdio.h>
#include "Windows.h"
#pragma once

void alertUser(
    HWND        hwnd, 
    DWORD       wErrNo, 
    LPCWSTR     wErrorDescription, 
    UINT        uType
);

#ifdef __cplusplus
}
#endif



