import std.io;
import std.lib;

i8 str[10] = "%d";

void swap(i32$a, i32$b) {
    i32 temp = $a;
    $a = $b;
    $b = temp;
    return;
}

i32 main() {
    i32 c = ~50;
    i8 str[10] = "%d";
    i32 a = 0;
    i32 b = 7;
    i32$ax=i32p(malloc(4));
    $ax = 10;
    printf(str,$ax);
    return 0;
}