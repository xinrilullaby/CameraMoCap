#include <loader.h>

int wrangleExtensions(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    // temp context for loading extensions
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
    log_info("Created extension-wrangling context");
    log_info("Last GL error %s", (const char*)(glewGetErrorString(glGetError())));

    wglMakeCurrent(hdc, tempGLContext);
    log_info("Temporary context was made current");

    GLenum glewInitResult = glewInit();
    glutInit(__p___argc(), __argv);

    if (glewInitResult != GLEW_OK) {
        MessageBoxA(
            hwnd,
            "Couldn't initialize glew!",
            "Error",
            MB_OK | MB_ICONERROR
        );
        log_error("GLEW couldn't load (%s)", glewGetErrorString(glewInitResult));
        ERR_GLEW_COULDNT_LOAD;
    }
   
    // Reset and remove the temp GL context
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempGLContext);

    return 0; // Success
}

GLuint loadShader(const char* path, GLuint type) {
    GLint   shaderCompResult;
    GLint   shaderInfoLogSize = 0;
    GLchar* shaderInfoLog;

    // Read vertex shader from file
    FILE* fpshader = fopen(path, "r");
    fseek(fpshader, 0L, SEEK_END); // Go to EOF
    size_t fpshader_size = ftell(fpshader);
    fseek(fpshader, 0L, SEEK_SET);
    char* pshader_source = (char *)calloc(fpshader_size + 1, sizeof(char) ); // Allocate C string of file size
    pshader_source[fpshader_size] = '\0';
    rewind(fpshader); // Go to beginning of file
    fread(pshader_source, sizeof(char), fpshader_size, fpshader);

    // Compile vertex shader object
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &pshader_source, NULL);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompResult);
    // Error handling
    if (shaderCompResult == GL_FALSE) {
        // Get shader info length
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &shaderInfoLogSize);
        // Allocate memory for this diva
        shaderInfoLog = (GLchar*)malloc(sizeof(char) * shaderInfoLogSize);
        glGetShaderInfoLog(
            shader, 
            shaderInfoLogSize, 
            &shaderInfoLogSize, 
            shaderInfoLog
        );
        log_error(shaderInfoLog);
        log_error("Failed to compile vertex shader, dumping shader source below:");
        log_debug(pshader_source);
        free(pshader_source);
        return (void*)0;
    } else {
        log_info("Vertex shader compiled successfully");
        free(pshader_source);
        return shader;
    }
}