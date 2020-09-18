# 进程, 程序, shell

## 进程=运行中的程序

一个**程序**是存储在文件中的**机器指令序列**. 运行一个程序意味着将这个机器指令序列载入内存, 然后让CPU逐条执行这些指令.

进程存在于用户空间. 用户空间是存放运行的程序和它们的数据的一部分内存空间.
`ps`(process status)命令能查看用户空间的内容, 每个进程都与一个终端相连.

`ps -ax | head 25`显示Unix系统进程的前24个. 系统进程中很大一部分是没有终端与之相连的.
它们在系统启动时启动, 而不是由用户在命令行输入. 这些系统进程能做什么?
  - kernel buffer and virtual memory pages
  - 系统管理日志(klogd, syslogd)
  - 调度批任务(cron, atd)
  - 防范可能的攻击(portsentry)
  - 让一般的用户登录(sshd, getty)

  文件有一些属性, 进程也有一些属性. 内核建立文件和销毁文件, 进程也类似. 就像管理磁盘的多个文件, 内核管理内存中的多个进程, 为它们分配空间, 并记录内存分配情况.
  一个进程不一定必须要占一段连续的内存. 就像文件在磁盘上被分成小块, 进程在内存也被分成小块. 
  同样和文件有记录分配了的磁盘块的列表相似, 进程也有保存分配到的内存页面(menory pages)的数据结构.
  建立一个进程有点像建立一个磁盘文件. 内核要找到一些用来存放程序指令和数据的空闲内存页. 内核还有建立数据结构来存放相应的内存分配情况和进程属性.
  使操作系统变得神奇的不仅是它的文件系统把一堆旋转圆盘上连接的蔟变成有序组织的树状目录结构, 而且以相似的机制, 它的进程系统将硅片上的一些位组织成一个进程社会--成长, 相互影响, 合作, 出生, 工作, 死亡. 这有点像蚂蚁庄园.
  为了理解进程, 下面要学习和实现一个Unix shell. shell是一个管理和运行程序的程序.

## shell
  常用的shell有3个主要功能:
    - **运行程序**
      grep, date, ls都是一些普通程序, 用C编写, 并被编译成机器语言. shell将它们载入内存并运行它们. 
    - **管理输入和输出**
      shell不仅运行程序, 使用`<`, `>`和`|`符号能将输入输出重定向. 这样可以告诉shell将进程的输入和输出连接到一个文件或其他进程.
    - **可编程**
      shell同时也是带有变量和流程控制的编程语言.

一个shell的主循环执行下面的4步:
  1. 用户键入`a.out`;
  2. shell建立一个新的进程来运行这个程序;
  3. shell将程序从磁盘载入;
  4. 程序在它的进程中运行直到结束.

```txt
while( !end_of_file ) {
    get command
    execute command
    wait for command to finish
}
```

为了要写一个shell, 需要学会:
  - 运行一个程序
  - 建立一个进程
  - 等待exit()

### 一个程序如何运行一个程序?

answer: `execvp(progName, argList);`
1. 程序调用execvp
2. 内核从磁盘将程序载入
3. 内核将argList复制到进程
4. 内核调用main(argc, argv)

观察exec1.c程序.

```bash
unix> gcc exec1.c
unix> ./a.out
*** About to exec ls -l
total 28
-rwxrwxr-x 1 denton denton 16784 Sep 17 20:04 a.out
-rw-rw-r-- 1 denton denton   296 Sep 17 20:04 exec1.c
-rw-rw-r-- 1 denton denton  3125 Sep 17 20:00 README.md
```

第二条消息哪里去了?
内核将新程序载入当前进程, 替代当前进程的代码和数据.

execvp就像换脑. exec系统调用从当前进程中把当前程序的机器指令清除, 然后在空的进程中载入调用时指定的程序代码, 最后运行这个新程序.
exec调整进程的内存分配使之适应新的程序对内存的要求. 相同进程, 不同内容.

编写psh1.c, 这时Unix shell的第一个草案.
程序运行正常, 但是就像设想那样, execvp用命令指定的程序代码覆盖了shell的程序代码, 然后在命令指定结束之后退出.
shell如何做到在运行程序的同时还能等待下一个命令呢? 方法之一就是启动一个新的进程, 由这个进程来执行命令.

### 如何建立新的进程?

answer: A process calls `fork` to replicate itself

进程拥有程序和当前运行到的位置. 程序调用fork, 当控制转移到内核中的fork代码后, 内核做:
  1. 分配新的内存块和内核数据结构;
  2. 复制原来的进程到新进程;
  3. 向运行进程集添加新进程;
  4. 将控制返回给两个进程.

当按下克隆机的开始按钮后, 世界上有2个你, 物理上, 心理上都是相同的. 但是每个人将开始你(他)自己的人生之路. 
类似地, 当一个进程调用fork之后, 就有两个二进制代码相同的进程. 而且它们都是运行到相同的地方. 但是每个进程都将可以开始它们自己的旅程.

观察forkdemo1.c
观察forkdemo2.c 

如何分辨parent和child?
child从fork返回0.

### parent如何等待child的退出?

answer: 进程调用wait等待child结束.

系统调用wait做两件事情:
  首先, wait暂停调用它的进程直到child结束;
  然后, wait取得child结束时传给exit的值.

观察waitdemo1.c --> Notification
该例子体现了wait两个重要的特征:
  - wait阻塞调用它的程序直到child结束:
    这一特征是两个进程能够同步它们的行为. 比如parent用fork创建一个child对一个文件排序. parent必须等排序结束后才能继续处理这个文件.
  - wait返回结束进程的PID:
    一个进程可以创建多个child. wait的返回值告诉parent任务结束了.

观察waitdemo2.c --> Communication

  wait的目的之一是通知parent: child结束运行了. 它的第二个目的是告诉parent: child是如何结束的.
  一个进程以3中方式结束:
    - 成功:
      一个进程顺利完成它的任务. 按照Unix的惯例, 成功的程序调用exit(0)或从main函数中return 0.
    - 失败:
      比如进程由于内存耗尽而提前退出. 按照Unix管理, 程序遇到问题而要退出调用exit时传给它一个非零的值.
      程序员可以根据不同的错误分配不同的值, 手册有详细的描述.
    - 死亡:
      程序可能被一个信号杀死. 信号可能来自键盘, 间隔计时器, 内核或其他进程.
      通常情况下, 一个既没有忽略又没有被捕获的信号会杀死进程.

```bash
unix> make wait2
gcc -o wait2 waitdemo2.c -Wall -Werror
unix> ./wait2
before: my pid is 2081
child 2082 here. will sleep for 5 seconds
child done. about to exit
done waiting for 2082. Wait returned: 2082
status: exit = 17, sig = 0, core = 0

unix> ./wait2 &
before: my pid is 2615
child 2617 here. will sleep for 5 seconds
unix> kill 2617
done waiting for 2617. Wait returned: 2617
status: exit = 0, sig = 15, core = 0
```

```c
pid_t result = wait(int* statusPtr);

// returns -1 if encounters error, otherwise returns pid.
```

wait系统调用挂起调用它的进程直到得到这个进程的child的一个结束状态. 结束状态是退出值或信号序号.
如果一个child已经退出或被杀死, 对wait的调用立即返回, wait返回结束进程的PID.
如果statusPtr不是NULL, wait将退出状态或信号序号复制到statusPtr指向的整数中.
如果调用的进程没有child也没有得到终止状态值, 则wait返回-1.

---
