/*
    Tokyo3D\main.c
*/

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "tokyo3d.h"


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define WIDTH  800
#define HEIGHT 600

Texture load_texture(const char* path) {
    Texture txt = {0};

    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        SDL_Log("Failed to load BMP: %s", SDL_GetError());
        return txt;
    }

    // Convert to 32-bit RGBA8888
    SDL_Surface* surf32 = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
    SDL_DestroySurface(surface); // free the original

    if (!surf32) {
        SDL_Log("Failed to convert BMP: %s", SDL_GetError());
        return txt;
    }

    txt.width  = surf32->w;
    txt.height = surf32->h;
    txt.pixels = malloc(sizeof(uint32_t) * txt.width * txt.height);
    if (!txt.pixels) {
        SDL_Log("Failed to allocate texture memory");
        SDL_DestroySurface(surf32);
        return txt;
    }

    memcpy(txt.pixels, surf32->pixels, sizeof(uint32_t) * txt.width * txt.height);

    SDL_DestroySurface(surf32);
    return txt;
}

void free_texture(Texture* txt) {
    if (txt->pixels) free(txt->pixels);
    txt->pixels = NULL;
}


int main(int argc, char *argv[]) {
    // Initialize SDL video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Tokyo3D Render",
                                          WIDTH, HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create a streaming texture (RGBA8888 format)
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             WIDTH, HEIGHT);
    if (!texture) {
        SDL_Log("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Tokyo3D code
    VertexBuffer starter, transformed, z_clipped, projected, screen_clipped;
    FrameBuffer fb;

    Space world = {
      (Vec3){0.0f, 0.0f, -1.0f},
      (Vec3){0.0f, 1.0f, 0.0f},
      (Vec3){1.0f, 0.0f, 0.0f},
    };

    float FOV_deg = 70;
    float focal = 1/tan( FOV_deg * (PI / 180.0f) /2);
    Camera cam = make_camera((Vec3){0.0f, 0.0f, 0.0f}, (Vec3){0.0f, 0.0f, 1.0f}, focal, world);

    VB_alloc(&starter);
    VB_alloc(&transformed);
    VB_alloc(&z_clipped);
    VB_alloc(&projected);
    VB_alloc(&screen_clipped);

    FB_alloc(&fb, WIDTH, HEIGHT);

    Texture t = load_texture("assets/cat.bmp"); // loading texture

    starter.len = 9; // 3 triangles = 9 vertex
    // T1
    starter.x[0] = -1.0f; starter.y[0] = -1.0f; starter.z[0] = 5.0f;
    starter.x[1] =  1.0f; starter.y[1] = -1.0f; starter.z[1] = 5.0f;
    starter.x[2] =  0.0f; starter.y[2] =  1.0f; starter.z[2] = 5.0f;

    starter.r[0]=1; starter.g[0]=0; starter.b[0]=0;
    starter.r[1]=0; starter.g[1]=1; starter.b[1]=0;
    starter.r[2]=0; starter.g[2]=0; starter.b[2]=1;
    // T2
    starter.x[3] = -0.7f; starter.y[3] = -0.7f; starter.z[3] = 4.0f;
    starter.x[4] =  0.7f; starter.y[4] = -0.7f; starter.z[4] = 4.0f;
    starter.x[5] =  0.0f; starter.y[5] =  0.7f; starter.z[5] = 4.0f;

    starter.r[3]=1; starter.g[3]=1; starter.b[3]=0; // yellow
    starter.r[4]=1; starter.g[4]=1; starter.b[4]=0;
    starter.r[5]=1; starter.g[5]=0; starter.b[5]=1; // magenta top
    // T3
    starter.x[6] = -0.5f; starter.y[6] =  0.0f; starter.z[6] = 3.0f; // near
    starter.x[7] =  0.5f; starter.y[7] =  0.0f; starter.z[7] = 6.0f; // far
    starter.x[8] =  0.0f; starter.y[8] = -0.8f; starter.z[8] = 4.5f; // middle

    starter.r[6]=0; starter.g[6]=1; starter.b[6]=1; // cyan
    starter.r[7]=1; starter.g[7]=0; starter.b[7]=0; // red
    starter.r[8]=0.2; starter.g[8]=0.3; starter.b[8]=0.3; // cool blue

    // Textures
    starter.u[0] = 0.0f; starter.v[0] = 0.0f;
    starter.u[1] = 1.0f; starter.v[1] = 0.0f;
    starter.u[2] = 0.5f; starter.v[2] = 1.0f;

    starter.u[3] = 0.0f; starter.v[3] = 0.0f;
    starter.u[4] = 1.0f; starter.v[4] = 0.0f;
    starter.u[5] = 0.5f; starter.v[5] = 1.0f;

    starter.u[6] = 0.0f; starter.v[6] = 0.0f;
    starter.u[7] = 1.0f; starter.v[7] = 0.0f;
    starter.u[8] = 0.5f; starter.v[8] = 1.0f;

    int running = 1;
    SDL_Event event;

    Uint64 last = SDL_GetPerformanceCounter(); // used for DT

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }

        // Lock texture to get pixel buffer
        void *pixels;
        int pitch;
        if (!SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
            SDL_Log("SDL_LockTexture Error: %s", SDL_GetError());
            break;
        }

        // Movements
        float speed = 2.0f;
        float angle_speed = 2.0f;
        // Getting DT
        Uint64 now = SDL_GetPerformanceCounter();
        Uint64 freq = SDL_GetPerformanceFrequency();

        float dt = (float)(now - last) / (float)freq;
        last = now;

        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W]) {
            // forward
            CAMERA_translate(&cam, scal(cam.space.F, speed*dt));
        }

        if (state[SDL_SCANCODE_S]) {
            CAMERA_translate(&cam, scal(cam.space.F, -speed*dt));
        }

        if (state[SDL_SCANCODE_A]) {
            // sx
            CAMERA_rotate(&cam, world.U, -angle_speed*dt);
        }

        if (state[SDL_SCANCODE_D]) {
            // dx
            CAMERA_rotate(&cam, world.U, angle_speed*dt);
        }

        if (state[SDL_SCANCODE_SPACE]) {
            // jump
            CAMERA_translate(&cam, scal(cam.space.U, speed*dt));
        }

        if (state[SDL_SCANCODE_LSHIFT]) {
            // shift sx
            CAMERA_translate(&cam, scal(cam.space.U, -speed*dt));
        }

        if (state[SDL_SCANCODE_ESCAPE]) {
            // quit
            running = false;
        }

        VB_transform(&starter, &transformed, &cam);
        VB_Z_clipping(&transformed, &z_clipped);
        VB_project(&z_clipped, &projected, &cam);
        // VB_screen_clipping(&projected, &screen_clipped);
        rasterization(&fb, &projected, &t);

        // printf("\nTriangles: %u\n", full_clipped.len / 3);
        // for (int i=0; i<full_clipped.len; i++)
        //   printf("\nVertexes: %f, %f, %f", full_clipped.x[i], full_clipped.y[i], full_clipped.z[i]);

        // Fill framebuffer with a gradient
        for (int y = 0; y < HEIGHT; y++) {
            uint32_t* dst = (uint32_t*)((uint8_t*)pixels + y * pitch);
            uint32_t* src = fb.color + y * WIDTH;
            memcpy(dst, src, WIDTH * sizeof(uint32_t));
        }

        SDL_UnlockTexture(texture);

        // DEBUG: frame printer
        static float accumulator = 0.0f;
        static int frames = 0;

        accumulator += dt;
        frames++;

        if (accumulator >= 1.0f) {
            float avg_ms = (accumulator / frames) * 1000.0f;
            printf("Avg frame time: %.3f ms (%d FPS)\n", avg_ms, frames);

            accumulator = 0.0f;
            frames = 0;
        }

        // Render the texture
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // Tokyo3D Cleanup
    free_texture(&t);
    VB_free(&starter);
    VB_free(&transformed);
    VB_free(&z_clipped);
    VB_free(&projected);
    VB_free(&screen_clipped);
    FB_free(&fb);


    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}