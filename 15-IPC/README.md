# 进程间通信IPC

## The Talk Command: Reading from Multiple Inputs

Unix的*talk*命令是一种通信工具. *talk*允许人们在终端间传递信息. *talk*甚至
可以跨越Internet来连接不同机器的终端.    
使用*talk*命令的时候, 屏幕被分为上下两个部分, 用户以字符终端模式来操作. 输
入字符的时候, 字符会同时显示在两个窗口中. 你所输入的字符会出现在上面的窗口,
而对方输入的字符出现在下面的窗口. *talk*使用socket进行通信.   

### Reading from two file descriptors at Once

*talk*从键盘和socket接收数据. 从键盘输入读取的字符被复制到屏幕中上半个窗口,
并通过socket发送出去. 同样从socke读入的字符被添加到屏幕下面的窗口中.   
*talk*的用户可以以任意速度和任意顺序输入字符. *talk*程序就必须在任何时刻都
准备好从任一数据源接收字符, 而不像其他的程序可以依靠明确的协议.   
服务器等待着*read*或*recvfrom*请求, 并用*write*或*sendto*发回一个应答. 用户
不可能一直在切换自己的角色(输入完后等待别人的应答, 然后再输入...).   
那么*talk*程序是如何解决这个问题呢? *talk*当然也不能这样做, 如下代码:

```c
while(1) {
    read(fd_kbd, &c, 1);     // read from keyboard
    waddch(wopwin, c);       // add to screen
    write(fd_sock, &c, 1);   // send to other person
    read(fd_sock, &c, 1);    // read from other person
    waddch(botwin, c);       // add to screen
}
```

按照上述代码的逻辑, 如果对方一直在输入信息, 而你却一直在看他发消息, 自己
没有输入过, 那结果会怎样? 程序在第一个*read*调用就挂起了, 并不会从你的对
方那里读取数据. 上面的方法只有在用户不断切换自己角色的时候才可以正常工作.   
这里可以调用*fcntl*函数将文件描述符设置为`O_NONBLOCK`标志从而使文件描述符
变为非阻塞模式. 使用非阻塞模式使得对于*read*的调用立即返回. 这个时候, 如果
并没有字符可以读取, *read*调用返回0. 虽然非阻塞的方式可以工作, 但是它占用
了太多CPU时间. 由于每次调用*read*都是一个系统调用, 程序就必须陷入到内核模
式工作. 这样在等待一个字符到来之前系统可能会切换上千次.   

### The select system call

Unix系统提供了系统调用*select*, 它允许程序挂起, 并等待从不止一个文件描述符
的输入. 它的原理很简单:
1. 获得所需要的文件描述符;
2. 将此列表传递给*select*;
3. *select*挂起直到任何一个文件描述符有数据到达;
4. *select*设置一个变量中的若干位, 用来通知你哪一个文件描述符已经有输入的数据.

观察下面的程序selectdeom.c, 它等待两个设备上是数据到达.   
*select*同时监控多个文件描述符. 在指定情况发生的时候,函数返回. 详细说一点, *select*
监听三组文件描述符上发生的事件: 
- 检查第一组是否可以读取;
- 检查第二组是否可以写入;
- 检查第三组是否有异常发生.

每一组文件描述符被记录到一个二进制位的数组中. 当参数指定的条件被满足或超时的
时候, *select*函数返回. 若指定的条件被满足, *select*返回满足条件的文件描述符数目.

### select vs. poll

也可以使用*poll*调用来代替*select*的功能. *select*是Berkeley研制出来的, 而*poll*
是贝尔实验室的成果. 这两者完成类似的功能, 而现代大部分Unix系统都支持它们.


## Communication Choices

*talk*中的文件描述符分别对应了键盘, 屏幕和socket, 但它们仍然可以被连接到其他进程
或设备上去. *talk*程序中进程间数据传输与进程间的操作都是极其重要的部分. 要知道选
择一个好的通信方式和选择正确的算法或数据结构一样重要.

