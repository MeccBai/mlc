
import std.io;

i32 main() {
    i32 a[10];

    i32 ax[10][20];
    i32$$ bx = i32pp(ax);
    a[0] = 10;
    ax[10][a[0]+10] = 10;

    a[1*2+5]=7;
    i32$b=i32p(a);

    i8 str[10] = "%d\n";
    i32 r1 = a[1*2+5];
    i32 r2 = ax[10][a[0]+10];
    printf(str, a[1*2+5]+ax[10][a[0]+10]);

    return 0;
}