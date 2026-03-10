target triple = "x86_64-w64-windows-gnu"
declare void @perror(i64 %0)
declare i32 @printf(...)
declare i32 @scanf(...)
declare i32 @putchar(i32 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fclose(i64 %0)
declare i32 @puts(i64 %0)
declare i32 @getchar()
declare i32 @fscanf(...)
declare i32 @fflush(i64 %0)
declare i32 @fprintf(...)
define i32 @main() { 
%str = alloca [15 x i8], align 16
%18 = call i32 (ptr, ...) @printf(ptr %str)
%19 = alloca i32
store i32 %18, ptr %19
ret i32 0
}