### One Problem, Three Solutions

- The Problem: Getting Data from Server to Client

如何选择哪一种通信方法? 想一想前面使用的**流socket**写过时间/日期服务器. 某一进程
知道当前时间, 而另一进程想获取当前时间, 如何让一个进程从另一个进程得到数据?

- Three Solutions: File, Pipe, Shared Memory

第一种方法是之前学过的, 另外两个方法是**命名管道**(named pipe)以及
**共享内存(shared memory segment)**. 这些方法分别通过磁盘, 内核以及用户空间进行数据
的传输. 那它们如何具体应用? 各有什么优缺点?

### IPC with files

进程间通信可以通过文件来进行通信. 某进程将数据写入文件, 别的进程再将数据从文件中读出.

#### A Time/Date Server Using Files

这里不必要写一个完成的C程序, 下面的一个shell脚本就可以完成任务:

```bash
#! /bin/bash
# time server - file version

while true; do
    date > /tmp/current_date
    sleep 1
done

```

此服务器每个1秒种将当前时间和日期写入文件中.

#### A Time/Date Client Using Files

客户端读取文件内容:

```bash
#! /bin/bash
# time client - file version
cat /tmp/current_time

```

#### Remarks on IPC with Files

- Access:   
客户端必须能够读取文件. 使用标准文件访问权限, 可以给予服务器写权限并且限制
客户端只有读权限.   

- Multiple Clients   
任意数目的客户端可以同时从文件中读取数据. Unix并不限制同时打开同一文件的进程
数目.   

- Race Condition:   
服务器通过清空内容再重写的方法来更新文件. 如果某客户恰好再=在清空和重新之间
读取文件, 那么它得到的将是一个空的或只有部分的内容.   

- Avoiding Race Condition:   
服务器和客户端可以使用某种类型的mutex(互斥量)来避免竞态条件. 后面的章节将会学
到文件锁的方法. 另外, 如果服务器在程序中使用*lseek*和*write*函数来替换*create*, 
这样文件永远不会为空, 因为*write*是一个原子操作, 它不会在执行中被打断.   

### Named Pipes

通常的管道只能连接相关进程. 常规管道由进程创建, 并由最后一个进程关闭   
使用**命名管道**可以连接不相关的进程, 并且**可以独立于进程存在**. 称这样的命名
管道为FIFO(先进先出队列).    
FIFO类似于放在草坪上的一段给花园浇水的水管. 任何人都可以将此水管的一端放在自己
的耳朵边, 而另一个人通过水管向对方说话. 人们可以通过此水管进行交流, 而在没有人
使用的时候, 水管仍然存在. FIFO可以看作由文件名标志的一根水管.   

#### Using FIFO

- 如何创建FIFO?   
库函数`mkfifo(char* name, mode_t mode)`使用指定的权限模式来创建FIFO.
`mkfifo`命令通常调用这个函数.   

- 如何删除FIFO?   
类似于删除文件, `unlink(fifoname)`函数可以用来删除FIFO.   

- 如何监听FIFO的连接?   
使用`open(fifoname, O_RDONLY)`函数. `open`函数阻塞进程知道某一进程打开FIFO进行写
操作.   

- 如何通过FIFO开始会话?   
使用`open(fifoname, O_WRONLY)`函数. 此时`open`函数阻塞进程知道某一进程打开FIFO进
行读操作.   

- 两进程如何通过FIFO进行通信?   
发生进程用`write`调用, 而监听进程用`read`调用. 写进程用`close`来通知读进程通信结束.   
下面两个shell脚本是基于FIFO的时间/日期服务器的Server和Client.

```bash
#! /bin/bash
# time server

while true; do
    rm -f /tmp/time_fifo
    mkfifo /tmp/time_fifo
    date > /tmp/time_fifo
done

#! /bin/bash
# time client

cat /tmp/time_fifo

```

#### Remarks on IPC with FIFPs

- Access:   
FIFO使用与通常文件相同的文件访问. 服务器有写权限, 而客户端只有读权限.   

