import std.io;

i32 main() {
    i32 x;
    i8 str[10] = "%d";
    i8 out[20] = "Your number is: %d";
    scanf(str,@x);
    printf(out, x);
    return 0;
}