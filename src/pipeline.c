/* 
    Tokyo3D\src\pipeline.c
*/

//#include "tokyo3d.h"
#include "pipeline.h"
#include "camera.h"


void VB_alloc(VertexBuffer* vb)
{

    vb->data = malloc(sizeof(float) * VB_MAX_CAPACITY * 9);

    if (!vb->data) // ERROR: malloc did not work
    {
        vb->len = 0;
        return;
    }

    vb->x = vb->data;
    vb->y = vb->data + VB_MAX_CAPACITY;
    vb->z = vb->data + VB_MAX_CAPACITY * 2;

    vb->r = vb->data + VB_MAX_CAPACITY * 3;
    vb->g = vb->data + VB_MAX_CAPACITY * 4;
    vb->b = vb->data + VB_MAX_CAPACITY * 5;

    vb->u = vb->data + VB_MAX_CAPACITY * 6;
    vb->v = vb->data + VB_MAX_CAPACITY * 7;

    vb->z_inv = vb->data + VB_MAX_CAPACITY * 8;

    vb->len = 0;
}

void VB_free(VertexBuffer* vb)
{
    free(vb->data);
    vb->data = NULL;

    vb->x = NULL;
    vb->y = NULL;
    vb->z = NULL;
    vb->r = NULL;
    vb->g = NULL;
    vb->b = NULL;
    vb->u = NULL;
    vb->v = NULL;
    vb->z_inv = NULL;

    vb->len = 0;
}

void VB_transform(const VertexBuffer* in, VertexBuffer* out, const Camera* cam)
{
    out->len = in->len;

    for (unsigned int i=0; i<in->len; i++)
    {
        float x = in->x[i] - cam->pos.x;
        float y = in->y[i] - cam->pos.y;
        float z = in->z[i] - cam->pos.z;

        out->x[i] = cam->space.R.x*x + cam->space.R.y*y + cam->space.R.z*z;
        out->y[i] = cam->space.U.x*x + cam->space.U.y*y + cam->space.U.z*z;
        out->z[i] = cam->space.F.x*x + cam->space.F.y*y + cam->space.F.z*z;

        out->r[i] = in->r[i];
        out->g[i] = in->g[i];
        out->b[i] = in->b[i];

        out->u[i] = in->u[i];
        out->v[i] = in->v[i];

        if (out->z[i] != 0.0f)
            out->z_inv[i] = 1.0f / out->z[i];
        else
            out->z_inv[i] = 0.0f;
    }
}

static inline void copy_vertex(VertexBuffer* vb, unsigned int idx, Vertex v) {
    vb->x[idx] = v.x;
    vb->y[idx] = v.y;
    vb->z[idx] = v.z;
    vb->r[idx] = v.r;
    vb->g[idx] = v.g;
    vb->b[idx] = v.b;
    vb->u[idx] = v.u;
    vb->v[idx] = v.v;
    vb->z_inv[idx] = v.z_inv;
}

static inline Vertex get_vertex(const VertexBuffer* vb, int idx)
{
    return (Vertex){
        vb->x[idx],
        vb->y[idx],
        vb->z[idx],
        vb->r[idx],
        vb->g[idx],
        vb->b[idx],
        vb->u[idx],
        vb->v[idx],
        vb->z_inv[idx]
    };
}

static inline float plane_dist(Plane p, Vertex v) { return p.a*v.x + p.b*v.y + p.c*v.z + p.d; }

static Vertex intersect(Vertex a, Vertex b, Plane p)
{
    float da = plane_dist(p, a);
    float db = plane_dist(p, b);

    float t = da / (da - db);

    Vertex out = {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.u + (b.u - a.u) * t,
        a.v + (b.v - a.v) * t,
        1.0f / (a.z + (b.z - a.z) * t) // REMEMBER: do NOT interpolate the z_inv
    };

    return out;
}

