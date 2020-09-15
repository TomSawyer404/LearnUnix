# 信号和终端控制

## 不同软件对终端的看法

- 软件工具
  1. 对磁盘文件和设备文件不加以区分的程序被称为软件工具. 它们包括who, ls, sort, grep...
  2. 软件工具从stdin读取字节, 进行一些处理, 然后将包含结果的字节流写到stdout.

- 特定设备程序
  1. 例如: 控制扫描仪, 记录压缩盘, 操作磁带驱动程序, 拍摄数码相片的程序.

-  用户程序
  1. vim, emacs, more...
  2. 这些成雪设置terminal driver的击键和输出处理方式.
  3. 用户程序常用到的有:
    - immediate response to keys(立即响应击键)
    - limited input set(有限的输入集)
    - timeout on input(输入的超时)
    - resistance to Ctrl-C(屏蔽Ctrl-C)

## Terminal Driver的模式
编写rotate.c

### 规范模式
运行程序, 程序的运行情况如下: (`<-`是backspace键)

输入: 
```input
abx<-cd
efg Ctrl-C
```

输出:
```out
bcde
```

我们可以看出程序对标准输入的处理有如下特征:
1. 程序未得到输入的`x`, 因为backspace键删除了它
2. 击键的同时字符显示在屏幕上, 但是直到按了回车键, 程序才接收到输入.
3. Ctrl-C键结束输入并终止程序

Buffering, echoing, editing和control key processing都由terminal driver完成.
Buffering和Editing包含规范处理(canonical processing). 当这些特征被启动, 终端连接处于**规范模式**

### 非规范模式

尝试这个试验, 输入仍然是
```input
abx<-cd
efg Ctrl-C>
```

终端输入:
```bash
unix> stty -icanon; ./rotate
abbcxy^?cdde

effggh^C
unix> 
```

`stty -icanon`关闭了driver中的规范处理模式.
显而易见, 非规范处理没有缓冲. `a`输入后, driver跳过缓冲层, 将字符直接送到程序rotate, 然后程序显示`b`.
用户输入未被缓冲是一件麻烦事, 当用户试图删除一个字符, driver不能做任何事; 字符早就送给程序了.

最后一个小实验, 输入同上, 输出如下:
```bash
unix> stty -icanon -echo; ./rotate
bcyde
dgh
```
这个例子关闭了规范模式和回显模式. driver不再显示所输入的字符, 输出仅来自程序.


## 写一个用户程序

我们编写`play_again.c`, 它的逻辑如下:
 - prompt user with question
 - accept input
 - if `y`, return 0
 - if `n`, return 1

### play_again0
观察`play_again0.c`. 该程序有两个问题, 这两个问题都由运行时处在规范模式引起.
1. 用户必须按回车键, *play_again0*才能接收到数据.
2. 用户按回车键时, 程序接收整行的数据并进行处理. 因此输入`sure thing!`会得到一个否定的回答.

### play_again1 - 即时响应
第一个改进是关闭规范输入, 使得程序能够在用户敲键盘的同时得到输入的字符.
程序首先将终端置于一个字符连着一个字符的模式(char-by-char mode), 然后调用函数显示一个提示符, 并获得一个响应, 最后设置终端为原始的模式.
注意, 最后并未将终端置于规范模式, 取而代之的是将原先设置复制到一个称为`original_mode`的结构体中, 结束时恢复这些设置.

程序输入输出如下:
```bash
unix> make 1
gcc -o play_again1 play_again1.c -Wall -Werror
unix> ./play_again1
Do you want another transcation (y/n)?u
cannot understand u, Please type y or no
r
cannot understand r, Please type y or no
e
cannot understand e, Please type y or no
s
cannot understand s, Please type y or no
y

```

### play_again2 - 非阻塞输入
假设这个程序运行在ATM, 而顾客在输入y或n之前走开了, 将会怎样? 下一个顾客跑过来按下y, 就能进入那个离开的顾客的账号.
如果用户程序包含超时特征(timeout feature), 会变得更安全.

