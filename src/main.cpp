#include <constants.h>

extern "C" {
    #include <log.h>
}
#include "Windows.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
#include <io.h>

#include <util.h>
#include <magic_debug.h>
#include <err_codes.h>
#include <user_alerts.h>
#include <loader.h>
// Libraries for the CL linker
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "user32.lib") 
#pragma comment(lib, "opengl32.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Internal vars and bufs
    char* debugMessageBuffer = (char*)malloc(sizeof(char) * CAMERAMOCAP_DEBUGBUF_SZ);  // Buffer for debug messages
    // Register the window class.
    const wchar_t   CLASS_NAME[]    = L"Sample Window Class";
    WNDCLASS        wc              = { };
    wc.lpfnWndProc                  = WindowProc;
    wc.hInstance                    = hInstance;
    wc.lpszClassName                = CLASS_NAME;
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
    // Check HWND handle to be non-null
    if (hwnd == NULL)
        return ERR_NULL_HWND_RETURNED;

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
    // loader.h
    wrangleExtensions(hwnd);
    // Recreate GL context with extensions
    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };
    // Create the actual context for rendering
    HGLRC hGLContext = wglCreateContextAttribsARB(hdc, 0, attribs);
    if (hGLContext == NULL) {
        MessageBoxA(
            hwnd,
            "Error occurred when creating the OpenGL rendering context. Check GPU drivers?",
            "Initialization error",
            MB_OK | MB_ICONERROR
        );
        return ERR_COULDNT_CREATE_OPENGL_CONTEXT;
    }
    // Make the new context current
    wglMakeCurrent(hdc, hGLContext);
    // Check availability of shader source files
    if (
        !_access("testshader.frag", 0) == 0 ||
        !_access("testshader.vert", 0) == 0
    ) {
        // Print message box with the error
        MessageBoxW(
            hwnd,
            L"Test shaders not found!",
            L"Error",
            MB_OK | MB_ICONERROR
        );
    } 
    // loadShader(const char* path, GLuint type) reads and compiles shaders
    GLuint vertexShader     = loadShader("testshader.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader   = loadShader("testshader.frag", GL_FRAGMENT_SHADER);
    GLuint shaderProgram;
    shaderProgram = glCreateProgram();
    if (vertexShader != 0)
        glAttachShader(shaderProgram, vertexShader);
    if (fragmentShader != 0)
        glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint programInfoLogSize = 0;

        log_error("Failed to link shader program");
        debugMessageBuffer = strcat(debugMessageBuffer, "program linking failed\n");
        debugMessageBuffer = strcat(
            debugMessageBuffer, 
            reinterpret_cast<const char *>(
                glewGetErrorString(glGetError())
            )
        );
        // Query OpenGL for info logs
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &programInfoLogSize);
        // Allocate memory for this diva
        char* programInfoLog = (GLchar*)malloc(sizeof(char) * programInfoLogSize);
        glGetProgramInfoLog(shaderProgram, programInfoLogSize, &programInfoLogSize, programInfoLog);
        log_error(programInfoLog);
        log_error("Failed to compile fragment shader, dumping shader source below:");
        free(programInfoLog);
    }
    glValidateProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // Vertex data
    GLfloat vertices[] = {
        -0.5f,  -0.5f,  0.0f,
         0.5f,  -0.5f,  0.0f,
         0.0f,   0.5f,  0.0f
    };  

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0, 
        3, 
        GL_FLOAT, 
        GL_FALSE, 
        3 * sizeof(float), 
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        glClearColor(0.2f, 0.0f, 0.2f, 0.2f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // printToFramebuffer(debugMessageBuffer);
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