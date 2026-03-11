target triple = "x86_64-w64-windows-gnu"
%struct.Two = type {i32,ptr}
%struct.One = type {i32,ptr}
declare i32 @printf(...)
declare i32 @putchar(i32 %0)
declare i32 @puts(i64 %0)
declare i32 @scanf(...)
declare i32 @getchar()
declare i32 @fflush(i64 %0)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fclose(i64 %0)
declare i32 @fscanf(...)
declare i64 @i8pp(...)
declare i32 @fprintf(...)
declare void @perror(i64 %0)
define i32 @main() { 
%s_num = alloca i32, align 4
store i32 5, ptr %s_num, align 4
%o = alloca %struct.One, align 16
%il3 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%il4 = getelementptr %struct.One, ptr %il3, i32 0, i32 0
store i32 0, ptr %il4
%il5 = getelementptr %struct.One, ptr %il3, i32 0, i32 1
store ptr null, ptr %il5
%ma6 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%tr7 = icmp eq ptr %ma6, null
%ma9 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%tr10 = icmp eq ptr %ma9, null
%11 = icmp ne i1 %tr10, 0
br i1 %11, label %.L0, label %.L2
.L0:
%ty12 = fptosi double 106.456 to i32
store i32 %ty12, ptr %s_num
br label %.L2
.L2:
%su = alloca i32, align 4
%14 = load i32, ptr %s_num, align 4
%tr15 = add i32 5, 8
%tr16 = ashr i32 %14, %tr15
store i32 %tr16, ptr %su, align 4
%fa17 = call i32 @GetSeason(i32 2)
store i32 %fa17, ptr %s_num
%xa = alloca float, align 4
%ty19 = sext i32 1 to i64
%fa20 = call float @switch_case(i64 %ty19, ptr null)
store float %fa20, ptr %xa, align 4
%ax = alloca [100 x [10 x i8]], align 16
%x = alloca ptr, align 8
%ty23 = getelementptr inbounds [100 x [10 x i8]], ptr %ax, i64 0, i64 0
store ptr %ty23, ptr %x, align 8
%xae = alloca [3 x float], align 16
%il25 = getelementptr [3 x float], ptr %xae, i32 0
%ty26 = fptrunc double 0.0 to float
%il27 = getelementptr [3 x float], ptr %il25, i32 0, i32 0
store float %ty26, ptr %il27
%fa28 = call float @switch_case(i32 1, ptr null)
%il29 = getelementptr [3 x float], ptr %il25, i32 0, i32 1
store float %fa28, ptr %il29
%il30 = getelementptr [3 x float], ptr %il25, i32 0, i32 2
store double 0.0, ptr %il30
%i = alloca i32, align 4
store i32 0, ptr %i, align 4
%xEA = alloca i32, align 4
%tr33 = icmp sgt i32 10, 20
store i1 %tr33, ptr %xEA, align 4
%t = alloca %struct.Two, align 16
%il35 = getelementptr %struct.Two, ptr %t, i32 0, i32 0
%il36 = getelementptr %struct.Two, ptr %il35, i32 0, i32 0
store i32 0, ptr %il36
%il37 = getelementptr %struct.Two, ptr %il35, i32 0, i32 1
store ptr %o, ptr %il37
%ma38 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%ma40 = load ptr, ptr %ma38, align 8
%ma41 = getelementptr %struct.Two, ptr %ma40, i32 0, i32 0
store i32 10, ptr %ma41
%xasd = alloca i32, align 4
%ma43 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%ma45 = load ptr, ptr %ma43, align 8
%ma46 = getelementptr %struct.Two, ptr %ma45, i32 0, i32 0
%ma47 = load i32, ptr %ma46, align 4
store i32 %ma47, ptr %xasd, align 4
br label %.L3
.L3:
%48 = load i32, ptr %i, align 4
%tr49 = icmp slt i32 %48, 10
%50 = load i32, ptr %i, align 4
%tr51 = icmp slt i32 %50, 10
%52 = icmp ne i1 %tr51, 0
br i1 %52, label %.L4, label %.L5
.L4:
%j = alloca i32, align 4
store i32 0, ptr %j, align 4
br label %.L6
.L6:
%54 = load i32, ptr %j, align 4
%tr55 = icmp slt i32 %54, 100
%56 = load i32, ptr %j, align 4
%tr57 = icmp slt i32 %56, 100
%58 = icmp ne i1 %tr57, 0
br i1 %58, label %.L7, label %.L8
.L7:
%60 = load i32, ptr %i, align 4
%ma59 = getelementptr [100 x [10 x i8]], ptr %ax, i32 0, i32 %60
%62 = load i32, ptr %j, align 4
%ma61 = getelementptr [10 x i8], ptr %ma59, i32 0, i32 %62
%63 = load i32, ptr %i, align 4
%64 = load i32, ptr %j, align 4
%tr65 = add i32 %63, %64
%ty66 = trunc i32 %tr65 to i8
store i8 %ty66, ptr %ma61
%67 = load i32, ptr %j, align 4
%tr68 = add i32 %67, 1
store i32 %tr68, ptr %j
br label %.L6
.L8:
%69 = load i32, ptr %i, align 4
%tr70 = add i32 %69, 1
store i32 %tr70, ptr %i
br label %.L3
.L5:
%ma71 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%ma72 = load i32, ptr %ma71, align 4
%ma73 = getelementptr %struct.Two, ptr %t, i32 0, i32 0
%ma74 = load i32, ptr %ma73, align 4
%tr75 = icmp sgt i32 %ma72, %ma74
%ma77 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%ma78 = load i32, ptr %ma77, align 4
%ma79 = getelementptr %struct.Two, ptr %t, i32 0, i32 0
%ma80 = load i32, ptr %ma79, align 4
%tr81 = icmp sgt i32 %ma78, %ma80
%82 = icmp ne i1 %tr81, 0
br i1 %82, label %.L9, label %.L10
.L9:
%ma83 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%ma84 = load i32, ptr %ma83, align 4
ret i32 %ma84
.L10:
%fa85 = call i32 @max(i32 1, i32 10)
ret i32 %fa85
.L11:
%hello = alloca [20 x i8], align 16
%il87 = getelementptr [20 x i8], ptr %hello, i32 0
%il88 = getelementptr [20 x i8], ptr %il87, i32 0, i32 0
store i8 72, ptr %il88
%il89 = getelementptr [20 x i8], ptr %il87, i32 0, i32 1
store i8 101, ptr %il89
%il90 = getelementptr [20 x i8], ptr %il87, i32 0, i32 2
store i8 108, ptr %il90
%il91 = getelementptr [20 x i8], ptr %il87, i32 0, i32 3
store i8 108, ptr %il91
%il92 = getelementptr [20 x i8], ptr %il87, i32 0, i32 4
store i8 111, ptr %il92
%il93 = getelementptr [20 x i8], ptr %il87, i32 0, i32 5
store i8 44, ptr %il93
%il94 = getelementptr [20 x i8], ptr %il87, i32 0, i32 6
store i8 32, ptr %il94
%il95 = getelementptr [20 x i8], ptr %il87, i32 0, i32 7
store i8 87, ptr %il95
%il96 = getelementptr [20 x i8], ptr %il87, i32 0, i32 8
store i8 111, ptr %il96
%il97 = getelementptr [20 x i8], ptr %il87, i32 0, i32 9
store i8 114, ptr %il97
%il98 = getelementptr [20 x i8], ptr %il87, i32 0, i32 10
store i8 108, ptr %il98
%il99 = getelementptr [20 x i8], ptr %il87, i32 0, i32 11
store i8 100, ptr %il99
%il100 = getelementptr [20 x i8], ptr %il87, i32 0, i32 12
store i8 33, ptr %il100
%il101 = getelementptr [20 x i8], ptr %il87, i32 0, i32 13
store i8 0, ptr %il101
%il102 = getelementptr [20 x i8], ptr %il87, i32 0, i32 14
store i8 0, ptr %il102
%il103 = getelementptr [20 x i8], ptr %il87, i32 0, i32 15
store i8 0, ptr %il103
%il104 = getelementptr [20 x i8], ptr %il87, i32 0, i32 16
store i8 0, ptr %il104
%il105 = getelementptr [20 x i8], ptr %il87, i32 0, i32 17
store i8 0, ptr %il105
%il106 = getelementptr [20 x i8], ptr %il87, i32 0, i32 18
store i8 0, ptr %il106
%il107 = getelementptr [20 x i8], ptr %il87, i32 0, i32 19
store i8 0, ptr %il107
%108 = call i32 (ptr, ...) @printf(ptr %hello)
%109 = alloca i32
store i32 %108, ptr %109
ret i32 0
}
define i32 @max(i32 %0,i32 %1) { 
%ix = alloca i32, align 4
store i32 %0, ptr %ix, align 4
%b = alloca i32, align 4
store i32 %1, ptr %b, align 4
%112 = load i32, ptr %ix, align 4
%113 = load i32, ptr %b, align 4
%tr114 = icmp sgt i32 %112, %113
%116 = load i32, ptr %ix, align 4
%117 = load i32, ptr %b, align 4
%tr118 = icmp sgt i32 %116, %117
%119 = icmp ne i1 %tr118, 0
br i1 %119, label %.L12, label %.L13
.L12:
%120 = load i32, ptr %ix, align 4
ret i32 %120
.L13:
%121 = load i32, ptr %b, align 4
ret i32 %121
.L14:
ret i32 0
}
define i32 @GetSeason(i32 %0) { 
%s = alloca i32, align 4
store i32 %0, ptr %s, align 4
%123 = load i32, ptr %s, align 4
%124 = icmp eq i32 %123, 0
br i1 %124, label %.L16, label %.L21
.L21:
%125 = icmp eq i32 %123, 1
br i1 %125, label %.L17, label %.L22
.L22:
%126 = icmp eq i32 %123, 2
br i1 %126, label %.L18, label %.L23
.L23:
%127 = icmp eq i32 %123, 3
br i1 %127, label %.L19, label %.L24
.L24:
br label %.L20

.L16:
ret i32 1

.L17:
ret i32 2

.L18:
ret i32 3

.L19:
ret i32 4

.L20:
ret i32 0


.L15:
ret i32 0
}
define float @switch_case(i32 %0,i64 %1) { 
%a = alloca i32, align 4
store i32 %0, ptr %a, align 4
%b = alloca i64, align 8
store i64 %1, ptr %b, align 8
%130 = load i32, ptr %a, align 4
%133 = icmp eq i32 %130, 1
br i1 %133, label %.L26, label %.L28
.L28:
br label %.L27

.L26:
%ty131 = fptrunc double 5.0 to float
ret float %ty131

.L27:
%ty132 = fptrunc double 30.0 to float
ret float %ty132


.L25:
%ty134 = fptrunc double 0.0 to float
ret float %ty134
}
