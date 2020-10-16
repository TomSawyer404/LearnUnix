# 连接与协议: 编写Web服务器

## 3 Main Operation

交互过程包含以下3个操作:
1. Server set up a service
2. Client connect to a server
3. Server and Client do bussiness.

下面分别讨论每个操作.

## Operation 1 and 2: Making Connection

基于流的系统需要建立连接. 

### Operation 1: Setting up a Server Socket
  
  1. Create a socket
     `socket = socket(PF_INET, SOCK_STREAM, 0)`
  2. Give the socket an address
     `bind(sock, &addr, sizeof(addr))`
  3. Arrange to take incoming calls
     `listen(sock, queue_size)`

  为了避免在编写服务器时重复输入上述代码, 将这3个步骤合成一个函数: `make_server_socket`.
  该函数的代码位于socklib.c;
  在编写服务器时, 只要调用函数就可以创建一个服务器端socket.

### Operation 2: Connecting to a Server

  1. Create a socket
     `sock = socket(PF_INET, SOCK_STREAM, 0)`
  2. Use the socket to connect to a server
     `connect(sock, &serv_addr, sizeof(serv_addr))`

  将这两个步骤抽象成一个函数: `connect_to_server`. 

编写代码socklib.c

## Operation 3: Server/Client Conversation

在实践中, 如何利用上述函数呢? client和server之间交互的内容又是什么呢?
本节将学习client程序和server程序编写的一般形式, 以及一些建立server的设计方案.

- The Generic Client
  网络客户通常调用服务器来获得服务, 一个典型的客户程序如下:
   
```c
int main() {
    int fd = connect_to_server(host, port);  // call the server
    if(fd == -1)  exit(1);

    talk_with_server(fd);   // chat with server
    close(fd);              // hang up when done
}
 ```

函数`tale_with_server`处理与服务器的会话. 具体内容取决于特定应用.
例如e-mail客户和邮件服务器交谈的是邮件, 而天气预报客户和服务器交谈的则是天气.

- The Generic Server

```c
int main() {
    int sock = make_server_socket(port);
    if(sock == -1)  exit(1);

    while(1) {
        int fd = accept(sock, NULL, NULL);  // take next call
        if(fd == -1)  break;

        process_request(fd);    // chat with client
        close(fd);              // hang up when done
    }
}
```

函数`process_request`处理与客户的请求.

### 使用socklib.c的timeserv/timeclnt

```c
void talk_with_server(fd) {
    char buf[LEN];
    int n = read(fd, buf, LEN);
    write(1, buf, n);
}

void process_request(fd) {
    time_t now;
    time(&now);

    char* cp = ctime(&now);
    write(fd, cp, strlen(cp));
}
```

服务器调用*time*从内核获得时间, 然后用*ctime*将时间转换为可打印的字符串.
服务器将该字符串写到socket.
客户从socket中读取字符串, 然后写到标准输出中.

这个新的版本遵循了先前版本的程序逻辑, 但是设计更加模块化, 代码更加清晰.

### 第二版的的Server: 使用fork

现在考虑第二版服务器的设计
第二部中程序没有调用*time()*函数来获得代表时间的数据, 而是直接使用了一个shell命令(date命令).


```c
void process_request(fd) {
/** send the date out to the client via fd */
    int pid = fork();
    switch(pid) {
        case -1: return;
        case 0:
            dup2(fd, 1);    // child runs date
            close(fd);      // by redirecting stdout
            execl("/bin/date", "date", NULL);
            oops("execlp");     // or quits
        default:
            wait(NULL);     // parent wait for child
    }

}
```

服务器用*fork*建立一个新的子进程. 该子进程将标准输出重定向到socket, 然后运行*date*.
*date*给出日期, 然后将日期写进标准输出, 这样字符串发送到客户端了.
本程序中调用*wait*. shell通常在调用*fork*后要调用*wait*, 那这里的调用有意义吗?

### 服务器的设计问题: DIY或代理