#### Blocking vs Nonblocking Input
  当调用getchar或read从文件描述符读取输入时, 这些调用通常会等待输入. 在上面的例子中, 对getchar的调用使得程序一直等待用户的输入, 直到用户输入一个字符. 程序**被阻塞**, 直到能获得某些字符或检测到EOF. 那然后关闭阻塞呢?
  阻塞不仅仅是终端连接是属性, 更是任何一个打开文件的属性.
  程序可以使用fcntl或open为文件描述符启动非阻塞输入(nonblock input).
  观察play_again2的代码, 发现以下问题:
运行在非阻塞模式下, 程序调用getchar给用户输入字符之前睡眠2s. 就是用户在1s内完成输入, 程序也在2s后才得到字符.
注意在显示提示符之后对fflush的调用. 如果没有那一行, 在调用getchar之前, 提示符将不能显示. 究其原因是, terminal driver不仅一行行地缓冲输入, 还一行行地缓冲输出, 直到它收到一个newline或程序试图从终端读取输入. 


## Signals(信号)

### Ctrl-C 做了什么?
1. 用户输入`Ctrl-C`;
2. 驱动程序收到字符;
3. 匹配VINTR和ISIG的字符被开启;
4. 驱动程序调用信号系统;

中断信号的击键组合不一定非是Ctrl-C, 可以使用stty将当前的VINTR控制字符替换成另一种键.

### 信号是什么?
信号是由单个词组组成的消息(A signal is a one-word message.)
当按下Ctrl-C时, 内核向当前正在运行的进程发送中断信号. 每个信号都有一个数字编码. 中断信号通常是*code number 2*.

信号来自哪? 信号来自内核, 生成信号信息的请求来自3个地方:
 - users
   用户通过输入`Ctrl-C`, `Ctrl-\`, 或是终端驱动程序分配给信号控制字符的其他任何键来请求内核产生信号.

 - kernel
   当进程执行出错时, 内核给进程发送一个信号, 例如:
   1. segmentation violation;
   2. a floating point exception;
   3. an illegal machine-language command.
   内核也利用特定信号通知其他进程特定事件的发生.

 - processes
   一个进程可以通过系统调用kill给另一个进程发送信号. 一个进程可以和另一个进程通过信号通信.

信号的分类:
 - 同步信号:
   由进程的某个操作产生的信号. 例如被零除.

 - 异步信号:
   由像用户击键这样的进程外事件引起的信号.

### 哪能找到信号的列表?

```bash
unix> man 7 signal
```

### 信号做什么?

视情况而定. 很多信号杀死进程. 某时刻进程还在运行, 下一秒它就消亡了, 从内存中被删除, 相应所有的文件描述符被关闭, 并且从进程表中被删除.
使用SIGINT消灭一个进程, 但是进程也有办法保护自己不被杀死.

### 进程该如何处理信号

1. 接受默认处理(通常是消亡): `signal(SIGINT, SIG_DFL);`

2. 忽略信号: `signal(SIGINT, SIG_IGN);`

3. 调用一个函数: `signal(SIGINT, functionName);`

处理信号的例子:
- 捕捉信号: sigdemo1.c

```bash
unix> make sig1
gcc -o sigdemo1 sigdemo1.c -Wall -Werror
unix> ./sigdemo1
Hello
^COUCH!
Hello
^COUCH!
Hello
^COUCH!
Hello
^COUCH!
Hello
```

- 忽略信号: sigdemo2.c

```bash
unix> make sig2
gcc -o sigdemo2 sigdemo2.c -Wall -Werror
unix> ./sigdemo2
YOU CANNOT STOP ME!!!
HaHa
HaHa
^CHaHa
HaHa
^CHaHa
HaHa
^\fish: ./sigdemo2 terminated by signal SIGQUIT (Quit request from job control with core dump (^\))))
```

按下Ctrl-\会发送一个不同的信号, 即quit信号, 这个程序没有忽略或捕捉SIGQUIT.

### 进程终止

程序使用signal来告诉内核需要忽略哪些信号. 如果有人编写了一个将所有类型的信号设置为`SIG_IGN`的程序, 然后执行一个无限循环呢?
幸好, 对系统管理员来说, Unix不可能让一个程序永不停止. 有2个信号是不能被忽略和捕捉的: SIGKILL 和 SIGSTOP.

---
