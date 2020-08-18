# cp程序

## cp的用法
```bash
unix> cp srcFile dstFile
```
如果`dstFile`文件不存在, cp程序就创建这个文件l; 如果存在, cp程序就覆盖它.
cp程序确保`srcFile`和`dstFile`的内容是一样的.

## 需要用到的系统调用
1. `create()`: 包含在`<fcntl.h>`下
2. `write()`: 包含着`<unistd.h>`下

## cp的程序逻辑
```pseudocode
open sourceFile for reading;
open copyFile for writing;
while (buffer = read(sourceFile)) != EOF:
    write from buffer to copyFile;
close sourceFile;
close copyFile;
```

---
