target triple = "x86_64-w64-windows-gnu"
@a = global i32 100
declare i32 @scanf(...)
declare i32 @putchar(i32 %0)
declare i32 @printf(...)
declare void @perror(i64 %0)
declare i32 @puts(i64 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare i64 @i8p(...)
declare i32 @fscanf(...)
declare i32 @getchar()
declare i32 @fflush(i64 %0)
declare i32 @fclose(i64 %0)
declare i32 @fprintf(...)
define i32 @main() { 
%x = alloca i32, align 4
%err = alloca [20 x i8], align 16
%il3 = getelementptr [20 x i8], ptr %err, i32 0
%il4 = getelementptr [20 x i8], ptr %il3, i32 0, i32 0
store i8 65, ptr %il4
%il5 = getelementptr [20 x i8], ptr %il3, i32 0, i32 1
store i8 110, ptr %il5
%il6 = getelementptr [20 x i8], ptr %il3, i32 0, i32 2
store i8 32, ptr %il6
%il7 = getelementptr [20 x i8], ptr %il3, i32 0, i32 3
store i8 101, ptr %il7
%il8 = getelementptr [20 x i8], ptr %il3, i32 0, i32 4
store i8 114, ptr %il8
%il9 = getelementptr [20 x i8], ptr %il3, i32 0, i32 5
store i8 114, ptr %il9
%il10 = getelementptr [20 x i8], ptr %il3, i32 0, i32 6
store i8 111, ptr %il10
%il11 = getelementptr [20 x i8], ptr %il3, i32 0, i32 7
store i8 114, ptr %il11
%il12 = getelementptr [20 x i8], ptr %il3, i32 0, i32 8
store i8 32, ptr %il12
%il13 = getelementptr [20 x i8], ptr %il3, i32 0, i32 9
store i8 111, ptr %il13
%il14 = getelementptr [20 x i8], ptr %il3, i32 0, i32 10
store i8 99, ptr %il14
%il15 = getelementptr [20 x i8], ptr %il3, i32 0, i32 11
store i8 99, ptr %il15
%il16 = getelementptr [20 x i8], ptr %il3, i32 0, i32 12
store i8 117, ptr %il16
%il17 = getelementptr [20 x i8], ptr %il3, i32 0, i32 13
store i8 114, ptr %il17
%il18 = getelementptr [20 x i8], ptr %il3, i32 0, i32 14
store i8 114, ptr %il18
%il19 = getelementptr [20 x i8], ptr %il3, i32 0, i32 15
store i8 101, ptr %il19
%il20 = getelementptr [20 x i8], ptr %il3, i32 0, i32 16
store i8 100, ptr %il20
%il21 = getelementptr [20 x i8], ptr %il3, i32 0, i32 17
store i8 33, ptr %il21
%il22 = getelementptr [20 x i8], ptr %il3, i32 0, i32 18
store i8 0, ptr %il22
%il23 = getelementptr [20 x i8], ptr %il3, i32 0, i32 19
store i8 0, ptr %il23
%err2 = alloca ptr, align 8
%ty25 = getelementptr inbounds [20 x i8], ptr %err, i64 0, i64 0
store ptr %ty25, ptr %err2, align 8
%26 = load ptr, ptr %err2, align 4
call void @perror(ptr %26)
ret i32 0
}
