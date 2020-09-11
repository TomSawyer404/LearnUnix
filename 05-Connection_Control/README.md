# 连接控制

## Unix的抽象

对Unix来说, 声卡、终端、鼠标和磁盘文件是同一种对象. 在Unix系统中, 每个设备都被当作一个文件. 
每个设备都有:
 - 文件名
 - i-node号
 - 文件所有者
 - 权限位的集合
 - 最近修改时间

## Devices have file name
通常表示设备的文件存放在目录`/dev`, 在Unix系统中输入: 

```bash
unix> ls -C /dev | head -10
```

能列出你机器的前10个设备.

## Devices are just like file
Unix的很多用户输入来自终端. ttysd、ttyse等文件都代表终端. 
按传统定义终端是键盘和显示单元, 但实际上可能包括: 
 - 20世纪70年代生产的打印机
 - 键盘
 - 串行接口的显示器
 - 调制解调器
 - 通过拨号上网的软件

 通过以下例子, 理解终端的文件性

```bash
unix> tty
/dev/pts/2

unix> echo 'hello world' > /dev/pts/2
hello world

unix> ls -li /dev/pts/2
5 crw--w---- 1 denton tty 136, 2 Sep  9 20:36 /dev/pts/2
```

`ls`的命令表面, `/dev/pts/2`拥有 *i-node号* 为5. 
文件类型为`c`, 表明这个文件实际上是以字符为单位进行传输的设备. 

### Devices file and file size
1. 常用的磁盘文件是字节组成的, 磁盘文件中的字节数就是文件的大小.
2. 设备文件是**链接**, 而不是容器. 键盘和鼠标不存储击键数和点击数.
3. 设备文件的*i-node*存储的是指向内核子程序的指针, 而不是文件大小和存储列表. 内核中传输设备数据的子程序被称为**设备驱动程序**.

在`/dev/pts/2`的例子中, 从终端进行数据传输的代码是在**设备-进程表**中编号为136的子程序. 该子程序接受一个整型参数.
在`/dev/pts/2`中, 参数是`2`. `136`和`2`这两个参数被称为**主设备号(major number)**和**从设备号(minor number)**. 
**主设备号**确定处理该设备实际的子程序, 而**从设备号**被作为参数传输到子程序.


### Devices file and Petmission bit
在这个例子中, 文件所有者和组tty成员拥有**写**设备的权限, 但是只有文件所有者有**读**设备的权限. 
读取设备文件就像普通文件一样, 从文件获得数据. 如果除了文件所有者还有其他用户能够读取`/dev/pts/2`, 那么其他人也能够读取该键盘上输入的字符. 读取其他人的终端输入会引起某些麻烦.
另一方面, 向其他人的终端写入字符是Unix中write命令的目标

### 编写write程序
在即时消息和聊天室出现之前, Unix用户通过命令write和其他终端上的用户聊天.
在命令行输入`man write`来了解程序功能.

仔细阅读`write0.c`, 在这里找不到连接到其他用户屏幕所需的特殊特征, 这个简单的`write0`程序将一个文件的内容一行行地复制到另一个文件.

### Device file and i-node
这些设备文件是怎么工作的呢?
目录是文件名和i-node的映射列表. 目录不能区分哪些文件名代表磁盘文件, 哪些文件名代表设备. **文件类型的区别体现在i-node上**.
- 每个i-node number指向一个*i-node table*中的一个结构. i-node可以是磁盘文件的, 也可以是设备文件的. 
- i-node的类型被记录在结构stat的成员变量`st_mode`的类型区域中.
- 磁盘文件的i-node包含指向数据块的指针. 设备文件的i-node包含指向**内核子程序表**.
考虑以下read是如何工作的. 内核首先找到文件描述符的i-node, 该i-node用于告诉内核文件的类型. 
 - 如果文件是磁盘文件, 那么内核通过访问**块分配表**来读取数据.
 - 如果文件是设备文件, 那么内核通过调用该设备驱动程序的read部分来读取数据.

## Device are not like file

连接具有属性, 磁盘文件和设备文件有不同的属性.

### 磁盘连接的属性
#### Buffering
系统调用fcntl可以关闭磁盘缓冲区
#### Auto-Append Mode
为什么Auto-Append Mode有用? 考虑日志文件wtmp.
wtmp存储所有的登录和推出记录. 当一个用户登陆时, 程序login在wtmp末尾追加一条登录记录. 当一个用户退出时, 系统在wtmp末尾追加一条退出记录. 这就像写日记一样, 每篇都被添加在末尾.
如果我们用lseek将当前位置移到文件末尾, 然后添加登录记录, 会出现什么问题? 观察下列时间片:
- 时间1: UserB的登录进程定位文件的末尾.
- 时间2: UserB的时间片用完, UserA的登录进程定位到文件末尾.
- 时间3: UserA的时间片用完, UserB的登录进程写入记录.
- 时间4: UserB的时间片用完, UserA的登录进程写入记录.
显而易见, UserA的登录进程写入记录**覆盖**了UserB的记录, UserB的登陆记录丢失.
这种情况被称为**竞争**(race condition). 
如何避免竞争? 内核提供了一个简单的解决办法: Auto-Appen Mode. 当文件描述符的`O_APPEND`位被开启后, 每个对write的调用自动调用lseek将内容添加到文件的末尾.

### 终端连接的属性

#### 终端的IO并不如此简单
终端和进程直接的连接看起来很简单. 通过使用getchar和putchar就能够再设备和进程间传输字节. 数据流的这种抽象使得键盘和屏幕看起来就像再进程中一样.

