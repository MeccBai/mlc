target triple = "x86_64-w64-windows-gnu"
%struct.o = type {i32,i32}
%struct.p = type {i32,i32}
%struct.oo = type {%struct.o,%struct.o}
declare i32 @printf(...)
declare i32 @puts(i64 %0)
declare i32 @putchar(i32 %0)
declare i32 @getchar()
declare i32 @scanf(...)
declare i32 @fscanf(...)
declare i32 @hello()
declare i32 @fprintf(...)
declare i64 @fopen(i64 %0,i64 %1)
declare i32 @fclose(i64 %0)
declare void @perror(i64 %0)
declare i32 @fflush(i64 %0)
define void @printOO(ptr noundef %0) { 
%ax = alloca %struct.oo, align 16
call void @llvm.memcpy.p0.p0.i64(ptr align 8 %ax, ptr align 8 %0, i64 16, i1 false)
%ma22 = getelementptr %struct.oo, ptr %ax, i32 0, i32 0
%ma23 = getelementptr %struct.o, ptr %ma22, i32 0, i32 0
store i32 10, ptr %ma23
%ma24 = getelementptr %struct.oo, ptr %ax, i32 0, i32 0
%ma25 = getelementptr %struct.o, ptr %ma24, i32 0, i32 1
store i32 20, ptr %ma25
%x = alloca i32, align 4
%ma27 = getelementptr %struct.oo, ptr %ax, i32 0, i32 0
%ma28 = getelementptr %struct.o, ptr %ma27, i32 0, i32 0
%ma29 = load i32, ptr %ma28, align 4
%ma30 = getelementptr %struct.oo, ptr %ax, i32 0, i32 0
%ma31 = getelementptr %struct.o, ptr %ma30, i32 0, i32 1
%ma32 = load i32, ptr %ma31, align 4
%tr33 = add i32 %ma29, %ma32
store i32 %tr33, ptr %x, align 4
ret void
}
define i32 @main() { 
ret i32 0
}