这里使用两种服务器设计办法:
- Do It Yourself
  服务器接收请求, 自己处理工作
- Delegate
  服务器接受请求, 然后创建一个新进程来处理工作

每种方法有什么优缺点?

- Do It Youself for Quick, Simple Tasks
  计算当前的日期和时间需要系统调用*time*和库函数*ctime*. 使用fork和exec来运行date至少需要
  3个系统调用和创建一个新进程. 对于一些服务器, 效率最高的办法是服务器自己来完成工作并
  且在*listen*中限制连接队列的大小. 
  文件socklib.c中的`make_server_socket_q`函数以队列大小作为参数

- Delegate for Slow, More Complex Tasks
  服务器处理耗时的任何或等待资源时, 需要**委托**(delegate)来完成其工作. 这就像上午中的
  电话接线员, 接收电话, 把连接传递到下一个销售或服务人员, 然后再回去接收下一个电话. 而
  服务器可以使用*fork*创建一个新进程来处理每个请求. 通过这种方式, 服务器可以同时处理多个
  任务.

- Using SIGCHLD for Zombie Prevention
  除了等待child死亡外, parent可以设置为接收表示child死亡的信号. 第八章中解释了当child
  退出或被终止时, 内核发送SIGCHLD给parent. 但它不同于本文讨论的其他信号, 默认时, SIGCHLD
  是被忽略的. parent可以为SIGCHLD设置一个信号处理函数, 它可以调用*wait*. 具体方法如下:

```c
/** native use of SIGCHLD handler with wait() - buggy */

void child_waiter(int), process_request(int);

int main() {
    int sock;
    signal(SIGCHLD, child_waiter);
    if( (sock = make_server_socket(PORTNUM)) == -1 )
        oops("make_server_socket");

    while(1) {
        int fd = accept(sock, NULL, NULL);
        if(fd == -1)
            break;
        process_request(fd);
        close(fd);
    }
}

void child_waiter(int signum) {
    wailt(NULL);
}

void process_request(int fd) {
    if(fork() == 0) {
        dup2(fd, 1);        // move socket to fd1
        close(fd);          // closes socket
        execlp("/bin/date", "date", NULL);
        oops("execlp date");
    }
}

```

下面分析程序的控制流. 当一个请求过来时, parent使用*fork*, 然后parent立即
返回去接收下一个请求, 让child去处理请求. 当child退出时, parent收到SIGCHLD
信号, 跳到信号处理函数调用*wait*. child从进程表中被删除, parent从处理函数中
返回到主函数. 该过程看上去似乎完美, 不过其中存在两个问题.

- 问题1
  程序执行信号处理函数跳转时会中断系统调用*accept*. 当*accpet*被信号中断时, 
  返回-1, 然后设置errno到EINTR. 代码*accept*返回的-1作为错误, 然后从主循环
  中跳出来. 这里需要更改main函数来区分真正的错误和北端的系统调用所产生的
  错误. 

- 问题2
  关于Unix是如何处理多个信号的. 如果多个child几乎同时退出, 将会发生什么?
  假设同时又3个SIGCHLD发送到parent. 最先到达的信号导致parent跳转到信号处理
  函数, 然后parent调用*wait*来保证child已经从进程表中删除. 这样就可以了吗?

  当parent运行信号处理函数时, 其他两个信号到达导致Unix阻塞. 但是并不是不
  缓存信号. 从而, 第二个信号被阻塞, 而第三个信号丢失了. 此时, 如果还有其他
  child退出, 来自于这些child 的信号也将丢失. 信号处理函数只调用了一次*wait*,
  所以每次丢失一个信号意味着少调用一次wait, 这将产生更多的zombie.
  解决方法就是在处理函数中调用*wait*足够多次来去除所有的终止进程. *waitpid*函数
  解决了此问题.

