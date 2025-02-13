#pragma once
#include <util.h>

void printToFramebuffer(const char* msg) {
    // glColor3f(1.0, 0.0, 1.0);
    glRasterPos2f(-0.9, -0.9);

    int len, i;
    len = (int)strlen(msg);

    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[i]);
    }
}