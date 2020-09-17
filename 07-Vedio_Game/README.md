# 事件驱动编程

## 屏幕编程

观察hello1.c, hello2.c

## 时钟编程1: sleep

观察hello3.c
程序运行, 将看到hello字符串从屏幕自上而下逐行显示, 每秒增加一行, 反色和正常显示交替出现.

观察hello4.c
hello4创造移动的假象. 字符串沿对角线缓慢向下移动. 注意在2次请求之后调用refresh来保证每次循环后旧的影像消失, 新的影像显示.

观察hello5.c
这个例子让字符串在屏幕四壁弹来弹去.
变量dir用来控制字符串的移动速度. 
 - 当dir是+1时, 字符串每一秒向右移一列.
 - 当dir是-1时, 字符串每一秒向左移一列.
我们现在直到了如何在屏幕的任何地方画字符串, 也知道了如何通过画, 擦掉和重画之间插入时延来创造动画效果.
只是:
 1. 一秒的时延太长, 需要更精确的计时器.
 2. 需要增加用户输入.

由此引出新话题: 时钟和高级信号编程.

## 时钟编程2: Alarms

系统中每个进程都有一个私有闹钟. 这个闹钟像一个厨房的计时器, 可以设置在一定秒数后响铃.
时间一到, 时钟就发送一个信号SIGALRM到进程, 除非进程为SIGALRM设置了handler, 否则信号将杀死这个进程.

sleep函数由3个步骤组成:
 1. 为SIGALRM设置一个处理函数;
 2. 调用`alarm(num_seconds)`;
 3. 调用pause.

系统调用pause挂起进程直到信号到达. 任何信号可以唤醒进程, 而非仅仅等待SIGALRM.
观察sleep1.c

## 时钟编程3: 间隔计时器

Unix很早就有sleep和alarm. 它们所提供的时钟精度为秒, 对于很多应用来说这个精度是不能让人满意的.
后来出现了**间隔计时器**(interval timer)的概念, 它有更高的精度.
每个进程有3个独立的计时器, 每个计时器都有2个设置: 初始间隔, 重复间隔.

`usleep(n)`将当前进程挂起 `n` microseconds, 或直到有一个不能被忽略的信号到达.

### 三个计时器

三个计时器的名字和功能如下:

  1. ITIMER_REAL
    - 这个计时器计量真实时间, 如同手表记录时间. 也就是说不管程序在用户态还是内核态用来多少CPU时间, 它都记录. 
    - 当这个计时器用尽, 发送SIGALRM消息.
  2. ITIMER_VIRTUAL
    - 这个计时器就像美式橄榄球中用的计时法, 只有进程在用户态运行时才计时. 虚拟计时器(virtual timer)的30s比实际计时器(real timer)的30s要长.
    - 当virtual timer用尽, 发送SIGALRM消息.
  3. ITIMER_PROF
    - 这个计时器在进程运行于用户态或该进程调用而陷入内核态时计时.
    - 当这个计时器用尽, 发送SIGPROF消息.

### 两种间隔: 初始和重复
  
  医生给你一些药丸并告诉你: " 过一小时吃第一粒, 然后每隔4小时吃一粒." 你需要设置计时器到一小时, 然后在每次时尽后再设置4小时.
  每个间隔计时器的设置都有这样两个参数: 初始时间和重复间隔. 在间隔计时器用的结构体中初始时间是`it_value`, 重复间隔是`it_interval`.

观察例子: ticker_demo.c
SetTicker通过装载初始间隔和重复间隔设置间隔计时器. 每个间隔是由**秒数**和**微秒数**组成, 仿佛实数的整数部分和小数部分.

间隔计数器的`struct itimerval`:

```c
struct itimerval {
    struct timeval it_value;    // time to next timer expiration
    struct timeval it_interval; // reload it_value with this
};

struct timeval {
    time_t tv_sec;          // seconds
    suseconds_t tv_usec;    // and microseconds
};
```

