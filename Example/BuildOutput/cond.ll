target triple = "x86_64-w64-windows-gnu"
declare i32 @printf(...)
declare i32 @scanf(...)
declare i32 @puts(i64 %0)
declare i32 @fscanf(...)
declare i32 @fprintf(...)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @putchar(i32 %0)
declare i32 @fflush(i64 %0)
declare i32 @fclose(i64 %0)
declare i32 @getchar()
declare void @perror(i64 %0)
define i32 @fab(i32 %0) { 
%i = alloca i32, align 4
store i32 %0, ptr %i, align 4
%2 = load i32, ptr %i, align 4
%tr3 = icmp eq i32 %2, 1
%4 = load i32, ptr %i, align 4
%tr5 = icmp eq i32 %4, 2
%tr6 = or i1 %tr3, %tr5
%8 = load i32, ptr %i, align 4
%tr9 = icmp eq i32 %8, 1
%10 = load i32, ptr %i, align 4
%tr11 = icmp eq i32 %10, 2
%tr12 = or i1 %tr9, %tr11
%13 = icmp ne i1 %tr12, 0
br i1 %13, label %.L0, label %.L1
.L0:
ret i32 1
.L1:
%14 = load i32, ptr %i, align 4
%tr15 = sub i32 %14, 1
%fa16 = call i32 @fab(i32 %tr15)
%17 = load i32, ptr %i, align 4
%tr18 = sub i32 %17, 2
%fa19 = call i32 @fab(i32 %tr18)
%tr20 = add i32 %fa16, %fa19
ret i32 %tr20
.L2:
ret i32 0
}
define i32 @mainx() { 
%result = alloca i32, align 4
%str = alloca [20 x i8], align 16
%45 = load i32, ptr %result, align 4
%46 = call i32 (ptr, i32, ...) @printf(ptr %str, i32 %45)
%47 = alloca i32
store i32 %46, ptr %47
ret i32 0
}