- Multiple Clients   
命名管道是一个队列而不是常规文件. 写者将字节写入队列, 而读者从队列头部移除字
节. 每个客户端都会将时间/日期的数据移出队列, 因此服务器必须重写数据.   

- Race Condition:   
FIFO版本的时间/日期服务器程序完全不存在竞态条件. 在信息的长度不超过管道容量的情
况下, *read*和*write*系统调用只是原子操作. 读操作将管道清空而写操作又将管道塞满.
在读者和写者连通之前, 系统内核将进程挂起. 因此锁机制在这里并不需要.   
时间/日期服务器将数据写入FIFO后, 将自己挂起知道客户端打开FIFO来读取数据. 在某些
应用程序中, 服务器从FIFO中读取数据, 然后等待客户端把数据写入. 大家想想看有没有服
务器等待客户端输入的例子?   

### Shared Memory

字节流是如何通过文件或FIFO来传输的? *write*将数据从内存复制到内核缓冲中. *read*将
数据从内核缓存复制到内存中.   
如果进程运行在用户空间的不同部分, 进程间如何将数据从内核缓存中复制进复制出呢?   
同一个系统的两个进程通过使用共享的内存段来交换数据.   
共享的内存段是用户内存的一部分. 每一个进程都有一个指向此内存段的指针. 依靠访问权
限的设置, 所有进程都可以读取这一块空间的数据.   
因此, 进程间的资源是共享的, 而不是复制来复制去的. 共享的内存段对于进程而言, 就类似
于共享变量对于线程一样.   

#### Facts about Shared Memory Segments

- A shared memory segment lives in memory independent of a process.
- A shared memory segment has a name, called a *key*.
- A key is an integer.
- A shared memory segment has an owner and permission bits.
- Processes may *attach* a segment, obtaining a pointer a to the segment.


#### Using a Shared Memory Segment

- How do I get a shared memory segment?   

```c
int seg_id = shmget(key, size-of-segment, flags);
```

如果内存段存在, 函数`shmget`找到它的位置. 如果不存在, 可以通过在flags值中指定一个创建
此段和初始化权限的请求.   

- How do I attach to a shared memory segment?

```c
void ptr = *shamt(seg_id, NULL, flags);
```

`shmat`在进程的地址空间中创建共享内存段的部分, 并返回一个只想好好此段的指针. flags参数
用来指定此内存段是否为只读.

- How do I read and write data to a shared memory segment?

```c
strcpy(ptr, "Hello");
```

`memcpy()`, `ptr[i]`以及一些其它通用的指针操作.

#### Remarks on IPC with FIFPs

- Access:   
客户端必须有对共享内存段的读权限. 共享内存段哦那个有一个权限系统, 它的工作原理和
文件权限系统类似. 共享内存段有自己的拥有者并且为用户, 组或其他成员设置了权限位, 来
控制他们各自的权限访问. 正因为有如此特性, 才可以让服务器只有写权限而客户端只有读权限.   

- Multiple Clients   
任意数目的客户都可以同时从共享内存段读数据.   

- Race Condition:   
服务器通过调用一个运行在用户空间的库函数*strcpy*来更新共享内存段. 如果客户端正好在服
务器向内存段中写入新数据的时候来访问内存段, 那么它可能既读到新数据也读到老数据.   

- Avoiding Condition:   
服务器和客户段必须使用相同的系统来对资源加锁. 内核提供了一种进程间加锁的机制, 称为
**Semaphore**. 我们在下一节学习这种机制.

### Comparing Communication Methods

#### Speed

通过文件或命名管道来传输数据需要更多的操作. 系统内核将数据复制到内核空间中, 然后再切
换回用户空间. 对于利用文件进行传输来说, 内核将数据复制到磁盘上, 然后将数据再从磁盘上
复制出去. 实际上, 在内存中存储数据比想象中要复杂许多. 虚拟内存系统允许用户空间中的段
交换到磁盘上, 因此就是共享内存机制同样也包括了对磁盘的读写操作.   

