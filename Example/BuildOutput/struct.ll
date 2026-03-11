target triple = "x86_64-w64-windows-gnu"
%struct.b = type {i32,i32}
%struct.a = type {%struct.b,i64}
%struct.d = type {ptr,i32}
%struct.c = type {%struct.a,ptr}
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @printf(...)
declare i32 @getchar()
declare void @perror(i64 %0)
declare i32 @fclose(i64 %0)
declare i32 @scanf(...)
declare i32 @puts(i64 %0)
declare i32 @putchar(i32 %0)
declare i32 @fflush(i64 %0)
declare i32 @fscanf(...)
declare i32 @fprintf(...)
define i32 @main() { 
%b1 = alloca %struct.b, align 8
%il2 = getelementptr %struct.b, ptr %b1, i32 0, i32 0
%il3 = getelementptr %struct.b, ptr %il2, i32 0, i32 0
store i32 10, ptr %il3
%il4 = getelementptr %struct.b, ptr %il2, i32 0, i32 1
store i32 20, ptr %il4
%a1 = alloca %struct.a, align 16
%il6 = getelementptr %struct.a, ptr %a1, i32 0, i32 0
%il7 = getelementptr %struct.a, ptr %il6, i32 0, i32 0
call void @llvm.memcpy.p0.p0.i64(ptr %il7, ptr %b1, i64 8, i1 false)
%il8 = getelementptr %struct.a, ptr %il6, i32 0, i32 1
store i32 0, ptr %il8
%c1 = alloca %struct.c, align 16
%il10 = getelementptr %struct.c, ptr %c1, i32 0, i32 0
%il11 = getelementptr %struct.c, ptr %il10, i32 0, i32 0
call void @llvm.memcpy.p0.p0.i64(ptr %il11, ptr %a1, i64 16, i1 false)
%il12 = getelementptr %struct.c, ptr %il10, i32 0, i32 1
store ptr null, ptr %il12
%d1 = alloca %struct.d, align 16
%il14 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%il15 = getelementptr %struct.d, ptr %il14, i32 0, i32 0
store ptr %c1, ptr %il15
%il16 = getelementptr %struct.d, ptr %il14, i32 0, i32 1
store i32 100, ptr %il16
%ma17 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%ma19 = load ptr, ptr %ma17, align 8
%ma20 = getelementptr %struct.c, ptr %ma19, i32 0, i32 0
%ma21 = getelementptr %struct.a, ptr %ma20, i32 0, i32 0
%ma22 = getelementptr %struct.b, ptr %ma21, i32 0, i32 0
store i32 30, ptr %ma22
%x = alloca i32, align 4
%ma24 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%ma26 = load ptr, ptr %ma24, align 8
%ma27 = getelementptr %struct.c, ptr %ma26, i32 0, i32 0
%ma28 = getelementptr %struct.a, ptr %ma27, i32 0, i32 0
%ma29 = getelementptr %struct.b, ptr %ma28, i32 0, i32 0
%ma30 = load i32, ptr %ma29, align 4
%tr31 = add i32 %ma30, 30
store i32 %tr31, ptr %x, align 4
%y = alloca i32, align 4
%tr33 = add i32 10, 20
%tr34 = add i32 %tr33, 30
store i32 %tr34, ptr %y, align 4
%str = alloca [10 x i8], align 16
%il36 = getelementptr [10 x i8], ptr %str, i32 0
%il37 = getelementptr [10 x i8], ptr %il36, i32 0, i32 0
store i8 37, ptr %il37
%il38 = getelementptr [10 x i8], ptr %il36, i32 0, i32 1
store i8 100, ptr %il38
%il39 = getelementptr [10 x i8], ptr %il36, i32 0, i32 2
store i8 0, ptr %il39
%il40 = getelementptr [10 x i8], ptr %il36, i32 0, i32 3
store i8 0, ptr %il40
%il41 = getelementptr [10 x i8], ptr %il36, i32 0, i32 4
store i8 0, ptr %il41
%il42 = getelementptr [10 x i8], ptr %il36, i32 0, i32 5
store i8 0, ptr %il42
%il43 = getelementptr [10 x i8], ptr %il36, i32 0, i32 6
store i8 0, ptr %il43
%il44 = getelementptr [10 x i8], ptr %il36, i32 0, i32 7
store i8 0, ptr %il44
%il45 = getelementptr [10 x i8], ptr %il36, i32 0, i32 8
store i8 0, ptr %il45
%il46 = getelementptr [10 x i8], ptr %il36, i32 0, i32 9
store i8 0, ptr %il46
%47 = load i32, ptr %x, align 4
%48 = load i32, ptr %y, align 4
%tr49 = add i32 %47, %48
%50 = call i32 (ptr, i32, ...) @printf(ptr %str, i32 %tr49)
%51 = alloca i32
store i32 %50, ptr %51
ret i32 0
}
