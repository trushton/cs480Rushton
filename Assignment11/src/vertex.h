#ifndef VERTEX_H
#define	VERTEX_H

struct textureVertex
{
    GLfloat position[3];
    GLfloat uv[2];
    GLfloat normal[3];
};

struct colorVertex
{
  GLfloat position[3];
  GLfloat color[3];
};

#endif