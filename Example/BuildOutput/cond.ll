target triple = "x86_64-w64-windows-gnu"
declare i32 @fflush(i64 %0)
declare i32 @scanf(...)
declare void @perror(i64 %0)
declare i32 @printf(...)
declare i32 @puts(i64 %0)
declare i32 @getchar()
declare i32 @putchar(i32 %0)
declare i32 @fscanf(...)
declare i32 @fclose(i64 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fprintf(...)
define i32 @mainx() { 
%result = alloca i32, align 4
%str = alloca [20 x i8], align 16
%25 = load i32, ptr %result, align 4
%26 = call i32 (ptr, i32, ...) @printf(ptr %str, i32 %25)
%27 = alloca i32
store i32 %26, ptr %27
ret i32 0
}
define i32 @fab(i32 %0) { 
%i = alloca i32, align 4
store i32 %0, ptr %i, align 4
%29 = load i32, ptr %i, align 4
%tr30 = icmp eq i32 %29, 1
%31 = load i32, ptr %i, align 4
%tr32 = icmp eq i32 %31, 2
%tr33 = or i1 %tr30, %tr32
%35 = load i32, ptr %i, align 4
%tr36 = icmp eq i32 %35, 1
%37 = load i32, ptr %i, align 4
%tr38 = icmp eq i32 %37, 2
%tr39 = or i1 %tr36, %tr38
%40 = icmp ne i1 %tr39, 0
br i1 %40, label %.L0, label %.L1
.L0:
ret i32 1
.L1:
%41 = load i32, ptr %i, align 4
%tr42 = sub i32 %41, 1
%fa43 = call i32 @fab(i32 %tr42)
%44 = load i32, ptr %i, align 4
%tr45 = sub i32 %44, 2
%fa46 = call i32 @fab(i32 %tr45)
%tr47 = add i32 %fa43, %fa46
ret i32 %tr47
.L2:
ret i32 0
}
