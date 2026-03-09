// std/io.mc

// 核心输出：支持格式化字符串
export i32 printf(...);

// 核心输入：注意在 mlc 调用时，变量前要加 @ 获取地址
export i32 scanf(...);

// 快速字符串输出：自动换行，效率极高
export i32 puts(i8$ str);

// 字符级操作：处理原始字节流的基础
export i32 putchar(i32 c);
export i32 getchar();

// 进阶补充：错误处理与缓冲区刷新
export i32 fflush(i8$ stream); // 传 0 通常代表刷新所有缓冲区
export void perror(i8$ msg);    // 打印系统级错误信息

export i8$ fopen(i8$ filename, i8$ mode); // 返回 FILE$
export i32 fclose(i8$ stream);
export i32 fprintf(...);
export i32 fscanf(...);

export struct iot {
    i32 a; i32 b;
};

export struct iota {
    i32 x; i32 y; iot io;
};