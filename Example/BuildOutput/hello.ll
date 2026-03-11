target triple = "x86_64-w64-windows-gnu"
%struct.p = type {i32,i32}
declare i32 @printf(...)
declare i32 @puts(i64 %0)
declare i32 @scanf(...)
declare i32 @putchar(i32 %0)
declare i32 @getchar()
declare i64 @fopen(i64 %0,i64 %1)
declare void @perror(i64 %0)
declare i32 @hello()
declare i32 @fclose(i64 %0)
declare i32 @fflush(i64 %0)
declare i32 @fscanf(...)
declare i32 @fprintf(...)
define i32 @main() { 
%p1 = alloca %struct.p, align 8
%str = alloca [20 x i8], align 16
%il23 = getelementptr [20 x i8], ptr %str, i32 0
%il24 = getelementptr [20 x i8], ptr %il23, i32 0, i32 0
store i8 112, ptr %il24
%il25 = getelementptr [20 x i8], ptr %il23, i32 0, i32 1
store i8 46, ptr %il25
%il26 = getelementptr [20 x i8], ptr %il23, i32 0, i32 2
store i8 120, ptr %il26
%il27 = getelementptr [20 x i8], ptr %il23, i32 0, i32 3
store i8 58, ptr %il27
%il28 = getelementptr [20 x i8], ptr %il23, i32 0, i32 4
store i8 37, ptr %il28
%il29 = getelementptr [20 x i8], ptr %il23, i32 0, i32 5
store i8 100, ptr %il29
%il30 = getelementptr [20 x i8], ptr %il23, i32 0, i32 6
store i8 10, ptr %il30
%il31 = getelementptr [20 x i8], ptr %il23, i32 0, i32 7
store i8 112, ptr %il31
%il32 = getelementptr [20 x i8], ptr %il23, i32 0, i32 8
store i8 46, ptr %il32
%il33 = getelementptr [20 x i8], ptr %il23, i32 0, i32 9
store i8 121, ptr %il33
%il34 = getelementptr [20 x i8], ptr %il23, i32 0, i32 10
store i8 58, ptr %il34
%il35 = getelementptr [20 x i8], ptr %il23, i32 0, i32 11
store i8 37, ptr %il35
%il36 = getelementptr [20 x i8], ptr %il23, i32 0, i32 12
store i8 100, ptr %il36
%il37 = getelementptr [20 x i8], ptr %il23, i32 0, i32 13
store i8 0, ptr %il37
%il38 = getelementptr [20 x i8], ptr %il23, i32 0, i32 14
store i8 0, ptr %il38
%il39 = getelementptr [20 x i8], ptr %il23, i32 0, i32 15
store i8 0, ptr %il39
%il40 = getelementptr [20 x i8], ptr %il23, i32 0, i32 16
store i8 0, ptr %il40
%il41 = getelementptr [20 x i8], ptr %il23, i32 0, i32 17
store i8 0, ptr %il41
%il42 = getelementptr [20 x i8], ptr %il23, i32 0, i32 18
store i8 0, ptr %il42
%il43 = getelementptr [20 x i8], ptr %il23, i32 0, i32 19
store i8 0, ptr %il43
%ma44 = getelementptr [20 x i8], ptr %str, i32 0, i32 0
%ma45 = getelementptr [20 x i8], ptr %str, i32 0, i32 0
%ma46 = load i8, ptr %ma45, align 1
%ty47 = trunc i32 1 to i8
%tr48 = add i8 %ma46, %ty47
store i8 %tr48, ptr %ma44
%ma49 = getelementptr %struct.p, ptr %p1, i32 0, i32 0
%ma50 = load i32, ptr %ma49, align 4
%ma51 = getelementptr %struct.p, ptr %p1, i32 0, i32 1
%ma52 = load i32, ptr %ma51, align 4
%53 = call i32 (ptr, i32, i32, ...) @printf(ptr %str, i32 %ma50, i32 %ma52)
%54 = alloca i32
store i32 %53, ptr %54
ret i32 0
}
