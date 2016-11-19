#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "common.h"

GLuint initializeTexture(const unsigned char *image_data, int width, int height, GLenum format);
void updateTexture(const unsigned char *image_data, int width, int height, GLenum format);

#endif