## 计算机有几个时钟?
如何才能让每个进程有3个独立的计时器? 有些系统同时有几百个进程在运行. 计算机里有几百个独立的时钟吗?
不, 一个系统只需要一个时钟来设置这些节拍.
一个硬件的时钟脉冲是计算机里唯一需要的钟.
  考虑两个进程: A和B. 进程A设置它的real timer为5秒, 进程B设置它的real timer为12s. 假设系统时钟每秒跳100下. 
  当进程A设置它的时钟时, 内核设置它的计数器为500. 
  当进程B设置它的时钟时, 内核设置它的计数器为1200.
  当计数器达到0了, 内核发送SIGALRM给进程A, 如果A设置了计时器的`it_interval`, 内核将这个值复制到`it_value`计数器, 否则内核就关掉这个计数器.

## 信号处理1: signal()

测试sigdemo3.c, 观察自己的系统是如何响应信号组合的.

 - SIGY打断SIGX的处理函数(接电话的时候有人敲门)
   当连续按下`Ctrl-C`和`Ctrl-\`会看到程序先跳到IntHandler, 接着跳到QuitHandler, 然后再回到IntHandler, 最后回到主循环.
 - SIGX打断SIGX的处理函数(两次敲门)
   这时有3种处理方法:
    1. 递归调用同一个处理函数
    2. 忽略第二个信号, 这个没有呼叫等待功能的电话机类似
    3. 阻塞第二个信号直到第一个处理完毕
    (Ubuntu 20.04 LTS实测是第3个方式)
 - 被中断的系统调用(接电话的时候有人敲门)
   如果输入`hel`, 接着按下`Ctrl-C`然后再继续输入`lo`再回车, 程序看到的是完整的`hello`还是`lo`? 程序是重新开始read还是从read返回同时设置errno到EINTR? (Ubuntu 20.04 LTS实测是只显示`lo`)

### 信号机制的弱点

1. 不知道信号被发送的原因.
2. 处理函数不能安全阻塞其他消息.

## 信号处理2: sigaction()

在POSIX种用sigaction代替signal.

```c
int sigaction(int signalNumber, const struct sigaction* action, struct sigaction* prevaction);

// returns 1 if fails, otherwise 0;

struct sigaction {
    // set only one of these two
    void (*sa_handler)();  // SIG_DFL, SIG_IGN, or function
    void (*sa_sigaction)(int, siginfo_t*, void*);  // new handler

    sigset_t sa_mask;   // signals to block while handling
    int sa_flags;       // enable various behaviors  
};

```

观察sigactdemo.c
如果以很快的速度连续按Ctrl-C和Ctrl-\, 退出信号将被阻塞直到中断信号处理完毕.
如果连续按两下Ctrl-C, 进程就被第二个中断信号杀死.
如果想要捕获所有的Ctrl-C, 讲`SA_RESETHAND`掩码从`sa_flags`种去掉.

## Data curruption

### Reentrant Code(重入代码)

一个signal handler或一个函数, 如果在激活的状态下能被调用而不引起任何问题就称之为**可重入**的.
如果signal handler是不可重入的, 必须阻塞信号. 但是如果阻塞信号, 就可能丢失信号. 

### kill: 从另一个进程发送信号

信号来自间隔计时器, 终端驱动, 内核或进程. 一个进程可以通过kill系统调用向另一个进程发送信号.

```c
int kill(pid_t pid, int sig);

// returns -1 if fail, otherwise returns 0.

```

Unix有两个信号可以用户程序使用. 它们是SIGUSR1和SIGUSR2. 这2个信号设有预定义的任务. 可以使用它们以避免使用已经预定义语义的信号.

## 继续游戏

bounce1d.c: 在一条直线上控制动画.

bounce2d.c: 二维动画.

## 异步IO

程序可以要求内核在得到输入时发送信号. 
Unix有2个异步输入系统:
  1. 当输入就绪时发送信号;
  2. 系统当被读入时发送信号.

UCB种通过设置file descriptor的`O_ASYNC`位来实现第一种方法
第二种方法是POSIX标准, 调用`aio_read`.

---
