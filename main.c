#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// 蛇
typedef struct snake {
    int x;
    int y;
    struct snake *next;
} Snake;

// 食物
typedef struct food {
    int x;
    int y;
} Food;

// 窗口
typedef struct border {
    int startx;
    int starty;
    int width;
    int height;
} Border;

// 移动方向
typedef enum direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Dir;

//创建窗口和边框
WINDOW *createWin(Border *border);
//初始化窗口显示
void initWin(WINDOW* win,Snake *snake,Food *food,Border border,Dir dir);
//处理键盘控制
int dealControl(Snake *snake,int *speed,Dir *dir);
//检查是否撞墙
int checkCrashWall(WINDOW *win, Snake *snake, Food *food, Border border, Dir dir);
//检查是否吃到食物
int checkEat(WINDOW *win, Snake *snake, Food *food, Border border, Dir dir);
//更新蛇身信息
void updateSnake(WINDOW *win, Snake *snake, Dir dir);
//刷新蛇身显示
void refreshSnake(WINDOW *win, Snake *snake, Snake *beforeTail);
//更新食物位置信息
void updateFood(WINDOW *win, Snake *snake, Food *food, Border border, Dir dir);
//刷新食物显示
void refreshFood(WINDOW *win, Food *food);
//销毁窗口
void destroyWin(WINDOW *win);


int main() {

    srand((unsigned) time(NULL));

    //ncurses初始化
    initscr();              // 初始化并进入 curses 模式
    cbreak();               // 行缓冲禁止，传递所有控制信息
    noecho();               // 禁止回显
    keypad(stdscr, TRUE);   // 程序需要使用键盘
    curs_set(0);            //隐藏光标

    //程序数据初始化
    int speed=500000;       //刷新间隔（与蛇前进速度成反比）
    Border border;
    Food food;
    Dir dir = RIGHT;        //默认开始向右走
    Snake *snake = (Snake *) malloc(sizeof(Snake)); //带头结点的单链表

    //创建并显示窗口框
    WINDOW *win = createWin(&border);

    //初始化窗口显示
    initWin(win,snake,&food,border,dir);

    // 非阻塞输入
    timeout(0);

    while (1) {

        // 非阻塞处理键盘控制
        int res=dealControl(snake,&speed,&dir);
        if(res==1){         //同向按键无效
            continue;
        } else if (res==0){ //长度超过1后反向按键，自我碰撞
            break;
        }

        //撞墙
        if (checkCrashWall(win, snake, &food, border, dir)) {
            break;
        }

        //未吃到食物，向当前方向前进一格，更新蛇身信息
        if (!checkEat(win, snake, &food, border, dir)) {
            updateSnake(win, snake, dir);
        }

        //程序睡眠(微秒)，实现速度控制
        usleep(speed);
    }

    //销毁窗口
    destroyWin(win);

    //退出curses模式
    endwin();

    return 0;
}


WINDOW *createWin(Border *border) {

    border->height = LINES - 2;                        //窗口高度
    border->width = COLS - 6;                          //窗口宽度
    border->starty = (LINES - border->height) / 2;    // 计算窗口中心位置的行数
    border->startx = (COLS - border->width) / 2;      // 计算窗口中心位置的列数

    //显示游戏名称和操作提示
    char *name = "snake 1.0";
    mvprintw(0, (COLS - strlen(name)) / 2, "%s", name);
    char *message="Press any key continue ...";
    mvprintw(LINES-1,(COLS-strlen(message))/2,message);
    refresh();

    //创建窗口
    WINDOW *local_win;
    local_win = newwin(border->height, border->width, border->starty, border->startx);
    //显示边框
    box(local_win, 0, 0);
    wrefresh(local_win);

    return local_win;
}

void initWin(WINDOW *win,Snake *snake,Food *food,Border border,Dir dir){
    // 初始化蛇头
    Snake *s = (Snake *) malloc(sizeof(Snake));
    s->x = border.width / 2;
    s->y = border.height / 2;
    s->next = NULL;
    snake->next = s;

    //初始化蛇和食物显示
    updateSnake(win,snake,dir);
    updateFood(win, snake, food, border, dir);

    //等待输入任意键开始游戏
    getch();

    //开始后用游戏操作提示覆盖游戏开始提示
    char *help="A:decrease speed  S:pause  D:increase speed";
    mvprintw(LINES-1,(COLS-strlen(help))/2,help);
    refresh();
}

