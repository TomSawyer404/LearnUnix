# 连接到近端或远端的进程: Server and Sockets

## bc: A Unix Calculator

bc其实是一个计算器的分析程序, 它本身并不计算表达式的值, 而是把中缀表达式转化为*逆波兰表达式*, 再把*逆波兰表达式*交给dc去计算.
(Ubuntu 20.04LTS并不自带dc, 这是因为GNU版本的bc是执行计算操作的.)

### coding bc: pipe, fork, dup, exec

1. 创建两个管道
2. 创建一个进程来运行dc
3. 在新创建的进程中, 重定向标准输入和标准输出到管道, 然后运行exec dc
4. 在parent中, 读取并分析用户的输入, 将命令传给dc, dc读取响应, 并把响应传给用户.

观察tinybc.c

### Remarks on Coroutines
其他的一些Unix工具能否被看作协同(coroutinue)进程呢? sort工具能否被当成coroutinue来使用?
答案是否定的. 在sort产生输出之前, 它读取文件中所有的数据, 通过管道来传递文件结尾标志的惟一途径就是将write-end关闭.
从另一方面讲, dc是逐行处理数据和命令的. 与dc的交互很简单并可预测.

一个clinet-server模型程序要成为协同系统必须由明确指明消息结束的方法, 并且程序必须使用简单并可预测的请求和应答

## popen: Making Processes Look Like Files

fopen打开一个指向文件的带缓存的连接:

```c
FILE* fp;                           // a pointer to a struct
fp = fopen("file1", "r");           // args are filename, connection type
c = getc(fp);                       // read char by char
fgets(buf, len, fp);                // line by line
fscanf(fp, "%d%d%s", &x, &y, x);    // token by token
fclose(fp);                         // close when done
```

popen看上去跟fopen类似, popen打开一个指向进程的带缓冲的连接:

```c
FILE* fp;                   // same type for struct
fp = popen("ls", "r");      // args are program name, connection type
fgets(buf, len, fp);        // exactly the same functions
pclose(fp);                 // close when done
```

下面程序将 `who | sort` 作为数据源, 通过popen来获得当前用户排列列表:

观察popendemo.c

### Writing popen: Using fdopen

popen是如何工作的? popen运行了一个程序并返回指向该程序标准输入或标准输出的连接.
这里需要一个新的进程来运行程序, 所以要用到fork命令. 
需要一个指向该进程的连接, 因此需要使用管道.
并且使用fdopen命令将一个文件描述符定向到缓冲流中.
最后在该进程中能够运行任何shell命令, 因此要用到exec.
但是会运行什么程序呢? 惟一能够运行任意shell命令的程序是shell本身即/bin/sh.
为了方便程序员的使用, sh支持`-c`选项, 可以告诉shell指向某命令然后退出, 例如: `sh -c "who|sort"`

观察popen.c程序
该版本的popen对信号不做任何处理, 这是不是有问题?

### Access to Data: Files, APIs, and Servers

fopen从文件获得数据, 而popen从进程获得数据. 这里主要关注以下数据访问的普遍问题并对三种实现方法进行比较.

- 方法1: 从**文件**获取数据
    可以通过读取文件来获得数据. 第二章所写的who程序就是从utmp文件读取数据的. 但基于文件的信息服务并不是很完美.
    客户端程序依赖特定的文件格式和结构体特定成员名称.
- 方法2: 从**函数**获取数据
    一个库函数用标准的函数接口来封装数据格式和位置. Unix提供了读取utmp文件的函数接口. getutent的帮助信息描述了
    读取utmp数据库函数的细节. 这样的话, 就算底层的存储结构变化了, 使用这个接口的程序仍然能正常运行.
    但使用基于API的信息服务也不一定是最好的办法. 有2种方法可以使用库函数. 
      a. 一个程序使用**静态链接**来办好实际的函数代码, 但这些函数可能包含的并不是正确的文件名或文件格式.
      b. 一个程序可以调用**共享库**种的函数, 但是这些共享库也不是安装在所以系统上, 或者其版本并不是所有程序所要使用的版本.
