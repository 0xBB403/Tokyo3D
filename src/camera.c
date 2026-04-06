/* 
    Tokyo3D\src\camera.c
*/

//#include "tokyo3d.h"
#include "camera.h"

void SPACE_rotate(Space* space, Vec3 axis, float angle)
{
    Vec3 rot_F = rodrigues_rotate(space->F, axis, angle);
    Vec3 rot_U = rodrigues_rotate(space->U, axis, angle);
    Vec3 rot_R = rodrigues_rotate(space->R, axis, angle);
    space->F = rot_F;
    space->U = rot_U;
    space->R = rot_R;
}

Camera make_camera(Vec3 pos, Vec3 dir, float focal, Space world)
{
    Vec3 F = normalize(dir);
    Vec3 R = cross(F, world.U);
    Vec3 U = cross(R, F);
    Space space = (Space){F, U, R};
    return (Camera){pos, space, focal};
}

void CAMERA_orthogonalize(Camera* cam)
{
    Vec3 new_F = normalize(cam->space.F);
    Vec3 new_R = normalize( cross(new_F, cam->space.U) );
    Vec3 new_U = normalize( cross(new_R, new_F) );
    cam->space = (Space){new_F,new_U,new_R};
}

void CAMERA_translate(Camera* cam, Vec3 transl)
{
    cam->pos = add(cam->pos, transl);
}

void CAMERA_rotate(Camera* cam, Vec3 axis, float angle)
{
    SPACE_rotate(&cam->space ,axis, angle);
    CAMERA_orthogonalize(cam);
}