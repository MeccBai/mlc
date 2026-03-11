target triple = "x86_64-w64-windows-gnu"
%struct.p = type {i32,i32}
declare i32 @putchar(i32 %0)
declare i32 @getchar()
declare i32 @printf(...)
declare i32 @scanf(...)
declare i32 @puts(i64 %0)
declare i32 @fscanf(...)
declare i32 @fflush(i64 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fprintf(...)
declare void @perror(i64 %0)
declare i32 @fclose(i64 %0)
define i32 @hello() { 
%str = alloca [16 x i8], align 16
%il2 = getelementptr [16 x i8], ptr %str, i32 0
%il3 = getelementptr [16 x i8], ptr %il2, i32 0, i32 0
store i8 72, ptr %il3
%il4 = getelementptr [16 x i8], ptr %il2, i32 0, i32 1
store i8 101, ptr %il4
%il5 = getelementptr [16 x i8], ptr %il2, i32 0, i32 2
store i8 108, ptr %il5
%il6 = getelementptr [16 x i8], ptr %il2, i32 0, i32 3
store i8 108, ptr %il6
%il7 = getelementptr [16 x i8], ptr %il2, i32 0, i32 4
store i8 111, ptr %il7
%il8 = getelementptr [16 x i8], ptr %il2, i32 0, i32 5
store i8 44, ptr %il8
%il9 = getelementptr [16 x i8], ptr %il2, i32 0, i32 6
store i8 32, ptr %il9
%il10 = getelementptr [16 x i8], ptr %il2, i32 0, i32 7
store i8 87, ptr %il10
%il11 = getelementptr [16 x i8], ptr %il2, i32 0, i32 8
store i8 111, ptr %il11
%il12 = getelementptr [16 x i8], ptr %il2, i32 0, i32 9
store i8 114, ptr %il12
%il13 = getelementptr [16 x i8], ptr %il2, i32 0, i32 10
store i8 108, ptr %il13
%il14 = getelementptr [16 x i8], ptr %il2, i32 0, i32 11
store i8 100, ptr %il14
%il15 = getelementptr [16 x i8], ptr %il2, i32 0, i32 12
store i8 33, ptr %il15
%il16 = getelementptr [16 x i8], ptr %il2, i32 0, i32 13
store i8 0, ptr %il16
%il17 = getelementptr [16 x i8], ptr %il2, i32 0, i32 14
store i8 0, ptr %il17
%il18 = getelementptr [16 x i8], ptr %il2, i32 0, i32 15
store i8 0, ptr %il18
%19 = call i32 (ptr, ...) @printf(ptr %str)
%20 = alloca i32
store i32 %19, ptr %20
ret i32 0
}
