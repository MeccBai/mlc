
import std.io;

struct b {
    i32 x;
    i32 y;
};

struct a {
    b b;
    i64 x;
};

struct c {
    a a;
    d$ d;
};

struct d {
    c$ c;
    i32 x;
};

i32 main() {
    b b1 = {10,20};
    a a1 = {b1};
    c c1 = {a1,null};
    d d1 = {@c1,100};
    a1.b.x = 30;
    d1.c->a.b.x = 30;
    i32 x = d1.c->a.b.x + 30;
    i32 y = (10+20) + 30;
    i8 str[10] = "%d";
    printf(str, x+y);
    return 0;
}