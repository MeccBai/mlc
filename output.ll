declare i32 @printf(...)
define i32 @main() { 
%str = alloca [15 x i8], align 16
%1 = getelementptr [15 x i8], ptr %str, i32 0
%2 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 0
store i8 72, ptr %2
%3 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 1
store i8 101, ptr %3
%4 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 2
store i8 108, ptr %4
%5 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 3
store i8 108, ptr %5
%6 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 4
store i8 111, ptr %6
%7 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 5
store i8 44, ptr %7
%8 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 6
store i8 87, ptr %8
%9 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 7
store i8 111, ptr %9
%10 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 8
store i8 114, ptr %10
%11 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 9
store i8 108, ptr %11
%12 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 10
store i8 100, ptr %12
%13 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 11
store i8 33, ptr %13
%14 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 12
store i8 0, ptr %14
%15 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 13
store i8 0, ptr %15
%16 = getelementptr inbounds [15 x i8], ptr %1, i32 0, i32 14
store i8 0, ptr %16
%17 = alloca i32, align 4
%18 = call i32 (ptr, ...) @printf(ptr %str)
store i32 %18, ptr %17, align 4
ret i32 0
 
}