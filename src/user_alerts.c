#include <user_alerts.h>

void alertUser(
    HWND        hwnd, 
    DWORD       wErrNo, 
    LPCWSTR     wErrorDescription, 
    UINT        uType
) {
    // Create a wchar buffer to concatenate the word error and the error number
    WCHAR buffer[128];
    swprintf_s(buffer, L"Error %u", wErrNo);
    // Print message box with the error
    MessageBoxW(
        hwnd,
        wErrorDescription,
        buffer,
        uType
    );
}
    