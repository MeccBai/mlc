

i32 main() {
    f32 xa = switch_case(i64(1),nullptr);
    i8 ax[10][100];
    i8 $$x=i8pp(ax);
    f32 xae[3] = {
        f32(0.0),
        switch_case(i64(1),nullptr)
    };
    i32 i = 0;
    One o = {0,nullptr };
    Two t = {0,@o};
    o.b->a = 10;
    i32 xasd = o.b->a;
    while(i<10) {
        i32 j = 0;
        while(j<100) {
            ax[i][j] = i8(i+j);
            j=j+1;
        }
        i=i+1;
    }
    if (o.a > t.a) {
        return o.a;
    }
    else {
        return max(1,10);
    }
}

f32 switch_case(i64 a,i64 $b) {
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

i32 max(i32 if,i32 b) {
    if (if>b) {
        return if;
    }
    else {
        return b;
    }
}