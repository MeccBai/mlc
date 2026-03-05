%struct.b = type {i32,i32}
%struct.a = type {%struct.b,i64}
%struct.c = type {%struct.a,ptr}
%struct.d = type {ptr,i32}
define i32 @main() { 
%b1 = alloca %struct.b, align 8
%2 = getelementptr %struct.b, ptr %b1, i32 0, i32 0
%3 = getelementptr inbounds %struct.b, ptr %2, i32 0, i32 0
store i32 10, ptr %3
%4 = getelementptr inbounds %struct.b, ptr %2, i32 0, i32 1
store i32 20, ptr %4
%a1 = alloca %struct.a, align 16
%6 = getelementptr %struct.a, ptr %a1, i32 0, i32 0
%7 = load %struct.b, ptr %b1, align 4
%8 = getelementptr inbounds %struct.a, ptr %6, i32 0, i32 0
store %struct.b %7, ptr %8
%9 = call i64 @i64(i32 100)
%10 = getelementptr inbounds %struct.a, ptr %6, i32 0, i32 1
store i64 %9, ptr %10
%c1 = alloca %struct.c, align 16
%12 = getelementptr %struct.c, ptr %c1, i32 0, i32 0
%13 = load %struct.a, ptr %a1, align 4
%14 = getelementptr inbounds %struct.c, ptr %12, i32 0, i32 0
store %struct.a %13, ptr %14
%15 = getelementptr inbounds %struct.c, ptr %12, i32 0, i32 1
store ptr null, ptr %15
%d1 = alloca %struct.d, align 16
%17 = getelementptr %struct.d, ptr %d1, i32 0, i32 0
%18 = getelementptr inbounds %struct.d, ptr %17, i32 0, i32 0
store %struct.c %c1, ptr %18
%19 = getelementptr inbounds %struct.d, ptr %17, i32 0, i32 1
store i32 100, ptr %19
%20 = load %struct.d, ptr %d1, align 4
%21 = getelementptr %struct.d, ptr %20, i32 0, i32 0
%22 = load %struct.c, ptr %21, align 4
%23 = getelementptr %struct.c, ptr %22, i32 0, i32 0
%24 = getelementptr %struct.a, ptr %23, i32 0, i32 0
%25 = getelementptr %struct.b, ptr %24, i32 0, i32 0
store i32 30, ptr %25
%x = alloca i32, align 4
%27 = load %struct.d, ptr %d1, align 4
%28 = getelementptr %struct.d, ptr %27, i32 0, i32 0
%29 = load %struct.c, ptr %28, align 4
%30 = getelementptr %struct.c, ptr %29, i32 0, i32 0
%31 = getelementptr %struct.a, ptr %30, i32 0, i32 0
%32 = getelementptr %struct.b, ptr %31, i32 0, i32 0
%33 = add i32 %32, 30
store i32 %33, ptr %x, align 4
%y = alloca i32, align 4
%35 = add i32 10, 20
%36 = add i32 %35, 30
store i32 %36, ptr %y, align 4
ret i32 0
}
