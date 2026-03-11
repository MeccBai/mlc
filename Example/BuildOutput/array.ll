target triple = "x86_64-w64-windows-gnu"
declare void @perror(i64 %0)
declare i32 @fprintf(...)
declare i32 @getchar()
declare i32 @printf(...)
declare i32 @putchar(i32 %0)
declare i32 @fclose(i64 %0)
declare i32 @fscanf(...)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @puts(i64 %0)
declare i64 @i32pp(...)
declare i64 @i32p(...)
declare i32 @scanf(...)
declare i32 @fflush(i64 %0)
define i32 @mainer() { 
%a = alloca [10 x i32], align 16
%ax = alloca [20 x [10 x i32]], align 16
%bx = alloca ptr, align 8
%ty4 = getelementptr inbounds [20 x [10 x i32]], ptr %ax, i64 0, i64 0
store ptr %ty4, ptr %bx, align 8
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
%ty15 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 0
store ptr %ty15, ptr %b, align 8
%str = alloca [10 x i8], align 16
%il17 = getelementptr [10 x i8], ptr %str, i32 0
%il18 = getelementptr [10 x i8], ptr %il17, i32 0, i32 0
store i8 37, ptr %il18
%il19 = getelementptr [10 x i8], ptr %il17, i32 0, i32 1
store i8 100, ptr %il19
%il20 = getelementptr [10 x i8], ptr %il17, i32 0, i32 2
store i8 10, ptr %il20
%il21 = getelementptr [10 x i8], ptr %il17, i32 0, i32 3
store i8 0, ptr %il21
%il22 = getelementptr [10 x i8], ptr %il17, i32 0, i32 4
store i8 0, ptr %il22
%il23 = getelementptr [10 x i8], ptr %il17, i32 0, i32 5
store i8 0, ptr %il23
%il24 = getelementptr [10 x i8], ptr %il17, i32 0, i32 6
store i8 0, ptr %il24
%il25 = getelementptr [10 x i8], ptr %il17, i32 0, i32 7
store i8 0, ptr %il25
%il26 = getelementptr [10 x i8], ptr %il17, i32 0, i32 8
store i8 0, ptr %il26
%il27 = getelementptr [10 x i8], ptr %il17, i32 0, i32 9
store i8 0, ptr %il27
%r1 = alloca i32, align 4
%tr30 = mul i32 1, 2
%tr31 = add i32 %tr30, 5
%ma29 = getelementptr [10 x i32], ptr %a, i32 0, i32 %tr31
%ma32 = load i32, ptr %ma29, align 4
store i32 %ma32, ptr %r1, align 4
%r2 = alloca i32, align 4
%ma34 = getelementptr [20 x [10 x i32]], ptr %ax, i32 0, i32 10
%ma36 = getelementptr [10 x i32], ptr %a, i32 0, i32 0
%ma37 = load i32, ptr %ma36, align 4
%tr38 = add i32 %ma37, 10
%ma35 = getelementptr [10 x i32], ptr %ma34, i32 0, i32 %tr38
%ma39 = load i32, ptr %ma35, align 4
store i32 %ma39, ptr %r2, align 4
ret i32 0
}
