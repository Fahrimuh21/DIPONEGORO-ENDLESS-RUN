#pragma once

#include <GL/gl.h>

// Loads a texture from disk. Path should be relative to executable, e.g. "texture/undip1.bmp"
GLuint loadTexture(const char* filename);
