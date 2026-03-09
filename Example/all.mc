
import std.io;
enum Season {
    Spr,Sum,Aut,Win
};

i32 main() {
    i32 s_num = 5;
    One o = {0,null };
    if (o.b == null) {
        s_num = i32(106.456);
    }
    i32 su = s_num >> 5 + 8;
    s_num = GetSeason(Season::Aut);
    f32 xa = switch_case(i64(1),null);
    i8 ax[10][100];
    i8 $$x=i8pp(ax);
    f32 xae[3] = {
        f32(0.0),
        switch_case(1,null)
    };
    i32 i = 0;
    i32 xEA = 10 > 20;
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
    i8 hello[20] = "Hello, World!";
    printf(hello);
    return 0;
}

f32 switch_case(i32 a,i64 $b) {
    switch (a) {
        case 1:
            return f32(5.0);
        default:
            return f32(30.0);
    }
    return f32(0.0);
}

struct Two {
    i32 a;
    One$b;
};

struct One {
    i32 a;
    Two$b;
};

i32 max(i32 ix,i32 b) {
    if (ix>b) {
        return ix;
    }
    else {
        return b;
    }
    return 0;
}

i32 GetSeason(Season s) {
    switch (s) {
        case Season::Spr:
            return 1;
        case Season::Sum:
            return 2;
        case Season::Aut:
            return 3;
        case Season::Win:
            return 4;
        default:
            return 0;
    }
    return 0;
}