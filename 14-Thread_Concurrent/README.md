# 线程机制: 并发函数的使用

## Doing Serveral Things At Once
  
  线程相对于函数就类似于进程相对于程序, 后者为前者提供运行环境. 
  很多函数可以同时运行, 但他们都在相同的进程中.

## Thread of Execution

### A Singel-Threaded Program

  观察程序`hello_single.c`.
  在该程序, *main*函数顺序地调用了两个函数. 每个函数执行了一个循环, 下面
  反映了程序的控制流:

```terminal
unix> gcc -o hello-single hello-single.c

unix> ./hello-single
HelloHelloHelloHelloHelloWorld
World
World
World
World

```

不间断地跟踪指令执行的路径在这里被称做**执行路线**(thread of execution). 
传统的程序只有一条单独的执行路线, 就是包含*goto*语句以及递归子程序的程
序也只有一条执行路线, 尽管这条路线有时有些弯弯绕绕.

### A Multithreaded Program

  人们无时无刻不在进行着多线程的任务管理. 如果父母需要做许多琐事, 他们通
  常会带上孩子一起去. 父母让一个孩子到杂货铺买牛奶, 另一个孩子去图书馆还
  书. 最后等两个孩子都回来之后, 大家再一起回家.

  一个线程就类似于上例帮父母做事儿的一个孩子. 如果想同时完成许多事儿, 最
  好多带几个孩子一起去. 类似地, 如果一个程序希望同时执行很多函数, 它必须
  创建多个线程. 下面的`hello_multi.c`实现了这样的思想.

  观察`hello_multi.c`.

  程序定义了`pthread_t`类型的两个变量t1和t2. 这两个线程类似于上面所说父母
  办事时所带的孩子.

```c
pthread_create(&t1, NULL, print_msg, (void*)"Hello");
````

  此函数就类似于父母喊道:"嗨! 孩子, 用参数`Hello`来运行函数`print_msg`." 
  参数含义:
      1. 线程的地址;
      2. 指向线程属性的指针;
      3. 所要执行的函数指针;
      4. 指向所要传递给函数的参数的指针.

```c
pthread_join(t1, NULL);
```

  此函数类似于父母等待孩子归来一样. *main*借助此函数来等待两个线程执行的线
  路返回. 函数`pthread_join`使用了两个参数:
      1. 所要等待的线程;
      2. 指向返回值的指针. 所此参数书NULL, 表示返回值不被考虑.

```terminal
unix> gcc hello_multi.c -lpthread -o hello-multi

unix> ./hello-multi
World
HelloWorld
HelloWorld
HelloWorld
HelloWorld
Hello

```

  使用线程进行编程就像给一些人赋予不同的任务. 如果加强项目管理, 保证
  所有的人都能够按序办事儿, 不和别人冲突, 这个项目肯定会提前完成. 下
  面介绍线程分工合作的技术

## Interthread Cooperation

  进程间可以通过管道, socket, 信号, *exit/wait*以及运行环境来实现会话.
  线程间的通信也很容易. 多个线程在一个单独的进程里运行, 共享全局变量,
  因此线程间可以通过设置和读取这些全局变量来进行通信. 不过要知道, 对共
  享内存的访问可是线程的一个既有用, 又极其危险的特性.

### Example 1: incrprint.c
  
  观察程序incrprint.c
  编译程序, 运行结果如下:

```terminal
unix> gcc incprint.c -o incprint.out -lpthread

unix> ./incprint.out
count = 1
count = 2
count = 3
count = 4
count = 5

```

  程序显示可以正常工作. 一个函数修改了变量, 另一个函数读取并显示了变量的
  值. 这个例子展示了如何使运行在不同线程中的函数共享全局变量.
  下面的例子更加有趣.

### Example 2: twordcount.c

  很多学生有这样的经验, 对着电脑数自己学期论文的字数以确定字数是不是足够.
  假设有一个学生有一篇10页纸的论文, 他有2种方法来计算这篇论文的字数.
  
  1. 一个字一个字地数10页纸;
  2. 找10个同学来, 给每个同学一页纸, 让他们分别计算, 最后把结果加起来.

  显然并行地计算10页纸的字数的方法会快很多.

  Unix平台上的*wc*程序的作用是计算一个或多个文件中的行, 单词以及字符的个数.
  不过*wc*是一个典型的单线程程序. 怎样来设计一个多线程程序来计数并打印两个文
  件中的所有数字呢?

#### Version 1: Two Thread, One Counter
  
  第一个版本程序创建分开的线程来对每一个文件进行计算. 所有的线程在检查到单词
  的时候对同一个计数器增值.

  观察代码twordcount1.c
  函数`count_words`是这样区分单词的: 凡是一个非字母或数字的字符跟在字母或数字
  的后面, 那么这个字母或数字就是单词的结尾. 当然这种思路忽略了文件的最后一个单
  词, 并且还把"U.S.A."看成三个独立的单词. 

```terminal
unix> gcc -o twc1.out twordcount1.c -lpthread

