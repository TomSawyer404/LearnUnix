# 目录与文件属性

本章将实现**ls**和**stat**小程序.

编写**ls**, 应当注意以下三点:
1. 如何列出目录的内容?
2. 如何读取文件的属性?
3. 给出一个名字, 如何判断它是目录还是文件?

## 目录与文件
查阅[操作系统: 三大简易元素](http://pages.cs.wisc.edu/~remzi/OSTEP/file-intro.pdf)的相关资料, 我们可以得到如下信息:

随着时间的推移, 存储虚拟化形成了两个关键的抽象.
- 文件
  1. 文件是一个线性字节数组, 每个字节都可以读取或写入. 
  2. 每个文件都有一个类似于身份证号码的数字, 由于历史原因, 这个数字被称为**inode number**.
- 目录
  1. 目录是一种特殊的文件, 它也有一个**inode number**.
  2. 目录有具体的内容, 它包含一个关于**映射**的列表.
  3. 这个**映射**的格式如下: `(user-readable-name, low-level-name)`. 假设有个*inode number*为10的文件, 它的名字叫`foo`, 那么这个`foo`所在的目录因此会有条目`("foo", "10")`.
  4. 目录中的每个条目都指向文件或其他目录

## 编写ls1.c
在命令行中输入
```bash
unix> man -k direct | grep read
```

找到`readdir`, 输入`man 3 readdir`, 阅读手册资料.
我们可以得知**ls**的一个简单逻辑:

```pseudocode
main() {
    opendir
    while( readdir ):
        print d_name
    closedir
}
```

我们编写`ls1.c`.

### 出现的问题
**ls1**实现了基本**ls**的基本功能, 但还可以做的更好:
1. 排序
  - **ls1**的输出没有经过排序. 
  - 解决办法: 把所以文件名读入一个数组, 用`qsort`函数把数组排序
2. 分栏
  - 标准的**ls**的输出是分栏排列的, 有些以行排序输出, 有些以列排列输出. 
  - 解决办法: 先把文件名读入数组, 然后计算出列的宽度和行数.
3. `.`文件
  - **ls1**列出了`.`文件, 而标准**ls**只有在给出`-a`参数时才会列出.
  - 解决办法: 使**ls1**能够接受参数`-a`, 并在没有`-a`的时候不显示隐藏文件.
4. 选项`-l`
  - 如果有选项`-l`, 标准的**ls**会列出文件的详细信息.
  - 解决办法: 由于结构体`dirent`没有文件大小, 文件所有者信息, 我们需要从别的地方获取文件的元数据(metadata).

## 编写stat程序
从[参考资料](http://pages.cs.wisc.edu/~remzi/OSTEP/file-intro.pdf)的39.9节中, 我们知道, 获取一个文件元数据的程序是`stat`.
使用方法:
```bash
unix> stat readme.md
    
    File: README.md
    Size: 2479            Blocks: 8          IO Block: 4096   regular file
    Device: 805h/2053d      Inode: 1704566     Links: 1
    Access: (0664/-rw-rw-r--)  Uid: ( 1000/  denton)   Gid: ( 1000/  denton)
    Access: 2020-08-21 11:33:05.489290258 +0800
    Modify: 2020-08-21 11:33:05.489290258 +0800
    Change: 2020-08-21 11:33:05.585304331 +0800
    Birth: -
```
继续在命令行输入: `unix> man -k stat | grep file`, 我们找到`stat`系统调用, 输入`unix> man 2 stat`, 仔细阅读手册, 了解关于结构体`stat`的信息.

有了这些信息, 我们开始编写**stat**程序.
写完后再命令行输入:

```bash
unix> ./stat1 stat1.c

mode: 100664
links: 1
user: 1000
group: 1000
size: 1139
name: stat1.c
modified_time: Aug 21 12:07:05 2020

unix> ls -l stat1.c
-rw-rw-r-- 1 denton denton 1139 Aug 21 12:07 stat1.c

unix> stat stat1.c
  File: stat1.c
  Size: 1139            Blocks: 8          IO Block: 4096   regular file
  Device: 805h/2053d      Inode: 1704565     Links: 1
  Access: (0664/-rw-rw-r--)  Uid: ( 1000/  denton)   Gid: ( 1000/  denton)
  Access: 2020-08-21 12:07:09.810160804 +0800
  Modify: 2020-08-21 12:07:05.738146533 +0800
  Change: 2020-08-21 12:07:05.742146547 +0800
  Birth: -

```

### 存在的问题
链接数, 文件大小的显示都还ok, 主要是文件的模式是以数字形式给出的, 但我们希望以`-rw-rw-r--`这样的形式给出.

## 利用掩码来建立映射
`st_mode`是一个16位的二进制数, 文件类型和权限被编码在这个数中. 它的结构如下(从1开始计数):
- 前4位: 文件类型
- 第5位: set-user-id
- 第6位: set-group-id
- 第7位: sticky位
- 8~10位: owner权限, r-w-x
- 11~13位: group权限, r-w-x
- 14~16位: other权限, r-w-x

在命令行输入: `unix> find /usr/inlcude -name stat.h`, 在Ubunut 20.04LTS系统下, `stat.h`的头文件在`/usr/include/linux/stat.h`. 打开阅读, 可知:

```c
#define S_IFMT   0170000  // type of file
#define S_IFSOCK 0140000  // socket
#define S_IFLNK  0120000  // sybolic link
#define S_IFREG  0100000  // regular
#define S_IFBLK  0060000  // block special
#define S_IFDIR  0040000  // directory
#define S_IFCHR  0020000  // characer special
#define S_IFIFO  0010000  // fifo
```

`S_IFMT`是一个掩码, 它的值`0170000`可以用来过滤出前`st_mode`四位表示的文件类型.
这些掩码的用法如下:

```c
if( (info.st_mode & 0170000) == 0040000 )
    printf("This is a directory!\n");
```

在实践中, `stat.h`还有更实用的宏(就在我摘抄代码的下面), 我们判断文件的话可以这么写:

```c
if( S_ISDIR(info.st_mode) )
    printf("This is a directory!\n");
```

有了这些信息, 我们可以编写`ls2.c`

## ls2还存在的问题
**ls2**基本实现了`ls -l`的功能, 但还有几个不圆满的地方:
1. 名字没有排序.
2. uid和gid可以转化为字符串
3. 无法显示指定目录的信息, 比如输入`unix> ./ls2 ../`会得到非预期的结果.

---
