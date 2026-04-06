# Tokyo3D
by 0xBB403
A small 3D software renderer written in C. Hobby project demonstrating a software pipeline for transforming, clipping, projecting, and rasterizing 3D geometry. Can render to a framebuffer for display in SDL3 or other libraries.
Using SDL3 to display the returned framebuffer.

Information:
- SoA and Vertex centered
- pipeline: transform -> clipping(Z -> projection -> screen) -> interpolation
- RH system, using Euler angles
- STRUCT-NAME_function(...) means it modify the struct that is passed
- VB_clipping works on (-1, 1, z) to (1, -1, -z) VertexData like OpenGL
- Supports textures

Compilation with gcc:
gcc main.c src/*.c -Ilib -Isdl/include -Lsdl/lib -lSDL3 -lm -o app
