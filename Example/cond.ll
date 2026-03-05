declare i32 @printf(...)
define i32 @fab(i32 %0) { 
%i = alloca i32, align 4
store i32 %0, ptr %i, align 4
%2 = load i32, ptr %i, align 4
%3 = icmp slt i32 %2, 1
%4 = icmp ne i32 %3, 0
br i1 %4, label %.L0, label %.L1
.L0:
ret i32 1
.L1:
%5 = load i32, ptr %i, align 4
%6 = sub i32 %5, 1
%7 = call i32 @fab(i32 %6)
%8 = load i32, ptr %i, align 4
%9 = sub i32 %8, 2
%10 = call i32 @fab(i32 %9)
%11 = add i32 %7, %10
ret i32 %11
.L2:
ret i32 0
}
define i32 @main() { 
%result = alloca i32, align 4
%13 = call i32 @fab(i32 10)
store i32 %13, ptr %result, align 4
%str = alloca [20 x i8], align 16
%15 = getelementptr [20 x i8], ptr %str, i32 0
%16 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 0
store i8 84, ptr %16
%17 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 1
store i8 104, ptr %17
%18 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 2
store i8 101, ptr %18
%19 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 3
store i8 32, ptr %19
%20 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 4
store i8 114, ptr %20
%21 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 5
store i8 101, ptr %21
%22 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 6
store i8 115, ptr %22
%23 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 7
store i8 117, ptr %23
%24 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 8
store i8 108, ptr %24
%25 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 9
store i8 116, ptr %25
%26 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 10
store i8 32, ptr %26
%27 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 11
store i8 105, ptr %27
%28 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 12
store i8 115, ptr %28
%29 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 13
store i8 58, ptr %29
%30 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 14
store i8 32, ptr %30
%31 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 15
store i8 37, ptr %31
%32 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 16
store i8 100, ptr %32
%33 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 17
store i8 92, ptr %33
%34 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 18
store i8 110, ptr %34
%35 = getelementptr inbounds [20 x i8], ptr %15, i32 0, i32 19
store i8 0, ptr %35
%38 = alloca i32, align 4
%39 = call i32 (ptr, i32, ...) @printf(ptr %36, i32 %37)
store i32 %39, ptr %38, align 4
ret i32 0
}
