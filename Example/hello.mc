import std.io;

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
    swap(@a,@b);


    printf(str,a);
    return 0;
}