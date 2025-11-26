#ifndef FILE_MAGIK_VEC2D
#define FILE_MAGIK_VEC2D

typedef struct {
    int x;
    int y;
} Vec2d;

Vec2d *Vec2d_new(int, int);
void Vec2d_free(Vec2d *v);

#endif
