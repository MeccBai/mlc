target triple = "x86_64-w64-windows-gnu"
%struct.b = type {i32,i32}
%struct.a = type {%struct.b,i64}
%struct.d = type {ptr,i32}
%struct.c = type {%struct.a,ptr}
declare i32 @fclose(i64 %0)
declare i32 @putchar(i32 %0)
declare i32 @scanf(...)
declare i32 @puts(i64 %0)
declare i32 @printf(...)
declare i32 @getchar()
declare i32 @fflush(i64 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare void @perror(i64 %0)
declare i32 @fprintf(...)
declare i32 @fscanf(...)
define i32 @main() { 
%b1 = alloca %struct.b, align 8
%il2 = getelementptr %struct.b, ptr %b1, i32 0, i32 0
%il3 = getelementptr %struct.b, ptr %il2, i32 0, i32 0
store i32 10, ptr %il3
%il4 = getelementptr %struct.b, ptr %il2, i32 0, i32 1
store i32 20, ptr %il4
%a1 = alloca %struct.a, align 16
%il6 = getelementptr %struct.a, ptr %a1, i32 0, i32 0
%7 = load %struct.b, ptr %b1, align 4
%il8 = getelementptr %struct.a, ptr %il6, i32 0, i32 0
store %struct.b %7, ptr %il8
%il9 = getelementptr %struct.a, ptr %il6, i32 0, i32 1
store i32 0, ptr %il9
%c1 = alloca %struct.c, align 16
%il11 = getelementptr %struct.c, ptr %c1, i32 0, i32 0
%il12 = getelementptr %struct.c, ptr %il11, i32 0, i32 0
call void @llvm.memcpy.p0.p0.i64(ptr %il12, ptr %a1, i64 16, i1 false)
%il13 = getelementptr %struct.c, ptr %il11, i32 0, i32 1
store ptr null, ptr %il13
%d1 = alloca %struct.d, align 16
%il15 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%il16 = getelementptr %struct.d, ptr %il15, i32 0, i32 0
store ptr %c1, ptr %il16
%il17 = getelementptr %struct.d, ptr %il15, i32 0, i32 1
store i32 100, ptr %il17
%ma18 = getelementptr %struct.a, ptr %a1, i32 0, i32 0
%ma19 = getelementptr %struct.b, ptr %ma18, i32 0, i32 0
%ma20 = load i32, ptr %ma19, align 4
store i32 30, ptr %ma20
%ma21 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%ma23 = load ptr, ptr %ma21, align 8
%ma24 = getelementptr %struct.c, ptr %ma23, i32 0, i32 0
%ma25 = getelementptr %struct.a, ptr %ma24, i32 0, i32 0
%ma26 = getelementptr %struct.b, ptr %ma25, i32 0, i32 0
%ma27 = load i32, ptr %ma26, align 4
store i32 30, ptr %ma27
%x = alloca i32, align 4
%ma29 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%ma31 = load ptr, ptr %ma29, align 8
%ma32 = getelementptr %struct.c, ptr %ma31, i32 0, i32 0
%ma33 = getelementptr %struct.a, ptr %ma32, i32 0, i32 0
%ma34 = getelementptr %struct.b, ptr %ma33, i32 0, i32 0
%ma35 = load i32, ptr %ma34, align 4
%tr36 = add i32 %ma35, 30
store i32 %tr36, ptr %x, align 4
%y = alloca i32, align 4
%tr38 = add i32 10, 20
%tr39 = add i32 %tr38, 30
store i32 %tr39, ptr %y, align 4
%str = alloca [10 x i8], align 16
%il41 = getelementptr [10 x i8], ptr %str, i32 0
%il42 = getelementptr [10 x i8], ptr %il41, i32 0, i32 0
store i8 37, ptr %il42
%il43 = getelementptr [10 x i8], ptr %il41, i32 0, i32 1
store i8 100, ptr %il43
%il44 = getelementptr [10 x i8], ptr %il41, i32 0, i32 2
store i8 0, ptr %il44
%il45 = getelementptr [10 x i8], ptr %il41, i32 0, i32 3
store i8 0, ptr %il45
%il46 = getelementptr [10 x i8], ptr %il41, i32 0, i32 4
store i8 0, ptr %il46
%il47 = getelementptr [10 x i8], ptr %il41, i32 0, i32 5
store i8 0, ptr %il47
%il48 = getelementptr [10 x i8], ptr %il41, i32 0, i32 6
store i8 0, ptr %il48
%il49 = getelementptr [10 x i8], ptr %il41, i32 0, i32 7
store i8 0, ptr %il49
%il50 = getelementptr [10 x i8], ptr %il41, i32 0, i32 8
store i8 0, ptr %il50
%il51 = getelementptr [10 x i8], ptr %il41, i32 0, i32 9
store i8 0, ptr %il51
%52 = load i32, ptr %x, align 4
%53 = load i32, ptr %y, align 4
%tr54 = add i32 %52, %53
%55 = call i32 (ptr, i32, ...) @printf(ptr %str, i32 %tr54)
%56 = alloca i32
store i32 %55, ptr %56
ret i32 0
}
