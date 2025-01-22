#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include "Windows.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <io.h>


#include <err_codes.h>
#include <user_alerts.h>
#pragma comment(lib,"user32.lib") 
#pragma comment(lib,"opengl32.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"CameraMoCap",                 // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Get GDI device context
    HDC hdc = GetDC(hwnd);
    wglMakeCurrent(hdc, NULL);
    
    // Set pixel format on device descriptor
    // before creating a rendering context
    PIXELFORMATDESCRIPTOR pfd = { 
        sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
        1,                     // version number  
        PFD_DRAW_TO_WINDOW |   // support window  
        PFD_SUPPORT_OPENGL |   // support OpenGL  
        PFD_DOUBLEBUFFER,      // double buffered  
        PFD_TYPE_RGBA,         // RGBA type  
        24,                    // 24-bit color depth  
        0, 0, 0, 0, 0, 0,      // color bits ignored  
        0,                     // no alpha buffer  
        0,                     // shift bit ignored  
        0,                     // no accumulation buffer  
        0, 0, 0, 0,            // accum bits ignored  
        32,                    // 32-bit z-buffer  
        0,                     // no stencil buffer  
        0,                     // no auxiliary buffer  
        PFD_MAIN_PLANE,        // main layer  
        0,                     // reserved  
        0, 0, 0                // layer masks ignored  
    }; 
    int  iPixelFormat;  
    
    // get the best available match of pixel format for the device context   
    iPixelFormat = ChoosePixelFormat(hdc, &pfd); 
    
    // make that the pixel format of the device context  
    if (!SetPixelFormat(hdc, iPixelFormat, &pfd)) {
        return ERR_COULDNT_SET_PIXELFORMAT;
    }

    HGLRC hGLContext = wglCreateContext(hdc);
    if (hGLContext == NULL) {
        WCHAR buffer[128];
        swprintf_s(buffer, L"Error %u", GetLastError());
        // Print message box with the error
        MessageBoxW(
            hwnd,
            L"Couldn't create OpenGL context",
            buffer,
            MB_OK
        );
        // Let's ignore this error and try moving on
        return ERR_COULDNT_CREATE_OPENGL_CONTEXT;
    }
    printf("Created GL context, last GL error: %u", glGetError());

    wglMakeCurrent(hdc, hGLContext);
    printf("Made GL context current, last GL error: %u", glGetError());

    GLenum glewInitResult = glewInit();

    if (glewInitResult != GLEW_OK) {
        MessageBoxW(
            hwnd,
            L"Couldn't initialize glew!",
            L"Error",
            MB_OK | MB_ICONERROR
        );
        return ERR_GLEW_COULDNT_LOAD;
    }

    if (
        !_access("testshader.fsh", 0) == 0 ||
        !_access("testshader.vsh", 0) == 0
    ) {
        // Print message box with the error
        MessageBoxW(
            hwnd,
            L"Test shaders not found!",
            L"Error",
            MB_OK | MB_ICONERROR
        );
    } 
    FILE* fpvertshader = fopen("testshader.vsh", "r");
    fseek(fpvertshader, 0L, SEEK_END); // Go to EOF
    size_t fpvertshader_size = ftell(fpvertshader);
    char* pvertshader_source = (char *)malloc(sizeof(char) * fpvertshader_size); // Allocate C string of file size
    rewind(fpvertshader); // Go to beginning of file
    
    for (int i = 0; i < fpvertshader_size; i++) {
        *(pvertshader_source) = getc(fpvertshader);
    }

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &pvertshader_source, NULL);
    glCompileShader(vertexShader);
    free(pvertshader_source);

    FILE* fpfragshader = fopen("testshader.fsh", "r");
    fseek(fpfragshader, 0L, SEEK_END); // Go to EOF
    size_t fpfragshader_size = ftell(fpvertshader);
    char* pfragshader_source = (char *)malloc(sizeof(char) * fpfragshader_size); // Allocate C string of file size
    rewind(fpfragshader); // Go to beginning of file
    
    for (int i = 0; i < fpfragshader_size; i++) {
        *(pfragshader_source) = getc(fpfragshader);
    }
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &pfragshader_source, NULL);
    glCompileShader(fragmentShader);
    free(pfragshader_source);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}