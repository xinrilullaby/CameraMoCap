#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifdef __cplusplus
extern "C" 
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <log.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/freeglut_ucall.h>

#include "Windows.h"

#include <err_codes.h>

int    wrangleExtensions(HWND hwnd);
GLuint  loadShader(const char* path, GLuint type);

#ifdef __cplusplus
}
#endif