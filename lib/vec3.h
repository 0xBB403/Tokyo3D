/* 
    Tokyo3D\lib\vec3.h
*/

// #include "tokyo3d.h"
#include "math.h"
#define PI 3.14159265358979323846f

#ifndef TOKYO3D_VEC3_H
#define TOKYO3D_VEC3_H

typedef struct{
    float x;
    float y;
    float z;
} Vec3;


float Q_rsqrt( float number );

static inline float mod2(Vec3 v)
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}

static inline float mod(Vec3 v)
{
    float mod_inv = Q_rsqrt(mod2(v));
    return 1.0 / mod_inv;
}

static inline Vec3 normalize(Vec3 v)
{
    float m = Q_rsqrt(mod2(v));
    return (Vec3){v.x*m, v.y*m, v.z*m};
}

static inline Vec3 scal(Vec3 v, float k)
{
    return (Vec3){v.x*k, v.y*k, v.z*k};
}

static inline float dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 cross(Vec3 a, Vec3 b)
{
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline Vec3 add(Vec3 a, Vec3 b)
{
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 sub(Vec3 a, Vec3 b)
{
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3 rodrigues_rotate(Vec3 v, Vec3 axis, float angle)
{
    Vec3 norm_axis = normalize(axis);
    float cos_t = cos(-angle);
    float sin_t = sin(-angle);
    Vec3 p1 = scal(v, cos_t);
    Vec3 p2 = scal(cross(norm_axis, v), sin_t);
    Vec3 p3 = scal(norm_axis, dot(norm_axis, v)*(1-cos_t));

    return (Vec3){
        p1.x + p2.x + p3.x,
        p1.y + p2.y + p3.y,
        p1.z + p2.z + p3.z
    };
}


#endif