

struct One {
    int x;
    char y;
};


int main() {
    struct One a;
    int x, y;
    int p = 10;
    if (p == 0) {
        a.x = 10;
    }
    else {
        a.y = 10;
    }
    switch ( !  a.x) {
        case 1:  a.x = 100;break;
        default:break;
    }
    return 0;
}