

struct One {
    int x;
    char y;
    struct Two   * b;
};

struct Two {
    struct One * a;
};

int main() {
    struct One a;
    int x, y;
    int p = 10;
    int * p = & x;
    int xX[10] = {0,1,2,3,4,5};
    int a[10] = {0,1,2} ,b[10] = {3,4,5,6},*xp = a;
    if (p == 0) {
        a.x = 10;
    }
    else {
        a.y = 10;
    }

    {
        int xaqw = 10;
        int ysxa = xaqw;
    }

    switch ( !  a.x) {
        case 1:  a.x = 100;break;
        case 2: a.x = 200;
        default:break;
    }
    return 0;
}