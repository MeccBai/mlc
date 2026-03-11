target triple = "x86_64-w64-windows-gnu"
%struct.One = type {i32,ptr}
%struct.Two = type {i32,ptr}
declare i32 @printf(...)
declare i32 @scanf(...)
declare i32 @puts(i64 %0)
declare i32 @putchar(i32 %0)
declare i32 @getchar()
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fscanf(...)
declare i32 @fclose(i64 %0)
declare i64 @i8pp(...)
declare void @perror(i64 %0)
declare i32 @fprintf(...)
declare i32 @fflush(i64 %0)
define i32 @GetSeason(i32 %0) { 
%s = alloca i32, align 4
store i32 %0, ptr %s, align 4
%2 = load i32, ptr %s, align 4
%3 = icmp eq i32 %2, 0
br i1 %3, label %.L1, label %.L6
.L6:
%4 = icmp eq i32 %2, 1
br i1 %4, label %.L2, label %.L7
.L7:
%5 = icmp eq i32 %2, 2
br i1 %5, label %.L3, label %.L8
.L8:
%6 = icmp eq i32 %2, 3
br i1 %6, label %.L4, label %.L9
.L9:
br label %.L5

.L1:
ret i32 1

.L2:
ret i32 2

.L3:
ret i32 3

.L4:
ret i32 4

.L5:
ret i32 0


.L0:
ret i32 0
}
define float @switch_case(i32 %0,i64 %1) { 
%a = alloca i32, align 4
store i32 %0, ptr %a, align 4
%b = alloca i64, align 8
store i64 %1, ptr %b, align 8
%9 = load i32, ptr %a, align 4
%12 = icmp eq i32 %9, 1
br i1 %12, label %.L11, label %.L13
.L13:
br label %.L12

.L11:
%ty10 = fptrunc double 5.0 to float
ret float %ty10

.L12:
%ty11 = fptrunc double 30.0 to float
ret float %ty11


.L10:
%ty13 = fptrunc double 0.0 to float
ret float %ty13
}
define i32 @main() { 
%s_num = alloca i32, align 4
store i32 5, ptr %s_num, align 4
%o = alloca %struct.One, align 16
%il16 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%il17 = getelementptr %struct.One, ptr %il16, i32 0, i32 0
store i32 0, ptr %il17
%il18 = getelementptr %struct.One, ptr %il16, i32 0, i32 1
store ptr null, ptr %il18
%ma19 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%tr20 = icmp eq ptr %ma19, null
%ma22 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%tr23 = icmp eq ptr %ma22, null
%24 = icmp ne i1 %tr23, 0
br i1 %24, label %.L14, label %.L16
.L14:
%ty25 = fptosi double 106.456 to i32
store i32 %ty25, ptr %s_num
br label %.L16
.L16:
%su = alloca i32, align 4
%27 = load i32, ptr %s_num, align 4
%tr28 = add i32 5, 8
%tr29 = ashr i32 %27, %tr28
store i32 %tr29, ptr %su, align 4
%fa30 = call i32 @GetSeason(i32 2)
store i32 %fa30, ptr %s_num
%xa = alloca float, align 4
%ty32 = sext i32 1 to i64
%fa33 = call float @switch_case(i64 %ty32, ptr null)
store float %fa33, ptr %xa, align 4
%ax = alloca [100 x [10 x i8]], align 16
%x = alloca ptr, align 8
%ty36 = getelementptr inbounds [100 x [10 x i8]], ptr %ax, i64 0, i64 0
store ptr %ty36, ptr %x, align 8
%xae = alloca [3 x float], align 16
%il38 = getelementptr [3 x float], ptr %xae, i32 0
%ty39 = fptrunc double 0.0 to float
%il40 = getelementptr [3 x float], ptr %il38, i32 0, i32 0
store float %ty39, ptr %il40
%fa41 = call float @switch_case(i32 1, ptr null)
%il42 = getelementptr [3 x float], ptr %il38, i32 0, i32 1
store float %fa41, ptr %il42
%il43 = getelementptr [3 x float], ptr %il38, i32 0, i32 2
store double 0.0, ptr %il43
%i = alloca i32, align 4
store i32 0, ptr %i, align 4
%xEA = alloca i32, align 4
%tr46 = icmp sgt i32 10, 20
store i1 %tr46, ptr %xEA, align 4
%t = alloca %struct.Two, align 16
%il48 = getelementptr %struct.Two, ptr %t, i32 0, i32 0
%il49 = getelementptr %struct.Two, ptr %il48, i32 0, i32 0
store i32 0, ptr %il49
%il50 = getelementptr %struct.Two, ptr %il48, i32 0, i32 1
store ptr %o, ptr %il50
%ma51 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%ma53 = load ptr, ptr %ma51, align 8
%ma54 = getelementptr %struct.Two, ptr %ma53, i32 0, i32 0
store i32 10, ptr %ma54
%xasd = alloca i32, align 4
%ma56 = getelementptr %struct.One, ptr %o, i32 0, i32 1
%ma58 = load ptr, ptr %ma56, align 8
%ma59 = getelementptr %struct.Two, ptr %ma58, i32 0, i32 0
%ma60 = load i32, ptr %ma59, align 4
store i32 %ma60, ptr %xasd, align 4
br label %.L17
.L17:
%61 = load i32, ptr %i, align 4
%tr62 = icmp slt i32 %61, 10
%63 = load i32, ptr %i, align 4
%tr64 = icmp slt i32 %63, 10
%65 = icmp ne i1 %tr64, 0
br i1 %65, label %.L18, label %.L19
.L18:
%j = alloca i32, align 4
store i32 0, ptr %j, align 4
br label %.L20
.L20:
%67 = load i32, ptr %j, align 4
%tr68 = icmp slt i32 %67, 100
%69 = load i32, ptr %j, align 4
%tr70 = icmp slt i32 %69, 100
%71 = icmp ne i1 %tr70, 0
br i1 %71, label %.L21, label %.L22
.L21:
%73 = load i32, ptr %i, align 4
%ma72 = getelementptr [100 x [10 x i8]], ptr %ax, i32 0, i32 %73
%75 = load i32, ptr %j, align 4
%ma74 = getelementptr [10 x i8], ptr %ma72, i32 0, i32 %75
%76 = load i32, ptr %i, align 4
%77 = load i32, ptr %j, align 4
%tr78 = add i32 %76, %77
%ty79 = trunc i32 %tr78 to i8
store i8 %ty79, ptr %ma74
%80 = load i32, ptr %j, align 4
%tr81 = add i32 %80, 1
store i32 %tr81, ptr %j
br label %.L20
.L22:
%82 = load i32, ptr %i, align 4
%tr83 = add i32 %82, 1
store i32 %tr83, ptr %i
br label %.L17
.L19:
%ma84 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%ma85 = load i32, ptr %ma84, align 4
%ma86 = getelementptr %struct.Two, ptr %t, i32 0, i32 0
%ma87 = load i32, ptr %ma86, align 4
%tr88 = icmp sgt i32 %ma85, %ma87
%ma90 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%ma91 = load i32, ptr %ma90, align 4
%ma92 = getelementptr %struct.Two, ptr %t, i32 0, i32 0
%ma93 = load i32, ptr %ma92, align 4
%tr94 = icmp sgt i32 %ma91, %ma93
%95 = icmp ne i1 %tr94, 0
br i1 %95, label %.L23, label %.L24
.L23:
%ma96 = getelementptr %struct.One, ptr %o, i32 0, i32 0
%ma97 = load i32, ptr %ma96, align 4
ret i32 %ma97
.L24:
%fa98 = call i32 @max(i32 1, i32 10)
ret i32 %fa98
.L25:
%hello = alloca [20 x i8], align 16
%il100 = getelementptr [20 x i8], ptr %hello, i32 0
%il101 = getelementptr [20 x i8], ptr %il100, i32 0, i32 0
store i8 72, ptr %il101
%il102 = getelementptr [20 x i8], ptr %il100, i32 0, i32 1
store i8 101, ptr %il102
%il103 = getelementptr [20 x i8], ptr %il100, i32 0, i32 2
store i8 108, ptr %il103
%il104 = getelementptr [20 x i8], ptr %il100, i32 0, i32 3
store i8 108, ptr %il104
%il105 = getelementptr [20 x i8], ptr %il100, i32 0, i32 4
store i8 111, ptr %il105
%il106 = getelementptr [20 x i8], ptr %il100, i32 0, i32 5
store i8 44, ptr %il106
%il107 = getelementptr [20 x i8], ptr %il100, i32 0, i32 6
store i8 32, ptr %il107
%il108 = getelementptr [20 x i8], ptr %il100, i32 0, i32 7
store i8 87, ptr %il108
%il109 = getelementptr [20 x i8], ptr %il100, i32 0, i32 8
store i8 111, ptr %il109
%il110 = getelementptr [20 x i8], ptr %il100, i32 0, i32 9
store i8 114, ptr %il110
%il111 = getelementptr [20 x i8], ptr %il100, i32 0, i32 10
store i8 108, ptr %il111
%il112 = getelementptr [20 x i8], ptr %il100, i32 0, i32 11
store i8 100, ptr %il112
%il113 = getelementptr [20 x i8], ptr %il100, i32 0, i32 12
store i8 33, ptr %il113
%il114 = getelementptr [20 x i8], ptr %il100, i32 0, i32 13
store i8 0, ptr %il114
%il115 = getelementptr [20 x i8], ptr %il100, i32 0, i32 14
store i8 0, ptr %il115
%il116 = getelementptr [20 x i8], ptr %il100, i32 0, i32 15
store i8 0, ptr %il116
%il117 = getelementptr [20 x i8], ptr %il100, i32 0, i32 16
store i8 0, ptr %il117
%il118 = getelementptr [20 x i8], ptr %il100, i32 0, i32 17
store i8 0, ptr %il118
%il119 = getelementptr [20 x i8], ptr %il100, i32 0, i32 18
store i8 0, ptr %il119
%il120 = getelementptr [20 x i8], ptr %il100, i32 0, i32 19
store i8 0, ptr %il120
%121 = call i32 (ptr, ...) @printf(ptr %hello)
%122 = alloca i32
store i32 %121, ptr %122
ret i32 0
}
define i32 @max(i32 %0,i32 %1) { 
%ix = alloca i32, align 4
store i32 %0, ptr %ix, align 4
%b = alloca i32, align 4
store i32 %1, ptr %b, align 4
%125 = load i32, ptr %ix, align 4
%126 = load i32, ptr %b, align 4
%tr127 = icmp sgt i32 %125, %126
%129 = load i32, ptr %ix, align 4
%130 = load i32, ptr %b, align 4
%tr131 = icmp sgt i32 %129, %130
%132 = icmp ne i1 %tr131, 0
br i1 %132, label %.L26, label %.L27
.L26:
%133 = load i32, ptr %ix, align 4
ret i32 %133
.L27:
%134 = load i32, ptr %b, align 4
ret i32 %134
.L28:
ret i32 0
}