#### Connected or Unconnected

文件和共享内存段就像公告牌一样, 数据产生者将信息贴到公告牌上, 多个消费者可以同时从公告
牌上阅读信息. FIFO要求建立连接, 因为在内核转换数据之前, 读者和写者都必须等待FIFO被打开,
并且也只有一个客户可以阅读此消息. 流socket是面向连接的, 而数据包socket则不是. 在某些应
用程序中, 这些区别起着关键性的作用.   

#### Range

你希望程序中的消息能传达多远的距离呢? 共享内存和命名管道只允许本机上的进程之间通信. 通过
文件进行传输允许不同机器上的进程进行通信. 使用IP地址的socket可以与不同机器上的进程通信, 
而使用Unix地址的socket却不能. 这样使用哪一种方法进行通信取决于通信实体间的距离了.   

#### Restricted Access

你是希望所有人都能与服务器通信还是只有特定权限的用户才行? 文件, FIFO, 共享内存以及Unix地址
socket都提供标准的Unix文件系统权限. 而Internet socket则不行.

#### Race Condition

使用共享内存和共享文件要比使用管道和socket麻烦. 管道和socket是由内核来管理的队列. 写者
将数据放进一端, 而读者则从另一端将数据读出, 进程并不需要考虑其内部结构.   
然而对于共享文件和共享内存的访问却不是由内核进行管理的. 如果某进程在读文件的过程中, 另
一个进程正在对文件进行重写, 读进程读到的很可能就是不完整或不一致的数据.

## Interprocess Cooperation and Coordination

如何处理令人恼火的竞态条件呢? 客户和服务器若通过共享文件或内存的方式来进行通信, 又如何来保
证它们正常运行而不出现冲突呢? 它们如何分工合作? 本节将介绍进程访问共享资源时所使用的技术:
**file locks**和**semaphores**.

### File Locks

#### 两种类型的锁

考虑以下这样的情况. 当客户正在一行一行读数据的时候, 服务器突然把文件抢过来, 将内容删除, 然
后开始重写数据. 客户端看着文件从自己眼皮底下被抢过去而无能为力. 因此, 当客户在读取文件时, 服
务器也必须等待客户完成. 其他的客户不必去等, 因为多个进程一起读文件不会带来任何风险.   
为了避免这些问题, 需要两种类型的锁:
1. write lock: 它告诉其他进程: "我在写文件, 在完成之前任何人都必须等待!"
2. read lock: 它告诉其他进程: "我在读文件, 要写文件必须等我完成! 要读文件不受影响."

#### 用文件锁编程

Unix提供了3个方法锁住打开的文件: *flock*, *lockf*, *fcntl*. 三者最灵活和移植性最好的应该是
*fcntl*.   
下面使用*fcntl*锁文件:

- How do I set a read lock on an open file?

Use `fcntl(fd, F_SETLKW, &lockinfo);`.   

1. 第一个参数: 该文件对应的文件描述符;
2. 第二个参数: `F_SETLKW`说明若必要的话, 可以等待其他进程释放锁;
3. 第三个参数:  指向一个`struct flock`类型的变量. 下列代码为一个文件描述符设置读数据锁:

```c
set_read_lock(int fd) {
    stuct flock lockinfo();
    lockinfo.l_type = F_RDLCK;          // a read lock on a region
    lockinfo.l_pid = getpid();          // for ME
    lockinfo.l_start = 0;               // starting 0 bytes from..
    lockinfo.l_whence = SEEK_SET;       // start of file
    lockinfo.l_len = 0;                 // extending until EOF
    fcntl(fd, F_SETLKW, &lockinfo);
}

```

- How do I set a write lock on an open file?

使用`fcntl(fd, F_SETLKW, &lockinfo)`, 并将`lockinfo.l_type`置为`F_WRLCK`.

- How do I release a lock I hold?

