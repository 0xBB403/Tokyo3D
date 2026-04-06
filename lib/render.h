/* 
    Tokyo3D\lib\render.h
*/

// #include "tokyo3d.h"
#include "pipeline.h"
#include <stdint.h>

#ifndef TOKYO3D_RENDER_H
#define TOKYO3D_RENDER_H

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MIN3(a,b,c) MIN(MIN(a,b), (c))

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MAX3(a,b,c) MAX(MAX(a,b), (c))

typedef struct {
    uint32_t* color;
    float* z;
    unsigned int W;
    unsigned int H;
    unsigned int size;
} FrameBuffer;

typedef struct {
    uint32_t* pixels;
    int width;
    int height;
} Texture;

void FB_alloc(FrameBuffer* fb, const unsigned int W, const unsigned int H);
void FB_free(FrameBuffer* fb);
void FB_clear(FrameBuffer* fb, uint32_t color);

void rasterization(FrameBuffer* fb, const VertexBuffer* vb, const Texture* txt);


#endif