- 方法3: 从**进程**获取数据
    第三种方法是从进程种读取数据. bc/dc和popen例子显示了如何创建一个进程到另一个进程的连接. 一个要得到用户列表的程序
    可以使用popen来建立与who程序的连接. 由who命令来负责正确的文件名和文件格式以及正确的库函数, 而不是你的程序.
    调用独立的程序获得数据还有其他好处. Server程序可以使用任何程序设计语言编写: shell脚本, C, Java或是Perl都可以.
    以独立程序的方式实现系统服务的最大好处是Client程序和Server程序可以运行在不同的机器上. 所有要做的知识和
    不同机器上的一个进程相连接.

## Sockets: Connecting to Remote Process

管道使得进程向其他进程发送数据就像文件发送数据一样容易, 但是管道具有两个重大的缺陷. 管道在一个进程被创建,
通过fork来实现共享. 因此, 管道只能连接相关的进程, 也只能连接同一台主机的进程. Unix提供了另一个进程间的通信机制--socket.

socket运行在不相干的进程见创建类似管道的连接, 甚至可以通过socket连接其他主机上的进程. 
理解如何用socket连接不同主机上的客户端和服务端, 其思想就跟打电话查询当地时间一样简单.

### 重要概念

- clinet and server
  server是提供服务的程序. 在Unix中, server是一个程序而不是一台计算机. server进程等待请求, 处理请求, 然后循环回去等待
  下一个请求.
  client进程则不需要循环, 它只需建立一个连接, 与server交换数据, 然后继续自己的工作.

- hostname and port
  运行于因特网上的server其实是某台计算机上运行的一个进程. 这里计算机被称为*host*. 机器通常被指定一个名字
  如`sales.xyzcorp.com`, 这被称为该机器的*hostname*. server在该host上有一个*port*. *host*和*port*的组合才标识
  了一个server.

- address family
  你的时间服务必须拥有一个电话号码, 他还可能有街道地址和邮编, 甚至可能有经度和维度或其他集合的数据等属性.
  上述每个集合的数据都是的服务地址.
  上面的每个地址分别属于不同的address family. 电话号码和分机号码是电话网络地址地址族的地址, 类似, 经度和维度
  在全球坐标系统地址族中才有意义.

- protocol
  协议是server和client之间交互的规则. 
  如果运行一个查号辅助服务, 会是怎样的? 协议会复杂一点. server需要回答和发送初始欢迎消息信息(例如:"欢迎访问查号辅助系统").
  client给出城市的名字后, server将询问所查名字(例如:"你需要查询什么?"). client以公司或个人的名字来应答.
  这是server给出所要请求的电话号码或此城市不存在所查询的名字的消息.
  消息的交互遵*directory-assistance protocol*(**DAP**).  

### Lists of Services: Well-Known Ports

在美国每个人都知道911是紧急服务, 号码411是查好服务. 这些就是众所周知的端口.
在文件/etc/services中定义了众所周知的服务端口号的列表:

### Writing timeserv.c: A Time Server

电话服务有6个步骤, 每个步骤与一个系统调用相对应:

1. 获取电话线     ==>  socket
2. 分配号码       ==>  bind
3. 允许接入调用   ==>  listen
4. 等待电话       ==>  accept
5. 传送数据       ==>  read/write
6. 挂断电话       ==>  close

观察程序timeserv.c

下面将对程序如何工作给出解释:

#### 步骤1: 向内核申请一个socket
  socket是一个通信端点. 就像位于墙上的电话插座一样, socket是产生呼叫和接受呼叫的地方.
  socket调用创建一个通信端点并返回一个identifer. 有很多种类型的通信系统, 每个被称为一个**通信域**.
  Internet本身就是一个域. 在后面会看到Unix内核是另一个域. Linux支持好几个其他域的通信.

  socket的类型指出了程序将要使用的数据流类型. `SOCK_STREAM`类型跟双向管道类似. 数据作为
  连续的字节流从一端写入, 再从另一端写出. 后面会介绍`SOCK_DGRAM`类型.

  函数中最后的参数protocol指的是内核网络代码所使用的协议, 不是client和server之间的协议. 
  一个0值代表选择标准的协议.

