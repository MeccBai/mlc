

struct One {
    i32 x;
    char y;
    Two  $ b;
};

struct Two {
    One $ a;
};

i32 main() {
    struct One a;
    i32 x, y;
    i32 p = 10;
    i32 * p = & x;
    i32 xX[10] = {0,1,2,3,4,5};
    i32 a[10] = {0,1,2} ,b[10] = {3,4,5,6},*xp = a;
    if (p == 0) {
        a.x = 10;
    }
    else {
        a.y = 10;
    }

    {
        i32 xaqw = 10;
        i32 ysxa = xaqw;
    }

    switch ( !  a.x) {
        case 1:  a.x = 100;break;
        case 2: a.x = 200;
        default:break;
    }
    return 0;
}