int dealControl(Snake *snake,int *speed,Dir *dir){

    //获取键盘输入
    int ch=getch();

    if (ch!=ERR){   //有键盘输入
        //同向按键无效
        if((ch==KEY_UP && *dir==UP) || (ch==KEY_DOWN && *dir==DOWN) ||
           (ch==KEY_LEFT && *dir==LEFT) || (ch==KEY_RIGHT && *dir==RIGHT)){
            return 1;
        }

        //反向按键，自我碰撞
        if (snake->next->next){
            if((ch==KEY_UP && *dir==DOWN) || (ch==KEY_DOWN && *dir==UP) ||
               (ch==KEY_LEFT && *dir==RIGHT) || (ch==KEY_RIGHT && *dir==LEFT)){
                return 0;
            }
        }

        //加减速，暂停，转向
        switch (ch) {
            case 'd':   //加速
            case 'D':
                if ((*speed)>200000){
                    *speed=(*speed)-200000;
                }
                break;
            case 'a':   //减速
            case 'A':
                if ((*speed)+200000<INT32_MAX){
                    *speed=(*speed)+200000;
                }
                break;
            case 's':   //暂停
            case 'S':
                while (1) {
                    ch = getch();
                    if (ch == 's' || ch == 'S') {
                        break;
                    }
                }
                break;
            case KEY_UP:    //向上
                *dir = UP;  //修改当前运动方向
                break;
            case KEY_DOWN:  //向下
                *dir = DOWN;//修改当前运动方向
                break;
            case KEY_LEFT:  //向左
                *dir = LEFT;//修改当前运动方向
                break;
            case KEY_RIGHT: //向右
                *dir = RIGHT;//修改当前运动方向
                break;
            default:        //其他无效按键跳过
                return 1;
        }
    }

    return 2;
}

int checkCrashWall(WINDOW *win, Snake *snake, Food *food, Border border, Dir dir) {
    //按当前运动方向蛇头即将到达的下一个坐标
    int headx = snake->next->x, heady = snake->next->y;
    switch (dir) {
        case UP:
            heady--;
            break;
        case DOWN:
            heady++;
            break;
        case LEFT:
            headx--;
            break;
        case RIGHT:
            headx++;
            break;
    }

    //撞墙
    if (headx <= 0 || headx >= border.width-1 || heady <= 0 || heady >= border.height-1) {
        //显示撞墙结果
        updateSnake(win,snake,dir);
        return 1;
    }

    //撞自己(身体)
    Snake *s=snake->next->next;
    while (s){
        if (s->x==headx && s->y==heady){
            return 1;
        }
        s=s->next;
    }

    return 0;
}

int checkEat(WINDOW *win, Snake *snake, Food *food, Border border, Dir dir) {
    //按当前运动方向蛇头即将到达的下一个坐标
    int headx = snake->next->x, heady = snake->next->y;
    switch (dir) {
        case UP:
            heady--;
            break;
        case DOWN:
            heady++;
            break;
        case LEFT:
            headx--;
            break;
        case RIGHT:
            headx++;
            break;
    }

    //吃到食物
    if (headx == food->x && heady == food->y) {
        //食物位置给蛇头
        Snake *s = (Snake *) malloc(sizeof(Snake));
        s->x = food->x;
        s->y = food->y;
        s->next = snake->next;
        snake->next = s;

        //更新蛇显示
        refreshSnake(win, snake, NULL);

        //更新食物信息
        updateFood(win, snake, food, border, dir);

        return 1;
    }

    return 0;
}

void updateSnake(WINDOW *win, Snake *snake, Dir dir) {

    //先只增加蛇头，等显示完蛇后（为显示函数提供蛇尾坐标，便于隐藏蛇尾），再删除蛇尾，即可实现蛇的前进

    // 插入新蛇头
    Snake *s = (Snake *) malloc(sizeof(Snake));
    s->x = snake->next->x;
    s->y = snake->next->y;
    switch (dir) {
        case UP:
            s->y--;
            break;
        case DOWN:
            s->y++;
            break;
        case LEFT:
            s->x--;
            break;;
        case RIGHT:
            s->x++;
            break;
    }
    s->next = snake->next;
    snake->next = s;

    //找蛇尾
    while (s->next->next) {
        s = s->next;
    }

    //刷新蛇身显示
    refreshSnake(win, snake, s->next);

    //刷新显示后，删除蛇尾！！！
    s->next = NULL;

}

void refreshSnake(WINDOW *win, Snake *snake, Snake *beforeTail) {

    //蛇的显示：每次只需要显示新蛇头位置和隐藏旧蛇尾，蛇中间沿用之前的显示

    //新蛇头显示
    Snake *s = snake->next;
    mvwaddch(win, s->y, s->x, '#');

    //旧蛇尾隐藏
    if (beforeTail) {
        mvwaddch(win, beforeTail->y, beforeTail->x, ' ');
    }

    wrefresh(win);

}

void updateFood(WINDOW *win, Snake *snake, Food *food, Border border, Dir dir) {

    //标志食物与蛇重合
    int flag = 0;
    do {
        //随机生成食物
        food->x = rand() % (border.width - 2) + 1;
        food->y = rand() % (border.height - 2) + 1;

        //判断是否与蛇重合
        Snake *s = snake->next;
        while (s) {
            if (s->x == food->x && s->y == food->y) {   //重合重新生成
                flag = 1;
                break;
            }
            s = s->next;
        }
    } while (flag);

    //更新食物显示
    refreshFood(win,food);

}

void refreshFood(WINDOW *win, Food *food) {
    //显示食物
    mvwaddch(win, food->y, food->x, '*');
    wrefresh(win);
}


void destroyWin(WINDOW *win){
    //显示退出游戏提示
    char *crash="crash !";
    char *quit="quiting...";

    attron(A_BOLD); //粗体文字
    mvprintw(LINES/2,(COLS-strlen(crash))/2,crash);
    mvprintw(LINES/2+1,(COLS-strlen(quit))/2,quit);
    attron(A_BOLD); //粗体文字
    refresh();

    //延时
    sleep(2);
    //销毁窗口
    delwin(win);
}