int clip_polygon_plane(Vertex* in, int in_count, Vertex* out, Plane p)
{
    int out_count = 0;

    for (int i = 0; i < in_count; i++)
    {
        Vertex current = in[i];
        Vertex prev = in[(i + in_count - 1) % in_count];

        float dc = plane_dist(p, current);
        float dp = plane_dist(p, prev);

        int curr_inside = dc >= 0;
        int prev_inside = dp >= 0;

        if (curr_inside && prev_inside)
        {
            out[out_count++] = current;
        }
        else if (prev_inside && !curr_inside)
        {
            out[out_count++] = intersect(prev, current, p);
        }
        else if (!prev_inside && curr_inside)
        {
            out[out_count++] = intersect(prev, current, p);
            out[out_count++] = current;
        }
    }

    return out_count;
}


// Works on (-1, 1) to (1, -1)
void VB_Z_clipping(const VertexBuffer* in, VertexBuffer* out)  // Z_clipping, now less messy
{
    Vertex poly1[8], poly2[8];
    int count;

    unsigned int out_i = 0;

    for (unsigned int i=0; i<in->len; i+=3)
    {
        // start triangle
        poly1[0] = get_vertex(in, i);
        poly1[1] = get_vertex(in, i+1);
        poly1[2] = get_vertex(in, i+2);
        count = 3;

        // Z clipping
        count = clip_polygon_plane(poly1, count, poly2, t3_near_plane);
        if (count == 0) continue;

        count = clip_polygon_plane(poly2, count, poly1, t3_far_plane);
        if (count == 0) continue;

        // TRIANGULATION 
        for (int k = 1; k < count - 1; k++)
        {
            if (out_i + 3 > VB_MAX_CAPACITY) return;

            copy_vertex(out, out_i,     poly1[0]);
            copy_vertex(out, out_i + 1, poly1[k]);
            copy_vertex(out, out_i + 2, poly1[k+1]);

            out_i += 3;
        }
    }

    out->len = out_i;
}

void VB_project(const VertexBuffer* in, VertexBuffer* out, const Camera* cam)
{
    out->len = in->len;

    for (unsigned int i = 0; i < in->len; i++)
    {
        float z = in->z[i];

        // no division by zero
        if (z == 0.0f) z = 0.0001f;

        out->x[i] = (in->x[i] / z) * cam->focal;
        out->y[i] = (in->y[i] / z) * cam->focal;
        out->z[i] = z;

        out->r[i] = in->r[i];
        out->g[i] = in->g[i];
        out->b[i] = in->b[i];

        out->u[i] = in->u[i];
        out->v[i] = in->v[i];

        out->z_inv[i] = in->z_inv[i];
    }
}

// TOFIX: problem in angle vertexes
void VB_screen_clipping(const VertexBuffer* in, VertexBuffer* out)
{
    Vertex poly1[8], poly2[8];
    int count;

    unsigned int out_i = 0;

    for (unsigned int i = 0; i < in->len; i += 3)
    {
        // triangolo iniziale
        poly1[0] = get_vertex(in, i);
        poly1[1] = get_vertex(in, i+1);
        poly1[2] = get_vertex(in, i+2);
        count = 3;

        // clipping
        count = clip_polygon_plane(poly1, count, poly2, t3_left);
        if (count == 0) continue;

        count = clip_polygon_plane(poly2, count, poly1, t3_right);
        if (count == 0) continue;

        count = clip_polygon_plane(poly1, count, poly2, t3_bottom);
        if (count == 0) continue;

        count = clip_polygon_plane(poly2, count, poly1, t3_top);
        if (count == 0) continue;

        // triangulation
        for (int k = 1; k < count - 1; k++)
        {
            if (out_i + 3 > VB_MAX_CAPACITY) return;

            copy_vertex(out, out_i,     poly1[0]);
            copy_vertex(out, out_i + 1, poly1[k]);
            copy_vertex(out, out_i + 2, poly1[k+1]);

            out_i += 3;
        }
    }

    out->len = out_i;
}