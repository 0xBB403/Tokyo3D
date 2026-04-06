/* 
    Tokyo3D\src\render.c
*/

//#include "tokyo3d.h"
#include "render.h"
#include <stdio.h>

void FB_alloc(FrameBuffer* fb, const unsigned int W, const unsigned int H)
{
    fb->W = W;
    fb->H = H;
    fb->size = W*H;
    fb->color = malloc(sizeof(uint32_t) * fb->size);
    fb->z = malloc(sizeof(float) * fb->size);
}

void FB_free(FrameBuffer* fb)
{
    fb->size = 0;
    free(fb->color);
    fb->color = NULL;
    free(fb->z);
    fb->z = NULL;
}

void FB_clear(FrameBuffer* fb, uint32_t color)
{
    for (unsigned int i=0; i < fb->size; i++)
    {
        fb->color[i] = color;
        fb->z[i] = FAR_PLANE; // IMPORTANT
    }
}

void rasterization(FrameBuffer* fb, const VertexBuffer* vb, const Texture* txt)
{
    uint32_t background_color = 0x141E1EFF; // cool gray background
    FB_clear(fb, background_color);

    // Rasterization
    for (unsigned int i=0; i<vb->len; i+=3)
    {
        unsigned int a_idx = i;
        unsigned int b_idx = i+1;
        unsigned int c_idx = i+2;

        float ax = vb->x[a_idx]; float bx = vb->x[b_idx]; float cx = vb->x[c_idx];
        float ay = vb->y[a_idx]; float by = vb->y[b_idx]; float cy = vb->y[c_idx];
        float az = vb->z[a_idx]; float bz = vb->z[b_idx]; float cz = vb->z[c_idx];

        float ar = vb->r[a_idx]; float br = vb->r[b_idx]; float cr = vb->r[c_idx];
        float ag = vb->g[a_idx]; float bg = vb->g[b_idx]; float cg = vb->g[c_idx];
        float ab = vb->b[a_idx]; float bb = vb->b[b_idx]; float cb = vb->b[c_idx];

        float au = vb->u[a_idx]; float bu = vb->u[b_idx]; float cu = vb->u[c_idx];
        float av = vb->v[a_idx]; float bv = vb->v[b_idx]; float cv = vb->v[c_idx];

        float az_inv = vb->z_inv[a_idx]; float bz_inv = vb->z_inv[b_idx]; float cz_inv = vb->z_inv[c_idx];

        // Precomputed for speed
        float ar_color = ar*az_inv; float br_color = br*bz_inv; float cr_color = cr*cz_inv;
        float ag_color = ag*az_inv; float bg_color = bg*bz_inv; float cg_color = cg*cz_inv;
        float ab_color = ab*az_inv; float bb_color = bb*bz_inv; float cb_color = cb*cz_inv;

        float au_texture = au*az_inv; float bu_texture = bu*bz_inv; float cu_texture = cu*cz_inv;
        float av_texture = av*az_inv; float bv_texture = bv*bz_inv; float cv_texture = cv*cz_inv;

        // Screen coordinates
        float ax_s = (ax * 0.5f + 0.5f) * fb->W;
        float ay_s = (-ay * 0.5f + 0.5f) * fb->H;

        float bx_s = (bx * 0.5f + 0.5f) * fb->W;
        float by_s = (-by * 0.5f + 0.5f) * fb->H;

        float cx_s = (cx * 0.5f + 0.5f) * fb->W;
        float cy_s = (-cy * 0.5f + 0.5f) * fb->H;

        // backface-culling
        float edge1x = bx_s - ax_s;
        float edge1y = by_s - ay_s;
        float edge2x = cx_s - ax_s;
        float edge2y = cy_s - ay_s;
        float cross = edge1x * edge2y - edge1y * edge2x;
        if (cross <= 0) continue; // skip

        // Boundaries REMEMBER: they make up for screen-clipping
        int screen_min_x = MAX(0, (int)((MIN3(ax,bx,cx)*0.5f + 0.5f) * fb->W));
        int screen_max_x = MIN(fb->W-1, (int)((MAX3(ax,bx,cx)*0.5f + 0.5f) * fb->W));
        int screen_min_y = MAX(0, (int)((-MAX3(ay,by,cy)*0.5f + 0.5f) * fb->H));
        int screen_max_y = MIN(fb->H-1, (int)((-MIN3(ay,by,cy)*0.5f + 0.5f) * fb->H));
        // int screen_min_y = MAX(0, (int)((0.5f - MAX3(ay,by,cy)*0.5f) * fb->H));
        // int screen_max_y = MIN(fb->H-1, (int)((0.5f - MIN3(ay,by,cy)*0.5f) * fb->H));  
        
        // printf("\nmin x: %d\tmax x: %d\nmin y: %d\tmax y: %d\n\n", screen_min_x, screen_max_x, screen_min_y, screen_max_y);

        float denom = (by_s - cy_s)*(ax_s - cx_s) + (cx_s - bx_s)*(ay_s - cy_s);
        if (fabs(denom) < 1e-6f) continue; // skip

        // Partial derivatives for α and β
        float dAlpha_dx = (by_s - cy_s) / denom;
        float dAlpha_dy = (cx_s - bx_s) / denom;
        float dBeta_dx  = (cy_s - ay_s) / denom;
        float dBeta_dy  = (ax_s - cx_s) / denom;

        float alpha_row = ((by_s - cy_s)*(screen_min_x - cx_s) + (cx_s - bx_s)*(screen_min_y - cy_s)) / denom;
        float beta_row  = ((cy_s - ay_s)*(screen_min_x - cx_s) + (ax_s - cx_s)*(screen_min_y - cy_s)) / denom;

        // Incrementation
        for (int y = screen_min_y; y <= screen_max_y; y++) {
            float alpha = alpha_row;
            float beta  = beta_row;

            for (int x = screen_min_x; x <= screen_max_x; x++) {
                float gamma = 1.0f - alpha - beta;
                //printf("\n%f, %f, %f", alpha, beta, gamma);

                if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                    int idx = y * fb->W + x;

                    float zp_inv = alpha*az_inv + beta*bz_inv + gamma*cz_inv;
                    float zp = 1.0f / zp_inv;

                    if (zp < fb->z[idx]) {
                        fb->z[idx] = zp;

                        // float r = (alpha*ar_color + beta*br_color + gamma*cr_color) * zp;
                        // float g = (alpha*ag_color + beta*bg_color + gamma*cg_color) * zp;
                        // float b = (alpha*ab_color + beta*bb_color + gamma*cb_color) * zp;

                        // uint32_t r8 = (uint32_t)(r * 255.0f);
                        // uint32_t g8 = (uint32_t)(g * 255.0f);
                        // uint32_t b8 = (uint32_t)(b * 255.0f);

                        // uint32_t interpol_color = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
                        // interpol_color = 0xFFFFFFFF;

                        float u = (alpha*au_texture + beta*bu_texture + gamma*cu_texture) * zp;
                        float v = (alpha*av_texture + beta*bv_texture + gamma*cv_texture) * zp;

                        int tx = (int)(u * (txt->width - 1));
                        int ty = (int)(v * (txt->height - 1));
                        if (tx < 0) tx = 0;
                        if (tx >= txt->width) tx = txt->width - 1;
                        if (ty < 0) ty = 0;
                        if (ty >= txt->height) ty = txt->height - 1;
                        uint32_t texture_color = txt->pixels[ty * txt->width + tx];

                        fb->color[idx] = texture_color;
                    }
                }

                alpha += dAlpha_dx;
                beta  += dBeta_dx;
            }

            alpha_row += dAlpha_dy;
            beta_row  += dBeta_dy;
        }
    }
}