unix>  ./twc1.out ~/Desktop/main.c  ~/Desktop/main.c
   64: total words

unix> wc -w ~/Desktop/main.c  ~/Desktop/main.c
31 /home/banana/Desktop/main.c
31 /home/banana/Desktop/main.c
62 total

```

  两个结果不相同, 因为两个程序对单词结尾的规则定义不同.

  这有一个更微妙的问题: 所有线程对同一个计数器进行操作, 并且在同时进行. 
  这会不会有问题? C语言并没有指定操作`TotalWords`是如何被计算机执行的.

  如果所有的线程**在同一时刻**都使用"Fetch-add-store"的机器指令序列来完成对计
  数器的操作, 结果会怎样呢?
  
  | Thread 1's Register | Comment | Thread 2' Register | Comment | TotalWords |   
  | 100 | Fetch value from TotalWords | 100 | Fetch value from TotalWords  | 100 |   
  | 101 | Add 1 to value | 101 | Add 1 to value | 100 |   
  | 101 | Store to TotalWords | 101 | Store to TotalWorlds | 101 |   

#### Version 2: Two Threads, One Counter, One Mutex

  如果两个线程需要安全地共享一个公共计数器, 它们也需要一种方法给变量加锁.

  线程系统包含了称为*mutual exclusion locks*的变量, 它可以使线程间很好的合作, 避免
  对于变量, 函数以及资源的访问冲突.

  观察程序twordcount2.c

  我们是否需要Mutex? 如果多个线程企图在同一时刻修改相同的变量, 它们只好使用mutex来
  避免访问冲突. 然而使用mutex使得程序运行速度变慢. 对所有文件的每一个单词都需要执行
  检查, 设置以及释放锁的操作, 这使得程序效率低下.

  更加有效的办法是为每个线程设置自己的计数器.

#### Version 3: Two Threads, Two Counter, Multiple Arguments to Threads

  下一个版本的字数统计程序为每个线程设置了自己的计数器, 从而避免了对于mutex的使用. 当
  线程返回后, 再将这两个计数器的值加起来得到最后结果.

  如何来得到这些线程的计数器? 又如何使线程将他们的计数值返回呢? 在一个通常的单线程程
  序中, 字数统计函数将得出的字数返回给它们的调用函数. 线程可以通过调用`pthread_exit`得
  到返回值, 这个返回值又可以通过`pthread_join`的调用被原先的线程得到. 具体方法参见手册.

  下面使用一个稍微简单一点的办法.
  调用线程邕高传递给函数一个指向某变量的指针, 让函数对此变量进行操作, 从而可以避免让
  线程将值传回. 传递指针引发了一个问题: 函数`pthread_create`只能允许传递一个参数给函数,
  而文件名又必须传给函数, 那么如何传递这个指针呢? 办法很简单, 只需建一个包含两个成员的
  结构体, 然后将此结构体的地址传给函数即可.

  观察twordcount3.c

  每次调用函数`count_words`之后都会接收到一个指向不同结构体的指针, 因此线程从不同文件中
  读取信息, 并对不同计数器进行增1操作. 因为结构体是*main*中的局部变量, 所以分配给个计数
  器的内存空间在*main*函数返回前一直保存着.

## Interthread Notification

  假设你是一个 大城市的选举负责人. 城市中小一点的选区很快就完成了统计票数的工作, 而你却
  要等到所有数字都出来之后才能宣布结果. 不过你希望在每个选区票数出来之后立即可以看到结果.

  在文件中统计字数就像在选区统计票数一样. 有的文件比较大, 因此就需要较长的时间来统计.

  一个线程是如何与另一个线程互通消息的呢? 在一个计数线程完成任务之后, 它是如何通知原线程
  它的结果已经产生了呢? 对于进程而言, 当child终止之后, 系统调用*wait*返回.
  是不是对于线程的处理也有类似的机制? 答案是否定的. 线程不存在父线程和子线程.

### Programming with Condition Variables

  新的计数系统采用了3种设备: 邮箱, 旗帜和锁.
  - 邮箱: 保存数据
  - 旗帜: 一个条件变量
  - 锁: 一个mutex互斥量

  原线程启动了2个计数线程然后等待结果的到来. 特别地, 原线程调用`pthread_cond_wait`函数等待
  **旗帜升起**. 这个系统调用将原线程挂起.

  当某一线程完成计数后, 此线程通过把指针存入**邮箱**变量的方法来传递结果. 
  1. 首先此线程对邮箱加锁; 
  2. 然后线程检查邮箱:
     如果邮箱非空, 则线程**在邮箱再一次锁上之前**, 把邮箱解锁并等待信号到来.
  3. 之后, 线程结果放入邮箱;
  4. 最后计数线程调用函数`pthread_cond_signal`将此条件变量flag这面旗帜升起来

  此时由于执行`pthread_cond_wait`而挂起等待条件变量flag变化的原线程被计数线程发出的信号唤醒
  了. 原线程急切地想冲过去打开邮箱, 然而此时的邮箱仍然被计数线程锁在那里.

  当计数线程通过调用`pthread_mutex_unlock`把邮箱打开后, 原线程终于得到了对这把锁的控制权.
  原线程把选举报告从邮箱里拿出来, 在屏幕上显示, 再将其加到总数中去, 然后原线程发出信号, 以防
  别的计数线程正在等待. 最后原线程循环回去, 继续调用`pthread_cond_wait`函数, 自动将mutex解锁,
  并将自己挂起知道下一次信号的到来.

  观察twordcount4.c

```terminal
unix> gcc -o twc4.out twordcount4.c -lpthread

