# IO重定向和管道

## 标准I/O与重定向的若干概念

  所有Unix I/O重定向都是基于standard stream of data的原理. 
  所有的Unix工具都是用文件描述符0, 1和2. 

  通常通过shell命令行运行Unix系统工具时, stdin, stdout, steerr连接在终端上. 因此, 工具从键盘读取数据并且把输出和错误消息写到屏幕.
举例来说, 如果输入`sort`并按下回车, 终端将连接到sort工具上. 随便输入几行文字, 当按下Ctrl-D键结束文字输入时, sort程序对输入进行排序并将结果写道stdout.

  大部分的Unix工具处理从文件或标准输入读入的数据. 如果命令行上给出文件名, 工具将从文件读取数据. 若无文件名, 程序则从标准输入读取数据.
  从另一方面来说, 大多数程序并不接受输出文件名; **它们总是将结果写到文件描述符1去**, 并将错误消息写到文件描述符2.
  如果希望进程的输出写到文件或另一个进程的输入, 就必须重定向相应的文件描述符.

### 重定向I/O的是shell而不是程序

  观察例子listargs.c

```bash
unix> gcc listargs.c -Wall -Werror

unix> ./a.out testing one two
Number of args: 4, Args are:
args[0] ./a.out
args[1] testing
args[2] one
args[3] two
This message is sent to stderr.

unix> ./a.out testing one two > xyz
This message is sent to stderr.

unix> cat xyz
Number of args: 4, Args are:
args[0] ./a.out
args[1] testing
args[2] one
args[3] two

unix> ./a.out testing >xyz one two 2> oops

unix> cat xyz
Number of args: 4, Args are:
args[0] ./a.out
args[1] testing
args[2] one
args[3] two

unix> cat oops
This message is sent to stderr.

```

本章工作就是编写可以完成三个基本的重定向操作的程序:
 - `who > userlist`, 将stdout连接到一个文件
 - `sort < data`, 将stdin连接到一个文件
 - `who | sort`, 将stdout连接到stdin

### Lowest-Available-fd

  那什么是文件描述符呢? 
  它是一个数组的索引号. 每个进程都有其打开的一组文件. 这些打开的文件被保持在一个数组中.
  文件描述符即为某文件在此数组中的索引.

  FACT: When you open a file, you always get the lowest available spot in the array.

  通过文件描述符建立一个新的连接就像在一条多路电话上接收一个连接一样. 
  每当有用户拨一个电话号码, 内部电话系统就为这个拨号请求分配一条内部的线路号.
  在许多这样的系统上, 下一个打进来的电话就被分配给最小可用的线路号.

## 如何将stdin定向到文件

  进程并不是从文件读数据, 而是从文件描述符读数据. 如果文件描述符0定位到一个文件, 那么此文件就是标准输入源.

### 方法1: close then open
  
  这种技术类似于挂断电话释放一条线路, 然后再将电话拿起从而得到另一条线路.
  观察stdinredir1.c

```bash
unix> gcc stdinredir1.c -Wall -Werror

unix> ./a.out
line1
line1
testing line2
testing line2
line3 here
line3 here
root:x:0:0:root:/root:/usr/bin/fish
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
bin:x:2:2:bin:/bin:/usr/sbin/nologin

```

程序前三行从标准输入读取并打印字符串, 然后重定向标准输入, 之后又从标准输入读取并打印字符串.
程序从键盘读取了前三行, 而后三行则是从passwd文件中读出的.
此程序并没有什么特别的地方, 它仅仅挂断电话又拨了一个新的号码而已. 当连接建立起来后, 就可以从标准输入的一个新的源接收数据了.

### 方法2: open..close..dup..close
  
  考虑一下这种情况: 电话响了, 你拿起楼上的分机, 但你意识到自己应该下楼去接电话.
  于是你让楼下的人把电话拎起, 这样就有两个连接, 然后把楼上的分级挂断, 此时楼下的电话是惟一的连接了.

  Unix系统调用dup建立指向已经存在的文件描述符的第二个连接, 这种方法需要四个步骤:

  - open(file)
    第一步是打开stdin将要重定向的文件. 这个调用返回一个文件描述符, 这个描述符并不是0, 因为0在当前已经打开了

  - close(0)
    下一步是将文件描述符0关闭, 文件描述符0现在已经空闲了.

  - dup(fd)
    系统调用dup(fd)将文件描述符fd做了一个复制. 此次复制使用最低可用文件描述符号.
    因此, 获得的文件描述符是0. 这样, 就将磁盘文件与文件描述符0连接到一起了.

  - close(fd)
    最后, 使用close(fd)来关闭文件的原始连接, 只留下文件描述符0的连接. 

下面的程序stdinredir2.c使用了第二种方法.

### 方法3: open..dup2..close

  一个简单一点的方案是将close(0)和dup(fd)结合在一起作为一个单独的系统调用dup2.

