
f32 switch(i64 a,i64 $b) {
    switch (a) {
        case 1:
            $b = i64(10);
            return 5.0;
        default:
            $b = i64(100);
            return 30.0;
    }
}

struct Two {
    i32 a;
    One$b;
};

struct One {
    i32 a;
    Two$b;
};

i32 main() {
    i8 ax[10][100];
    i8 $$x=i8pp(ax);
    i32 i = 0;
    One o = {0,nullptr };
    Two t = {0,@o};
    o.b->a = 10;
    while(i<10) {
        i32 j = 0;
        while(j<100) {
            ax[i][j] = i8(i+j);
            j=j+1;
        }
        i=i+1;
    }
}


i32 max(i32 a,i32 b) {
    if (a>b) {
        return a;
    }
    else {
        return b;
    }
}