使用`fcntl(fd, F_SETLKW, &lockinfo)`, 并将`lockinfo.l_type`置为`F_UNLCK`.

- How do I lock only part of a file?

使用`fcntl(fd, F_SETLKW, &lockinfo)`, 并将`lockinfo.l_start`置为开始偏移量.
同时将`lockinfo.l_len`置为区域的长度.   


编写基于文件的时间服务器和客户端代码.
观察`file_ts.c`和`file_tc.c`.

#### 文件锁小结

使用`F_SETLKW`参数调用*fcntl*可以使进程挂起直到内核允许进程设置指定的锁. 在读取数据之前, 客户
必须设置读取数据的锁. 若服务器对文件加写数据锁, 客户只好等待服务器完成. 服务器在重写数据之前, 
也必须对文件加写数据锁, 如果这时客户加了一个读数据锁, 那服务器会挂起直到所有客户释放这个锁.

#### 重要细节: 进程可以忽略锁机制

前面对于文件锁的讨论中, 不过客户还是服务器读或修改文件的时候, 程序都是自觉有序地等待, 设置及释放
文件锁. 那么当别的进程设置了锁的时候, 其他进程是否可以忽略它, 仍旧继续原来的读取或是修改操作吗?
答案是肯定的. Unix的锁机制允许进程通过这种方式合作, 但并不强迫它们一定要用.


### Semaphores

前面介绍了用锁的机制来解决访问文件的冲突, 共享内存段如何来避免数据冲突呢? 在共享内存段中是否也存在
读数据锁和写数据锁? 不是, 单进程使用一个更加灵活的机制来合作: **Semaphore**(信号量).   
信号量是一个内核变量, 它可以被系统中任何进程所访问. 进程间可以使用着变量来协调对于共享内存和其他资
源的访问. 上一章讨论了如何在特定的情况发生时使用条件变量来通知其他线程. 条件对象是进程中的全局变量, 
而信号量则是系统中的全局变量.   
在日期/服务器和客户端程序中如何使用信号量呢?   

#### Counters and Operations

在无客户端读取时, 服务器将数据写入共享内存段中. 同样地, 在服务器没有对共享内存段进行写操作时, 客户端
可以读取数据. 可以将这些规则转换为关于变量值的表达式:

- 客户端等待直到`number_of_writers == 0`
- 服务器等待直到`number_of_readers == 0`

信号量是系统级的全局变量, 这里可以使用两个信号量分别代表读者数和写者数. 管理这些变量需要两个操作.   

1. reader必须等待writer number为0时, 才可以将reader number加1. 当某reader读完数据, reader number必须
被减1;
2. writer也必须等待reader number为0时才可以将writer number加1. 等待reader numebr为0以及将writer number
加1是两个独立的操作, 必须分开, 即这两个操作都是原子操作. 通过使用信号量来通信的进程可以使用若干个这样
的变量, 并且独立的进行这些原子操作.   

#### Sets of Semaphores, Sets of Actions

时间服务器系统使用两个信号量, 并且读者和写者必同时对两个活动集进行操作.   
在修改共享内存之前, 服务器必须对这组活动集进行操作:
- [0] 等待`num_readers`变成0
- [1] 将`num_writers`加1
当服务器完成写操作之后, 它必须再对下面这组活动集进行操作:
- [0] 将`num_writers`减1

在客户读取共享内存之前, 必须对下面这组活动集进行操作:
- [0] 等待`num_writers`变成0
- [1] 将`num_readers`加1
当客户完成读操作后,需要对下面这组活动集进行操作:
- [0] 将`num_readers`减1

#### The code of Server

观察代码`shm_ts2.c`.

此程序中, 使用信号量集的服务器必须完成下面的5个步骤:

1. Create the semaphore set   
`semset_id = smeget(ket_t key, int numsems, int flags)`   
*semget*函数创建了一个包含`numsems`个信号量的集合。`shm_ts2`程序创建了包含两个信号量的集合. 此集合所拥有
的权限模式是0666. 函数*semget*返回此信号量集的ID.

