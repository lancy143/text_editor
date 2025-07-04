# 构建自己的文本编辑器 - 编程考题
## Level 0: 环境设置与基础
参考：
1. Makefile
  1. https://blog.csdn.net/newcong0123/article/details/51865671
makefile
  - -Wall 代表"所有警告", 并让编译器在看到程序中的代码时向你发出警告, 这些代码在技术上可能没有错误, 但被认为是 C 语言的错误或有问题的用法, 例如在初始化变量之前使用变量.
  - -Wextra 和 -pedantic 会打开更多警告. 对于本教程中的每个步骤, 如果你的程序能够编译通过, 除了在某些情况下出现"未使用的变量"警告外, 它不应产生任何警告. 如果你收到任何其他警告, 请检查以确保你的代码与该步骤中的代码完全匹配.
  - -std=c99 指定我们正在使用的 C 语言标准的确切版本, 即 C99. C99 允许我们在函数内的任何地方声明变量, 而 ANSI C 要求所有变量都在函数或块的顶部声明.
根据文件最终情况还需补充内容.

## Level 1: 终端原始模式
任务
目标: 修改终端设置，实现原始模式以读取用户按键。
任务:
1. 实现原始模式设置（disableRawMode和enableRawMode函数）
2. 读取用户输入，直到用户按下'q'退出
3. 实现错误处理机制
tcsetattr(), tcgetattr(), 和 read() 在失败时都会返回 -1 , 并设置 errno 值来指示错误.
4. 显示按键的ASCII值，包括控制字符
要求:
- 禁用回显模式（ECHO）显示正在执行的 批处理 命令及执行的结果等，位置是显示在 屏幕 上，禁用就是类似于输入密码的样子
ECHO 是一个 bitflag, 二进制表示是: 00000000000000000000000000001000. 我们对该值使用按位非运算符(~)来获取 11111111111111111111111111110111. 然后我们将该值与标志位字段按位与, 这会强制标志位字段中的第四位变为 0, 并导致每隔一位保留其当前值. 

- 禁用规范模式（ICANON）需要用户输入了回车，才会去执行这一行命令。
与上述同理，实现在键入q的瞬间退出

- 禁用信号处理（ISIG, IXON等）
1. 默认情况下, Ctrl-C 会向当前进程发送 SIGINT 信号使其终止, 并向当前进程 Ctrl-Z 发送 SIGTSTP 信号使其挂起. 关闭信号处理可以使得在键入Ctrl-C时不直接退出程序
2. XON 来自两个控制字符 Ctrl-S 和 Ctrl-Q 产生的名字: XOFF 负责暂停传输, XON 负责重启传输.<----Ctrl-S Ctrl-Q 

- 禁用各种终端处理特性（BRKINT, INPCK, ISTRIP等）
- 设置适当的输入超时（VMIN和VTIME）

- 确保在程序退出时恢复终端原始设置
记住初始的终端状态

- 实现错误处理机制
tcsetattr(), tcgetattr(), 和 read() 在失败时都会返回 -1 , 并设置 errno 值来指示错误.

参考：
1. termios：https://softool.cn/blog-158.html
2. errno表示错误代码。 记录系统的最后一次错误代码。代码是一个int型的值，在errno.h中定义。系统每一次出错都会对应一个出错代码，例如12表示“Cannot allocate memory"。
3. stderr是linux(unix)标准出错输出。linux中的一个进程启动时，都会打开三个文件：标准输入、标准输出和标准出错处理。通常这三个文件都与终端联系。这三个文件分别对应文件描述符0、1、2。系队统自定义了三个文件指针stdin、stdout、stderr，分别指向标准输入、标准输出和标准出错输出。通常结合fprintf使用：fprintf(stderr,"error message")。
4. perror是错误输出函数，在标准输出设备上输出一个错误信息。是对errno的封装。例如perror("fun"),其输出为：fun：后面跟着错误信息(加一个换行符)。包含头文件stdio.h.
5. stderror是通过参数errno，返回错误信息。即stderror(errno)，可用printf函数打印出错信息，用于调试。包含头文件string.h。
问题：
1. 输入q即退出,那怎么输入q呢()
2. Ctrl-M 无响应 ---->与vscode的快捷键冲突,取消此快捷键即可
3. Ctrl-V 实现粘贴功能 ---> 
4. 实现Ctrl-Q退出,发现键与vscode的快捷键冲突
5. 回车回退无法使用
Level 2: 原始输入和输出处理
任务
目标: 处理终端的原始输入和输出，为文本编辑功能做准备。
任务:
1. 实现处理特殊键（箭头键、Page Up/Down、Home、End等）的功能
  1. 箭头 这些字节采用转义序列的形式, 以 '\x1b', '[' 开头, 后面跟着 'A', 'B' 或 'C', 'D'
  2. Page Up/Down 
