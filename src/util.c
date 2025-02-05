#pragma once
#include <util.h>

void printFramebuffer(const char* msg) {
    glColor3f(1.0, 0.0, 0.0);
    glRasterPos2f(0.1, 0.1);

    int len, i;
    len = (int)strlen(msg);

    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, msg[i]);
    }
}