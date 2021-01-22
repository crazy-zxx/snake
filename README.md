# Snake
贪吃蛇（基于ncurses库）

## Ncurses安装
在MAC上无法使用windows的conio.h库,所以使用了Unix下的Ncurses库,在MAC上安装包肯定要使用homebrew了:
```
brew install ncurses 
```

安装完成之后直接在C程序头写上:

```
#include <ncurses.h> 
```

当编译C的时候使用下述的格式:
```
gcc <program file> -lncurses 
```

必须加入编译参数: `-lncurses`

## Ncurses使用文档
[点击下载](https://files.cnblogs.com/files/lishuaicq/ncurses-cn-2nd-pdf.zip)