#### 步骤2: 绑定地址到socket上, 地址包括主机, 端口
  在Internet域中, 地址由host和port构成.
  bind调用把一个地址分配给socket. 该地址分配就类似于把一个电话号码分配给墙上的插座; 当进程要与
  server连接的时候, 它们就使用该地址.
  每个地址族(address family)都有自己的格式. 因特网地址族(`AF_INET`)使用host和port来标志.
  地址就是一个以host和port为成员的结构体. 自己写的程序应该首先初始化该结构的成员, 然后再填充具体
  的值, 最后填充地址族.

#### 步骤3: 再socket上, 允许接受呼叫并设置队列长度为1
  server接收接入的呼叫, 所以这里的程序必须使用listen.
  listen请求内核允许指定的socket接收接入呼叫. 并不是所用类型的socket都能接收接入呼叫. 但
  `SOCK_STREAM`类型是可以的. 第二个参数指出接收队列的长度. 队列的最大程度取决于具体socket的实现.

#### 步骤4: 等待/接收呼叫
  accept阻塞当前进程, 一直到指定socket上的接入连接被建立起来, 然后accpet将返回文件描述符, 并
  用该文件描述符进行读写操作. 此文件描述符实际上是连接到呼叫进程的某个文件描述符的一个连接.

  就像人们使用来电显示的信息来决定如何处理打入的电话一样, 一个网络程序可以使用呼叫进程的地址来
  决定如何处理该连接.

#### 步骤5: 传输数据
  accept调用所返回的文件描述符是一个普通文件的描述符. 程序timeserv.c用fdopen将文件描述符定向到
  缓存的数据流, 以便于使用fprintf调用来进行输出. 在以前只能使用write来完成这项工作.

#### 步骤6: 关闭连接
  accept所返回的文件描述符可以由标准的系统调用close关闭. 当一端的进程关闭了该端的socket, 若另
  一端的进程在试图读数据的话, 它将得到文件结束标记. 

### 测试timeserv.c

```unix shell
banana@ubuntu> make
gcc -o tinybc tinybc.c -Wall -Werror -g

banana@ubuntu> ./timesev &

banana@ubuntu> telnet ubuntu 13000
Trying 127.0.1.1...
Wow! got a call!
fdopen: Success
Connected to ubuntu.
Escape character is '^]'.
Connection closed by foreign host.

```

### 编写timeclnt.c: 时间服务客户端

1. 获取一根电话线   ==>     socket
2. 呼叫服务器       ==>     connect
3. 传送数据         ==>     read/write
4. 挂断电话         ==>     close

观察程序timeclnt.c

下面是对该程序的解释。

#### 步骤1: 向内核请求建立socket
  client需要一个socket跟网络建立连接. 
  client必须建立Internet域(`AF_INET`)socket, 并且它还必须是流socket(`SOCK_STREAM`).

#### 步骤2: 与server相连
  connect调用试图把由`sockid`所表示的socket和由`serv_addrp`所指向的socket地址相连.
  如果连接成功的话, connect返回0. 而此时, sockid是一个合法的文件描述符, 可以用来进行
  读写操作. 写入该文件描述符的数据被发送到连接的另一端的socket, 而从另一端写入的数据
  将从该文件描述符读取.

#### 步骤3, 4: 传送数据和挂断
  在成功连接后, 进程可以从该文件描述符读写数据, 就像与普通文件或管道相连接一样.
  在时间服务的client/server例子中, timeclnt只是从服务器读取一行数据.
  读取时间之后, client关闭文件描述符然后退出. 若client退出而不关闭描述符, 内核将
  完成关闭文件描述符的任务.

## Software Daemons

  Unix Server程序有短小, 简结的名字. 很多Server程序都是以d结尾, 如httpd, inetd, syslogd和atd.
  这里的d代表**精灵**(daemon). 有的翻译叫**守护进程**.
  daemon就是一个为他人提供服务的帮助者, 它随时等待去帮助别人.
  在你的系统中, 输入命令`ps -el`或`ps -ax`就可以看到以字符d结尾的进程.

  大部分精daemon进程都是在系统启动后就处于运行状态了. 位于类似/etc/rc.d目录中是shell脚本在后台
  启动了这些服务, 它们的运行与终端相分离, 时刻准备提供数据或服务.

---