```c
void child_waiter(int signum) {
    while( waitpid(-1, NULL, WNOHANG) > 0 );
}
```

  *waitpid*提供了*wait*函数超集的功能. 其第一个参数表示它要等待的进程ID. 值为-1
  表示等待所有的child. 第二个参数是指向integer的指针, 用来获取状态. 服务器
  并不关心child发生了什么, 不过一个健壮的服务器可能用该信息来跟踪错误.

  *waitpid*最后一个参数表示选项. WNOHANG参数告诉waitpid: 如果没有zombie, 则不必
  等待.

  该循环直到所有退出的child都被停止了才终止. 即使多个child同时退出并产生了多
  个SIGCHLD, 所有的这些信号都会被处理.

## Writing a Web Server

  至此, 已经学习了编写Web服务器的必备知识.
  Web服务器是已经编写的目录服务器的扩展. 主要扩展是一个cat服务器和一个exec服务器.

### What a Web Server Does

一个Web服务器通常要具备3种用户操作:
1. list directories
2. cat files
3. run programs

Web服务器通过基于流的socket连接为客户提供上述3种操作. 用户连接到服务器后, 发生请
求, 然后服务器返回客户请求的信息. 具体过程如下:

| Client | Server |
|:----:|:----:|
| user selects a link -> |  |
| connect to server -> | accept a call |
| write a request | read a request |
|  | handle request: |
|  | directory: list it |
|  | regular file: cat it |
|  | .cgi file: run it |
|  | not exist: error message |
| read the reply | <- write a reply |
| hangup |  |
| display the reply |  |
| html: render it |  |
| image: draw it |  |
| sound: play it |  |
| repeat |  |

### Planing Our Web Server

所要编写的操作如下:

- Set up the server
  使用socklib.c中的`make_server_socket`.

- Accept a call
  使用*accept*来得到指向client的文件描述符. 可以使用*fdopen*使得
  该文件描述符转换成缓冲流.

- Read a request
  什么是一个请求? client如何请求服务? 这还需要进一步学习

- Handle the request
  通过*opendir*和*readdir, open*和*read, dup2*和*exec*的使用实现上述功能.

- Send a reply
  什么是一个reply? client期待接收的又是什么? 这也需要进一步学习.

### The Protocol of a Web Server

  Web Client(通常是一个browser)和Web Server之间的交互主要包含客户的请求
  (request)和服务器的应答(reply). 请求和应答的格式在**hypertext transfer
  protocol**(HTTP)中有定义. HTTP像上一章中的时间服务器, 使用纯文本. 这里
  可以用*telnet*和Web Server进行交互. Web Server在端口80监听. 下面是一个
  实际的例子:

```terminal
unix> telnet www.baidu.com 80
Trying 14.215.177.39...
Connected to www.a.shifen.com.
Escape character is '^]'.
GET /index.html HTTP/1.1    # This line is something you input!
                            # don't foget a blank line!
HTTP/1.1 200 OK
Accept-Ranges: bytes
Cache-Control: no-cache
Connection: keep-alive
Content-Length: 14615
Content-Type: text/html
Date: Sun, 11 Oct 2020 10:07:51 GMT
P3p: CP=" OTI DSP COR IVA OUR IND COM "
P3p: CP=" OTI DSP COR IVA OUR IND COM "
Pragma: no-cache
Server: BWS/1.1 
...

```

- HTTP Request: GET
  *telnet*创建了一个socket并调用*connect*来连接到Web服务器. 服务器接受
  连接请求, 并创建一个基于socket的从client的键盘到Web服务器进程的数据通
  道.
  接下来输入请求: `GET /index.html HTTP/1.0`

  一个HTTP Request包含3个字符串. 第一个是命令, 第二个是参数, 第三个是所
  用协议的版本号.

  HTTP还包含几个其他命令. 大部分Web请求使用GET, 因为大部分时间中用户是单
  击链接来获取网页. GET命令可以跟几行参数. 这里使用了简单的请求, 以一个空
  来表示参数的结束, 并使用与本书前面提及的关于shell的相同约定. 实际上, 一
  个Web Server只是集成了*cat*和*ls*的Unix Shell.