2. Set both semaphores to 0   

`semctl(int semset_id, int semnum, int cmd, union semun arg)`   
这里使用*semctl*来对信号量进行控制(control). 此函数的各参数含义如下:   
- 第一个参数是此集合的ID;
- 第二个参数是集合中某特定信号量的号码;
- 第三个参数是控制命令; 如果此控制命令需要参数, 那么使用第四个参数向其提供所需的参数.

在`shm_ts2`程序中, 使用SETVAL命令来给每一个信号赋一个初始值零.

3. Wait until no readers, then increment `num_writers`   
`semop(int semid, struct sembuf* actions, size_t numactions)`   
函数*semop*对信号量集完成一组操作.   
- 第一个参数用来指定信号量集;
- 第二个参数是一组活动的数组;
- 最后一个参数则是该数组的大小.

集合中的每一个活动都是一个结构体, 它的作用就是"使用选项`sem_flg`来完成对号码为`sem_num`的信号量的操作
`sem_op`". 整个活动集合被作为组来完成, 这一点是关键. 上面程序中的函数`wait_and_lock`完成两个操作:
等待读者数到0, 然后将写者数加1.    
这里创建了一个包含着两个活动的数组:
- 活动0: 等待信号量0变成0;
- 活动1: 将信号量1加1;
进程挂起直到这两个活动被完成. 只要读者计数器一变成0, 写者计数器立即加1, 然后*semop*函数返回.   
使用`SEM_UNDO`标志允许内核在进程退出时恢复这些操作. 在上面这个程序中, 当写者计数器被加1时, 共享内存段
是被锁住的. 如果对此计数器做减1操作之后, 进程非法终止, 其他进程则永远无法读取到共享内存段的内容了.

4. Decrement `num_writers`   
在`release_lock`函数中, 只需完成一件事情: 对写者数减1. 这里使用一个只包含该活动的数组作为参数来调用
*semop*函数, 从而完成对写者数目的修改. 如果这时某客户正在等待, 它立即就可以继续执行读操作了.

5. Delete the semaphore   
`semctl(semset_id, 0, IPC_RMID, 0)`   
任务完成后, 服务器再次调用*semctl*函数来删除信号量

#### The code of Client

观察代码`shm_tc2.c`.

程序中并没有对可能出现的所有情况进行处理. 具体来说, 如何防止两个服务器程序同时运行? 在我们的程序中, 服
务器仅仅等待客户的读者服务器变为0而并没有对其他服务器的写者计数器进行判断.   

#### 等待某信号量变为正数

客户端等待服务器的写者信号量变为0, 而同时服务器等待着客户端的读者信号量变为0. 但在其他程序中, 也希望
等待的是某个信号量变成正数值. 举例来说, 也许希望等待信号量的值变为2. 如何来写这样的程序呢?   
这里使用一个不太直接的方法: 让系统内核对信号量做减2操作. 信号量不允许为负值, 因此系统内核将调用挂起
直到信号量的值大于或等于2. 信号量一旦到达2, 某进程就对它做减2操作, 然后把任何其他要对这个信号量减2的
进程挂起.   
这个操作的`sem_op`成员工作方式如下:
- 若`sem_op`是正值, 活动: 通过`sem_op`函数对信号量减2.
- 若`sem_op`是零, 活动: 挂起直到信号量等于0.
- 若`sem_op`是负值, 活动: 挂起直到信号量变成正值.

## A Print Spooler

### Many Writer, One Reader