2. 实现屏幕清除和光标定位功能 
3. 添加基本的文本显示功能
4. 实现滚动和刷新机制
Level 2 演示视频
实现
1. 获取终端界面大小以实现光标相关控制以及初始化界面等效果 : 调用 ioctl() 并传入参数 TIOCGWINSZ 来获取终端的大小. 
2. 转义序列 Everything you never wanted to know about ANSI escape codes 据此来找到那些特殊键在键入后获取的信息.
3. 屏幕清除应在初始\出错和退出时都能引用
4. 光标移动:E.cx 是光标(列)的水平坐标, E.cy 是垂直坐标(行).
enum editorKey {
  ARROW_LEFT = 200,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};
给超出 char 范围的大整数值, 这样它们就不会与任何普通按键发生冲突. 
5. Page Up 和 Page Down : 按下 Page Up 发送 <esc>[5~, 按下 Page Down 发送 <esc>[6~

Level 3: 实现文本查看器
状态栏的设计体现了信息反馈的用户体验原则。用户需要知道当前的状态：在哪个文件、第几行、文件是否被修改等。这些信息有助于用户建立心理模型。
任务
目标:实现一个简单的文本查看器，能够显示文本文件的内容，并具备基本的滚动、光标移动和状态显示功能。
任务:
1. 行查看器实现
- 定义数据结构erow，用于存储每行文本的信息，包括文本内容和长度。
- 在编辑器配置结构体中补充相关字段，如当前行、列位置等。
- 初始化文本编辑器后，能够在编辑器中显示一行文字"Hello, World!"。
- 通过命令行参数指定文本路径，从文件中读取文本并显示在编辑器中。当无命令行参数时，显示"Hello, World!"，否则显示文件内容。
2. 多行查看器实现
- 能够读取并显示多行文本。
- 处理横向和纵向超出屏幕的文本显示，
- 允许光标在行头按左键移动到上一行行尾，以及在行尾按右键移动到下一行行首。
- 实现Page Up和Page Down键的滚动功能，以及End键将光标移动到行尾的功能。
- 处理制表符（Tab）的显示，将其渲染为多个空格，以确保文本对齐。
3. 状态栏实现
- 在屏幕底部添加状态栏，显示文件名（最多20字符，超出显示"No Name"）、文件行数和当前行号。
- 在状态栏下方增加一行，用于后续实现文本搜索功能（如Ctrl- F搜索）。
- 使用转义序列和m命令实现状态栏的文本渲染。
实现思路
1. 行查看器
typedef struct erow { 
  int size;//大小
  char *chars; //每行字符
} erow;

struct editorConfig {
    int cx, cy; //光标的xy
    int screenrows; //行
    int screencols; //列
    int numrows; /* Number of rows */
    int rowoff; //行偏移量
    int coloff; //列偏移量
    char *filename; //文件名
    erow *row;
    struct termios orig_termios;
};
2. 初始界面显示 : 逐行输出,先获取界面行数左侧输出~,行数中间打印"Hello World"
3. 多行查看器实现:从单行到多行获取 , 初始效果只能显示出终端行数大小的文本,后加入滚动(滚动思路即把光标始终显示在看得见的区域)
4. 状态栏显示 使用反转颜色易于观察,加入文件名和文件总行数显示.


参考内容与补充知识
1. m 命令（选择图形渲染）会导致它之后的文本以各种可能的属性打印，包括粗体（ 1 ）、下划线（ 4 ）、闪烁（ 5 ）和反转颜色（ 7 ）。例如，你可以使用命令 <esc>[1;4;5;7m 指定所有这些属性。参数 0 清除所有属性，是默认参数，所以我们使用 <esc>[m 回到正常文本格式


