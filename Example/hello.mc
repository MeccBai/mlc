import std.io;
import one;

struct o {
    i32 a; i32 b;
};

struct oo {
    o o1;
    o o2;
};

void printOO(oo ax) {
    ax.o1.a = 10;
    ax.o1.b = 20;
    i32 x = ax.o1.a + ax.o1.b;
    return;
}

i32 main() {
    return 0;
}