多个用户共享一个打印机. 如何使用客户端/服务器模型来设计一个共享打印机的程序呢? 多个用户可能会同时发送
打印请求, 但是打印机在某一时刻只能打印一个文件. 打印程序就必须接收多个并发的输入, 并将单个的输出流送到
打印设备上. 如何来写这个服务器程序呢?   
在Unix系统中打印文件最简单的方法就是使用如下的命令:   
`cat filename > /dev/lp1`或`cp filename /dev/lp1`   
这里/dev/lp1是打印机设备文件名字. 当然系统中打印设备文件名并不一定和上面一样, 但在Unix系统中将数据传给
打印机或其他设备的唯一方法就是通过*open*打开文件, 然后使用*write*系统调用将数据写进打印文件中.   
我们已经学习过数据锁和信号量机制了, 为什么不可以自己写一个cat或cp的打印程序, 让它通过写数据锁来防止
对设备文件的同步访问冲突呢?   
基于锁机制的文件复制程序确实没有问题. 考虑以下对打印机加锁后, 结果会怎样.   
如果某程序对打印机加锁, 其他的文件复制程序都必须挂起等待第一个程序完成任务并释放锁. 那么下一步哪个程序
执行呢? 内核将所有挂起进程中的一个唤醒, 但是这个进程却不一定就是排在第二的. 显然这样的决定有失公平.
允许用户通过复制数据到打印设备文件来实现打印还有另一个问题:   
若某些人试图作假, 他们可以不使用这样一个加锁的程序来打印.   
第三个问题则是某些文件需要特殊处理. 例如图像文件有可能需要被转换为打印机可以看懂的图像命令. 很多用户
并不知道如何将数据转换为通用的格式, 那么它们就得不到正确的结果.

### A Client/Server Moduel

程序的客户/服务器模型解决了前面提到的打印问题. 只有一种称为**线性打印精灵**(line printer daemon)的服务
器程序有权限去写数据到打印设备文件中, 而其他的用户进程则不行.   
当用户需要打印文件时, 他们运行一个称为*lpr*的客户端程序. *lpr*对文件做了一个复制, 然后将复制的文件放在
打印任务队列中. 用户可以删除或编辑这个文件. 并且打印精灵程序可以将图片和格式做转换以使得他们能够正确地
被打印出来.   
客户端和服务器如何通信? 客户端将整个文件传给服务器还是客户端仅仅将文件名传给服务器呢? 如果服务器和客户
不在同一台机器上, 情况又如何呢? 不同版本的Unix中有不同的打印系统, 有的用socket, 有的用*fork*和文件.   

## IPC Overview

| Method | Type |   
|:----:|:----:|   
| exe/wait | M |   
| environ | M |   
| pipe | S |   
| kill-signal | M |   
| inet socket | S |   
| inet socket | M |   
| Unix socket | S |   
| Unix socket | M |   
| named pipe | S |   
| shared mem | R |   
| msg queue | M |   
| files | R |   
| variables | M |   
| file locks | C |   
| semaphpres | C |   
| mutexes | C |   
| link | C |   

*Key*:
- M: Sends data in short to medium-sized messages
- S: stream of data using read and write
- R: random access to data
- C: used to synchronize/coordinate tasks

## Connections and Games

说到游戏和网络, 让我们回忆下Dennis Ritchie是如何来描述用来引出Unix的空间探险游戏的:   

> First written on Multics, ..., it was nothing less than a simulation of the movement of the major
> bodies if the Solar System, with the player guiding a ship here and there, oserving the scenery,
> and attempting to land on the various planets and moons.

驾驶着太空船四处巡视, 观看空间景色并且可能在行星或其他卫星上登陆不就像生活中的网络冲浪一样吗?
可能冲浪并不是最好的比喻, 但确实人们打开他们的浏览器, 到全世界四处浏览, Web服务器将各处的景象返回.
人们使用*telnet*, *ssh*还有*ftp*登录到其他机器上. 也想Internet恰巧就是Ritchie和Thompson在1969年开始
模拟广阔空间的实观吧!


### What's next?

学习Unix系统编程最好办法就是不断读程序, 写程序. 大家可以在网上找到大量的信息, 以及介绍Unix内部实现和
编程接口的书籍. 大家多注意下每天都使用的程序还有一些吸引你的新程序. 通过使用, 学习, 并且经常自己来实现
一些已经存在的程序, 就可以更深, 更精, 更广泛地了解Unix编程了.   

---
