#ifndef UNICODE
#define UNICODE
#endif

#ifndef MAGICDBG_DEBUG
#define MAGICDBG_DEBUG
#endif

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "Windows.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
#include <io.h>

#include <util.h>
#include <magic_debug.h>
#include <err_codes.h>
#include <user_alerts.h>
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "user32.lib") 
#pragma comment(lib, "opengl32.lib")

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
        WS_OVERLAPPEDWINDOW | CS_OWNDC,            // Window style

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
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int  iPixelFormat;  
    
    // get the best available match of pixel format for the device context   
    iPixelFormat = ChoosePixelFormat(hdc, &pfd); 
    
    // make that the pixel format of the device context  
    if (!SetPixelFormat(hdc, iPixelFormat, &pfd)) {
        return ERR_COULDNT_SET_PIXELFORMAT;
    }

    HGLRC tempGLContext = wglCreateContext(hdc);
    if (tempGLContext == NULL) {
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

    wglMakeCurrent(hdc, tempGLContext);
    printf("Made GL context current, last GL error: %u", glGetError());

    GLenum glewInitResult = glewInit();
    glutInit(__p___argc(), __argv);

    if (glewInitResult != GLEW_OK) {
        MessageBoxW(
            hwnd,
            L"Couldn't initialize glew!",
            L"Error",
            MB_OK | MB_ICONERROR
        );
        return ERR_GLEW_COULDNT_LOAD;
    }

    // Recreate GL contet with extensions
    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };

    HGLRC hGLContext = wglCreateContextAttribsARB(hdc, 0, attribs);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempGLContext);
    wglMakeCurrent(hdc, hGLContext);

    // Check availability of shaders
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
    printFramebuffer("vertex shader compiled");

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
    printFramebuffer("fragmentShader compiled");

    free(pfragshader_source);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    

    float vertices[] = {
        -0.5f,  -0.5f,  0.0f,
         0.5f,  -0.5f,  0.0f,
         0.0f,   0.5f,  0.0f
    };  

    unsigned int VBO;
    glGenBuffers(1, &VBO);  
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  
    glBindVertexArray(VAO);
    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 1. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
    // 2. use our shader program when we want to render an object
    glUseProgram(shaderProgram);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SwapBuffers(hdc);
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