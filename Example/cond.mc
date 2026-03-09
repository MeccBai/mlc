
import std.io;

i32 fab(i32 i) {
    if (i == 1 || i == 2) {
        return 1;
    }
    else {
        return fab(i-1) + fab(i-2);
    }
    return 0;
}

i32 main() {
    i32 result = fab(10);
    i8 str[20] = "The result is: %d";
    printf(str, result);
    return 0;
}