观察listchar.c, 一个简单的实验说明我们的模型并不完整.
输入输出:
```bash
unix> make listchars
unix> ./listchars
hello
char   0 is h code 104
char   1 is e code 101
char   2 is l code 108
char   3 is l code 108
char   4 is o code 111
char   5 is
 code 10
 Q
unix> 
```
如果字符代码直接从键盘流向getchar, 则再每个字符后可看到一个响应.
1. 输入单词`hello`中的5个字符并按回车, 程序才开始处理这些字符. 输入看起来被缓冲了. 就像流向磁盘的数据, 从终端流出的数据在沿途某个地方被存储了起来.
2. Enter键或Return键通常发生ASCII码`13`, 即*carriage return*. listchars的输出显示ASCII码`13`被`10`(*line feed* or *newline*)所替代.
3. listchars在每个字符串的末尾添加一个newline(`\n`). newline符告诉鼠标移动到下一行, 但没告诉它移动到左边. 代码`13`(carriage return)告诉鼠标移动到左边. [参考CSAPP课件Page 7对打字机的示意](http://www.cs.cmu.edu/afs/cs/academic/class/15213-f15/www/lectures/16-io.pdf)
综上, listchars在文件描述符中间必定有一个处理层, 包含以下三种处理:
1. 进程在用户输入Return后才接收数据;
2. 进程讲用户输入的Return(`13` in ASCII)看作newline(`10` in ASCII);
3. 进程发送newline, 终端接收Return-Newline对.
与终端的连接包含一套完整的属性和处理步骤.

## The Terminal Driver
处理进程和外部设备间数据流的内核子程序的集合被称为*terminal driver*和*the tty driver*(tty特指Teletype公司生产的老式打印机). 
驱动程序包含很多控制设备操作的设置. 进程可以读、修改和重置这些驱动程序标志.

### stty命令.
stty命令让用户读取和修改终端驱动程序的设置. 
```bash
unix> stty --all
speed 38400 baud; rows 32; columns 134; line = 0;
intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>; eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S;
susp = ^Z; rprnt = ^R; werase = ^W; lnext = ^V; discard = ^O; min = 1; time = 0;
-parenb -parodd -cmspar cs8 -hupcl -cstopb cread -clocal -crtscts
-ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr icrnl -ixon -ixoff -iuclc -ixany -imaxbel -iutf8
opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
isig icanon iexten echo echoe echok -echonl -noflsh -xcase -tostop -echoprt echoctl echoke -flusho -extproc
```
`icrnl`是*Input: convert Carriage Return to NewLine*, 即在前面的例子中驱动程序所做的操作. 
`onlcr`是*Output: add to NewLine a Carriage Return*.
一个属性前的减号代表这个操作被关闭.

tty驱动程序包含对传入数据的操作:
 - 输入: driver如何处理来自terminal的字符
 - 输出: driver如何处理流向terminal的字符
 - 控制: 字符如何被表示? 位的个数, 位的奇偶性, 停止位等
 - 本地: driver如何处理来自driver内部的字符

改变终端驱动程序的设置就像改变磁盘文件连接的设置一样:
1. 从driver获得属性
2. 修改所要修改的属性
3. 将修改过的属性送回驱动程序

### 编写terminal driver的几个例子
在`termios.h`中, 库函数提供了tcgetarr和tcsetattr来访问终端控制程序.
关于终端的信息被包含在`termios`的结构体中. 它们在大部分Unix版本都有如下结构:
```c
struct termios {
    tcflag_t c_iflag;      /* input modes */
    tcflag_t c_oflag;      /* output modes */
    tcflag_t c_cflag;      /* control modes */
    tcflag_t c_lflag;      /* local modes */
    cc_t     c_cc[NCCS];   /* special characters */
};
```


#### 例1: echostate.c
该例子说明终端是否被设置成回显字符模式. 读取模式, 测试位, 并报告结果

#### 例2: setecho.c
改变回显位的状态, 如果命令以`y`开始, 终端的回显标志被开启, 否则回显被关闭.
```bash
unix> make setecho
unix> ./echostate; ./setecho n; ./echostate; stty echo
echo is on, since its bit is 1
echo is OFF, since its bits is 0
unix> stty -echo; ./echostate; ./setecho y; ./setecho n
echo is OFF, since its bits is 0
```
第一个命令使用setecho关闭回显. 然后用stty将回显重新开启. driver和driver setting被存储在内核, 而不是进程.
一个进程可以改变driver setting, 另一个不同的进程可以读取或修改setting.

#### 例3: showtty.c
该例子显示大量驱动程序属性.
```bash
unix> make showtty
unix> ./showtty
the baud rate is Fast
The erase character is ascii 127, Ctrl-
The line kill character is ascii 21, Ctrl-U
Ignore break condition is OFF
Signal interrupt on break is OFF
Ignore chars with parity errors is OFF
Mark parity errors is OFF
Enable input parity check is OFF
Strip character is OFF
Map NL to CR on input is OFF
Ignore CR is OFF
Map CR to NL on input is ON
Enable start/stop output control is OFF
Enable start/stop input control is OFF
Enable signals is ON
Canonical input(srase and kill) is ON
Enable echo is ON
Echo ERASE as BS-SPACE-BS is ON
Echo KILL by starting new line is ON
```
showtty用来显示deiver里16个属性的当前状态. 程序使用了结构表以简化代码. 一个简单函数`show_flagset`接收一个整数和驱动程序标志集.

---