- HTTP Reply: OK
  Server读取请求, 检查请求, 然后返回一个请求. 应答有两部分: Header和Content.
  Header以状态行起始, 如下所示:

```HTTP
HTTP/1.1 200 OK
```

  状态行含有两个或多个字符串.
  - 第一个字符串: 协议的版本
  - 第二个字符串: 返回码, 这里是200, 其文本解释是OK
  
  这里请求的文件叫`/index.html`, 而服务器给出的应答表示可以得到该文件. 如果
  服务器中没有所请求的文件名, 返回码将是404, 其解释是"未找到".

  Header的其余部分是关于应答的附加信息. 在该例子中, 附加信息包含服务器名, 应答
  时间, 服务器所发送的数据类型以及应答的连接类型.

  应答的其余部分是返回的具体内容.

- HTTP小结
  Web Client和Web Server交互的基本结构如下:
  1. Client sends request
      `GET filename HTTP/version`
      optional arguments
      a blank line

  2. Server sends reply
      `HTTP/version status-code status-message`
      additional infomation
      a blank line
      content

  协议的完整描述可以参考RFC文档.
  Web服务器必须接收Clinet的HTTP请求, 并发送HTTP应答. 请求和应答采用纯文本格
  式, 是为了便于使用C中的输入输出以及字符串函数读取和处理.

### Writing a Web Server
  要求Web服务器只支持GET命令, 只接收请求行, 跳过其余参数, 然后处理请求和发送
  应答. 主要循环如下:

```c
while(1) {
    fd = accept(sock, NULL, NULL);  // take a call
    fpin = fdopen(fd, "r");         // make it a FILE*
    fgets(fpin, request, LEN);      // read client request
    read_until_crnl(fpin);          // skip over arguments
    process_rq(request, fd);        // reply to client
    fclose(fpin);                   // hang up connection
}

```

观察Webserv.c

这个程序可以运行, 但不完整, 也不安全, 需要如下改进:
1. zombie的去除;
2. buffer overflow 包含;
3. CGI(Common Gateway Interface)程序需要设置一些环境变量;
4. HTTP头部可以包含更多信息.

### Running the Web Server

编译程序, 在某个端口运行它:

```terminal
banana@ubuntu> gcc webserv.c socklib.c -o ws

banana@ubuntu> ./ws 49152

```

然后用浏览器访问, 网址为`http://ubuntu:49152/webserv.c`, 浏览器可以看到文本内容.
如果有html文件, 浏览器便能解析出更多的内容来.

NOTE: 我的hotname是`ubuntu`, 我在本机的浏览器上输入的hostname便是`ubuntu`. 如果是桥接的网络, 
      那可以用ip. 想知道自己的hostname, 直接在命令行输入`hostname`即可(要按照net-tools).


### Comparing Web Server

  Web Server允许其他机器上的客户得到目录信息, 读取文件和允许程序. 所有的Web Sever
  都要完成这些基本操作, 并且必须遵守HTTP协议.

  那么服务器之间有什么区别呢? 有的服务器容易配置和操作, 有的提供了更多安全特征, 有
  的则快速处理请求和使用较少内存. 其中一个重要的特征就是服务器的效率问题. 服务器可
  以同时处理多少个请求? 对于每个请求, 服务器需要多少系统资源?

  本书的Web服务器对于每个请求都创建新进程来处理. 这是高效的办法吗? 读取文件和目录的
  请求需要较长的时间, 所有服务器没有必要等待这些操作完成, 但是有必要用一个新的进程吗?

  第三章方法可以同时运行多个操作. 程序可以在一个进程中允许多个任务, 这可通过使用线程
  (thread)来实现.
  在后面的章节我们会学习到.

## What's Next?

  电话呼叫模型并不是client和server通信的惟一方式. 有些人通过邮件发送订购请求来买商品.
  使用*message-based*的通信系统, 每个购物者可以一次处理多个商店的购物, 而商店可以同时
  处理多个客户的请求.
  下一章将学习使用明信片模型的网络编程: **Datagram sockets**

---
