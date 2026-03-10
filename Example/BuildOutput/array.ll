target triple = "x86_64-w64-windows-gnu"
declare i32 @puts(i64 %0)
declare i32 @printf(...)
declare i32 @scanf(...)
declare i32 @putchar(i32 %0)
declare i32 @getchar()
declare i32 @fprintf(...)
declare i32 @fclose(i64 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fscanf(...)
declare void @perror(i64 %0)
declare i64 @i32pp(...)
declare i64 @i32p(...)
declare i32 @fflush(i64 %0)
define i32 @mainer() { 
%a = alloca [10 x i32], align 16
%ax = alloca [20 x [10 x i32]], align 16
%bx = alloca ptr, align 8
%ma5 = getelementptr [10 x i32], ptr %a, i32 0, i32 0
store i32 10, ptr %ma5
%ma6 = getelementptr [20 x [10 x i32]], ptr %ax, i32 0, i32 10
%ma8 = getelementptr [10 x i32], ptr %a, i32 0, i32 0
%ma9 = load i32, ptr %ma8, align 4
%tr10 = add i32 %ma9, 10
%ma7 = getelementptr [10 x i32], ptr %ma6, i32 0, i32 %tr10
store i32 10, ptr %ma7
%tr12 = mul i32 1, 2
%tr13 = add i32 %tr12, 5
%ma11 = getelementptr [10 x i32], ptr %a, i32 0, i32 %tr13
store i32 7, ptr %ma11
%b = alloca ptr, align 8
%str = alloca [10 x i8], align 16
%r1 = alloca i32, align 4
%r2 = alloca i32, align 4
ret i32 0
}
