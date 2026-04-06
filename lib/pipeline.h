/* 
    Tokyo3D\lib\pipeline.h
*/
// pipeline: transform -> clipping(Z -> projection -> screen) -> interpolation

// #include "tokyo3d.h"
#include "camera.h"
#include <stdlib.h> // NULL, malloc, free

#define t3_bool int
#define t3_true 1
#define t3_false 0

#define NEAR_PLANE 0.1f
#define FAR_PLANE 5.0f
#define NEAR_INV (1.0f / NEAR_PLANE)

#define VB_MAX_CAPACITY 50000

#ifndef TOKYO3D_PIPELINE_H
#define TOKYO3D_PIPELINE_H

typedef struct{
    float* data; // ptr to VertexBuffer

    float* x;
    float* y;
    float* z;

    float* r;
    float* g;
    float* b;

    float* u; // for textures
    float* v; // for textures
 
    float* z_inv;

    unsigned int len;
} VertexBuffer; // SoA

typedef struct{
    float x;
    float y;
    float z;

    float r;
    float g;
    float b;

    float u;
    float v;

    float z_inv;
} Vertex;

typedef struct{
    float a, b, c, d;
} Plane;

static Plane t3_near_plane = {0, 0, 1, -NEAR_PLANE};  // z >= near
static Plane t3_far_plane  = {0, 0, -1, FAR_PLANE};   // z <= far
static Plane t3_left   = { 1, 0, 0, 1};
static Plane t3_right  = {-1, 0, 0, 1};
static Plane t3_bottom = {0,  1, 0, 1};
static Plane t3_top    = {0, -1, 0, 1};


void VB_alloc(VertexBuffer* vb); // Used to allocate a VertexBuffer before hot loop
void VB_free(VertexBuffer* vb); // Free VertexBuffer after hot loop (outside)
void VB_transform(const VertexBuffer* in, VertexBuffer* out, const Camera* cam);
void VB_Z_clipping(const VertexBuffer* in, VertexBuffer* out); // TOFIX: atm is broken
void VB_project(const VertexBuffer* in, VertexBuffer* out, const Camera* cam);
void VB_screen_clipping(const VertexBuffer* in, VertexBuffer* out); // REMEBER: Not really necessary due to resterization structures

#endif