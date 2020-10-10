# 实现一个shell: psh2.c

编写psh2.c

它工作正常, 下一个版本需做到如下改进:
  1. 让用户可以他通过按下Ctrl-D或者输入`exit`退出程序;
  2. 让用户能够在一行中输入所有参数.

## 思考: 用进程编程

### execvp/exit就像call/return

  一个C程序由很多函数组成. 一个函数可以调用另一个函数, 同时传给它一些参数. 被调用的函数执行一定的操作, 然后返回一个值.
  每个函数都有它的局部变量, 不同的函数通过call/return系统进行通信.
  这种通过参数和返回值在拥有私有数据的函数间通信的模式是结构化程序设计的基础. Unix鼓励将这种应用于程序之内的模式扩展到程序之间.

### exec/exit

  一个C程序可以fork/exec另一个程序, 并传给它一些参数. 这个被调用的程序执行一定的操作, 然后通过exit(n)来返回值. 
  调用它的进程可以通过wait(&result)来获取exit的返回值. child的exit返回值可以在result的8~15位之间找到.
  函数调用所用到的栈几乎是没有限制的. 一个被调用的程序还可以调用其他程序.
  Unix使创建一个新进程方便而且快捷. 用fork/exit和exit/wait来调用程序和返回结果不仅适用于于shell, Unix程序经常被设计成一组子程序, 而不是一个带有很多函数的大程序.
  由exec传递的参数必须是字符串. 由于进程间通信的参数类型为字符串, 这样就强迫了子程序的通信也必须使用文本作为参数类型.
  几乎是偶然的, 这种基于文本的进程接口支持跨平台的交互, 而这一点非常重要.

### 全局变量和fork/exec

  全局变量是有害的, 它破坏了封装原则, 导致出人意料的副作用和难以维护的代码. 但有时候去掉全局变量却更糟糕.
  怎样才能做到不把参数变得复杂的情况下管理一堆每人都要用到的变量? 尤其是必须将他们向下传几层的时候, 情况会更麻烦.

  Unix提供的方法来建立全局变量. 环境(environment)是一些传递给进程的string型变量集合. 
  它对fork/exec和exit/wait机制是一个有用的补充.

## shell变量和环境

### shell编程

  shell是一个编程语言解释器, 这个解释器解释从键盘输入的命令, 也解释存储在脚本中的命令序列.
  shell脚本是一个包含一系列命令的文件. 运行一个脚本就是运行这个文件中的每个命令.
  shell脚本是真正的程序. 除了命令之外还包括以下元素:
    - 变量
    - 用户输入
    - 控制
    - 环境

  编写smsh1
  (教材给的代码由SIGSEGV错误, 懒得找代码漏洞在哪了)
  

### 环境

  Unix运行用户在称之为**环境(environment)**的地方以变量的形式存放这些设置. 
  每个用户都有一个惟一的主目录, 用户名, 邮件文件, 终端类型和喜欢用的编辑器, 很多个性化设置由环境中的变量记录.

  **环境**是每个程序都可以存放的一个字符串数组. 
  每个数组中的字符串都已`var=value`这样的形式出现, 数组地址被存放在一个名为`environ`的全局变量里. 
  环境就是environ指向的字符数组, 读环境就是读这个字符串数组, 改变环境就是改变字符串, 改变这个数组中的指针或者这个全局指针指向的其他数组.

编写showenv.c, 它的功能就像命令env

---