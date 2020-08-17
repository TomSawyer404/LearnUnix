# 01-文件IO

我们编写3个程序来初步学习*文件IO*
1. cat
2. grep
3. more

前两个程序参考[ostep-project](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/initial-utilities)

## cat
第一个程序很简单，打印所有读到的数据。程序的逻辑步骤如下：

```pseudocode
打开一个文件;
读取文件的信息到内存缓冲区;
把内存缓冲区的信息输出到stdout;

```

实现细节参考《操作系统：三大简易元素》的项目实践:
- Your program wcat can be invoked with one or more files on the command line; it should just print out each file in turn.
- In all non-error cases, wcat should exit with status code 0, usually by returning a 0 from main() (or by calling exit(0)).
- If no files are specified on the command line, wcat should just exit and return 0. Note that this is slightly different than the behavior of normal UNIX cat (if you'd like to, figure out the difference).
- If the program tries to fopen() a file and fails, it should print the exact message "wcat: cannot open file" (followed by a newline) and exit with status code 1. If multiple files are specified on the command line, the files should be printed out in order until the end of the file list is reached or an error opening a file is reached (at which point the error message is printed and wcat exits).


## grep
该程序模拟linux自带的`grep`。

程序的逻辑如下：
```pseudocode

if 参数 >= 3:
    打开文件;
    while 读取文件信息到缓冲区 == True:
        if 查找字符串 == True:
            打印结果;
else if 参数 == 2:
    从标准输入读取信息到缓冲区;
    查找字符串;
else:
    print("wgrep: searchterm [file ...]");

```

实现细节参考《操作系统：三大简易元素》的项目实践:
- Your program wgrep is always passed a search term and zero or more files to grep through (thus, more than one is possible). It should go through each line and see if the search term is in it; if so, the line should be printed, and if not, the line should be skipped.
- The matching is case sensitive. Thus, if searching for foo, lines with Foo will not match.
- Lines can be arbitrarily long (that is, you may see many many characters before you encounter a newline character, \n). wgrep should work as expected even with very long lines. For this, you might want to look into the getline() library call (instead of fgets()), or roll your own.
- If wgrep is passed no command-line arguments, it should print "wgrep: searchterm [file ...]" (followed by a newline) and exit with status 1.
- If wgrep encounters a file that it cannot open, it should print "wgrep: cannot open file" (followed by a newline) and exit with status 1.
- In all other cases, wgrep should exit with return code 0.
- If a search term, but no file, is specified, wgrep should work, but instead of reading from a file, wgrep should read from standard input. Doing so is easy, because the file stream stdin is already open; you can use fgets() (or similar routines) to read from it.
- For simplicity, if passed the empty string as a search string, wgrep can either match NO lines or match ALL lines, both are acceptable.


## more
我们的*more*应当实现以下三种方式的输入：

```bash
unix> more filename.txt

unix> cat filename.txt | more

unix> more < filename.txt

```

程序的逻辑伪代码如下:
```pseudocode
显示24行;
显示[more?];

if input() == '\n':
    显示下一行;
else if input() == '空格':
    显示24行;
else if input() == 'q':
    exit();

```
我们写出两个版本的*more*

- `more01.c`: 
  - 完成了基本功能.
  - 出现的问题：
    1. 命令`unix> ls /bin | more01` 的执行结果和预期不一样
    2. 命令`unix> ./more01 < more01.c` 的执行结果和预期不一样
- `more02.c`: 
  修复了`more01`的2个问题.
  - Key Point:
    `/dev/tty`是键盘和显示器的设备描述符, 向这个文件写相当于显示在用户的屏幕上，读相当于从键盘获取用户输入.

