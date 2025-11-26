#include "Vec2d.h"
#include "glib.h"

Vec2d *Vec2d_new(int x, int y) {
    Vec2d *v = g_malloc(sizeof(Vec2d));
    v->x = x;
    v->y = y;

    return v;
}

void Vec2d_free(Vec2d *v) {
    free(v);
}