unix> ./twc4.out ~/Desktop/main.c ~/Desktop/main.c
MAIN: waiting for flag to go up
COUNT: waiting to get lock
COUNT: have lock, storing data
COUNT: raising flag
COUNT: unlocking box
MAIN: Wow! flag was raised, I have the lock
    32: /home/banana/Desktop/main.c
MAIN: waiting for flag to go up
COUNT: waiting to get lock
COUNT: have lock, storing data
COUNT: raising flag
COUNT: unlocking box
MAIN: Wow! flag was raised, I have the lock
    32: /home/banana/Desktop/main.c
    64: total words

```

  `phread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)`使线程挂起知道另一
  个线程通过条件变量发出消息. `pthread_cond_wait`函数总是和mutex一起使用. 此函数先自
  动释放指定的锁, 然后等待条件变量的变化. 如果调用此函数之前, mutex并没有被锁住, 函数
  的执行结果是不确定的. 在返回原调用函数之前, 此函数自动将指定的mutex重新锁住.

  `pthread_cond_signal(pthread_cond_t* cond)`函数通过条件变量cond发消息. 若没有线程等
  侯消息, 什么都不会发生; 若是多个线程都在等待, 只唤醒它们中的一个.


## A Threaded Web Server

### 在多线程的版本允许一个新功能
 
  多线程的特性允许我们添加一个新功能: 内部统计. 服务器的运行者通常希望知道服务器的运行
  时间, 接收客户端请求的数目以及发送会客户端的数据量.

  因为对于所有的请求共享内存空间, 可以使用共享变量的方式来统计. 那么用户如何访问这些统
  计数据呢? 这里加入一个特殊的URL: status. 当远程用户请求此URL时, 服务器将内部的统计数
  据发送给客户端.

### 防止Zombie Threads

  如果程序员忘记使用`pthread_join`函数来等待线程返回, 这些被线程所占用的资源就无法被回收
  , 类似于用*malloc*来分配的空间却没有用*free*释放掉一样.

  在字数统计的程序中, 原线程不得不等待所有的计数线程返回之后, 才可以搜集数据. 然而Web服务
  器却没有理由等待处理请求的线程返回. 因为原线程不需要从这些线程得到任何返回数据.

  这里同样可以创建不需要返回的线程, 称之为**独立线程**(Detached Threads). 当函数执行完毕
  之后, 独立线程自动释放它所用的所有资源, 它们自身甚至也不允许等待其他的线程返回. 可以通
  过传递一个特殊的属性参数给函数`pthread_create`来创建一个独立线程.

```c
/* Creating a detached thread */
pthread_t t1;
pthread_attr_t attr_detached;
pthread_attr_init(&attr_detached);
pthread_attr_setdetached(&attr_detached, PTHERAD_CREATE_DETACHED);
pthread_create(&t1, &attr_detached, func, arg);
```

  观察新的服务器代码: twebserv.c

---