```c
int newfd

newfd = dup2(oldfd, newfd);

// returns -1 if fails, otherwise newfd is returned.
```


## 为其他程序重定向I/O

  当用户输入`who > userlist`运行who程序, 并将who的标准输出重定向到名为userlist的文件上. 这是如何完成的?
  **关键之处就在于fork和exec之间的时间间隙**.
  
  看下面举例说明:

### Start here

文件描述符1连接在打开的文件f上.

### After parent calls fork

child包含了与parent相同的代码, 数据和打开文件的文件描述符.
因此child的文件描述符1依然指向的是文件f. 然后child调用了close(1).

### After child calls close(1)

parent没有调用close(1), 因此parent的文件描述符1指向f.
child调用close(1)后, 文件描述符1变成了最低未用文件描述符.
child现在试图打开文件g.

### After child calls creat("g", m)

child的文件描述符1被连接到g. child的标准输出被重定向到g.
child然后调用exec来运行who.

### After child exec a new program

child指向who程序. 于是child中的代码和数据都被who程序的代码和数据所代替了, **然而文件描述符被保留下来**.
**打开文件**并不是程序的代码也不是程序的数据, 它们属于进程的属性, 因此exec调用并不改变它们.
who命令将当前userlist送至文件描述符1. 其实这组字节已经被写到文件g中去了, 而who命令却毫不知情.

观察程序whotofile.c, 理解上面的说明.

## 管道编程

下列系统调用可以创建管道.

```c
result = pip(int array[2]);

// returns -1 if fails, otherwise 0 is returned.
```

调用pipe来创建管道并将其两端连接到两个文件描述符.
array[0]为**读数据端**的文件描述符, 而array[1]则为**写数据端**的文件描述符.
像一个打开的文件的内部情况一样, 管道的内部实现隐藏在内核, 进程只能看见两个文件描述符.

程序pipedemo.c展示了如何创建管道并使用管道来向自己发送数据.
实际上, 很少会有程序用管道向自己发送数据. 将pipe和fork结合起来, 就可以连接两个不同的进程了.

### 使用fork来共享管道

当进程创建一个管道之后, 该进程就有了连向管道两端的连接. 当这个进程调用fork的时候, 它的child也得到了这两个连向管道的连接.
parent和child都可以将数据写到**数据端口**, 并读取**数据端口**将数据读出.
两个进程都可以读写管道, 但是当一个进程读, 另一个进程写的时候, 管道的使用效率是最高的.

程序pipedemo2.c说明了如何将pipe和fork结合起来, 创建一对通过管道来通信的程序.

程序pipe.c使用2个程序的名字作为参数, 把上面的技巧都综合在一起.

```
pipe who sort
pipe ls head
```

pipe.c用了和shell一样的死了和技术来建立管道. 但是shell并不像pipe.从一样运行外部程序.
shell首先创建管道, 然后调用fork创建两个新进程, 再将标准输入和输出重定向到创建的管道, 最后再通过exec来执行两个程序.

### 技术细节: 管道并非文件

#### reading from pipes

1. read on a pipe blocks
    当进程试图从管道中读数据时, 进程被挂起直到数据被写进管道. 那么如何避免进程永无止境地等下去呢?
2. Reading EOF on a pipe
    当所有的writer关闭了管道writing-end时, 试图从管道读取数据的调用返回0, 这意味着文件的结束
3. Multiple readers can cause trouble
    管道是一个队列. 当进程从管道读取数据之后, 数据就已经不存在了.
    如果两个进程试图对同一个管道进行读操作, 在一个进程读取一些之后, 另一个进程读到的将是后面的内容.
    它们读到的数据必然是不完整的, 除非两个进程使用某种方法来协调他们对管道的访问.

#### writing to pipes

1. write to a pipe blocks until there is space
    管道容纳数据的能力比磁盘文件差的多. 当进程试图对管道进行写操作时, 此调用将挂起进程直到管道中有足够的空间去容纳新的数据. 比如进程想写入1000Bytes, 而管道现在只能容纳500Bytes, 那么这个写入调用就只好等待直到管道中再有500Bytes空出来. 如果某进程试图写a million bytes, 那结果会怎样?

2. write guarantees a minmimum chunk size
    POSIX标志规定内核不会拆分小于512Bytes的块. 而Linux则保证管道中可以存在4096Bytes的连续缓存. 如果两个进程向管道写数据, 并且每一个进程都限制其消息不大于512Bytes, 那么这些消息都不会被内核拆分.

3. write fails if no readers
    如果所有的reader都已经将管道reading-end关闭, 那么对管道的写入调用将会执行失败. 如果在这种情况下, 数据还可以被接收的话, 它们会到哪里去呢? 为了避免数据丢失, 内核采用了2种方法来通知进程:"此时的写操作是无意义的"." 首先, 内核发送SIGPIPE消息给进程. 若进程被终止, 则无任何事情发生. 否则write调用返回-1, 并且将errno置为EPIPE.

---
