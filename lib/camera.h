/*
    Tokyo3D\lib\camera.h
*/

// #include "tokyo3d.h"
#include "vec3.h"

#ifndef TOKYO3D_CAMERA_H
#define TOKYO3D_CAMERA_H

typedef struct {
    Vec3 F;
    Vec3 U;
    Vec3 R;
} Space;

void SPACE_rotate(Space* space, Vec3 axis, float angle);

typedef struct{
    Vec3 pos;
    Space space;
    float focal;
} Camera;

Camera make_camera(Vec3 pos, Vec3 dir, float focal, Space world);
void CAMERA_orthogonalize(Camera* cam);
void CAMERA_translate(Camera* cam, Vec3 transl);
void CAMERA_rotate(Camera* cam, Vec3 axis, float angle);


#endif
