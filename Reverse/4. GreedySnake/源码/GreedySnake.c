#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
####################
#S*###  *#         #
##     ### ####### #
###### ### #  * ## #
#     *##* # *  ## #
# *### ##*##       #
###### ## ##########
#*  ## *           #
#*     ########*   #
####################
*/
char map[] = "#####################S*###  *#         ###     ### ####### ####### ### #  * ## ##     *##* # *  ## ## *### ##*##       ####### ## ###########*  ## *           ##*     ########*   #####################";
char *m;
char steps[1000] = {0};
char *sp = steps;
int ciper[] = {0x49,0x0,0x8,0x5,0x4d,0x78,0x66,0x33,0x50,0x5f,0x9,0x36,0x9,0x45,0x48,0x49,0x27,0xc,0x42,0x72,0x71,0x0,0x43,0x16,0x12};
char key[] = "/lib64/ld-linux-x86-64.so.2";
typedef struct position
{
    int x;
    int y;
} pos;

typedef struct QueueNode
{
    pos position;
    struct QueueNode *next;
} queueNode;

typedef struct Queue
{
    queueNode *front;
    queueNode *rear;
    int len;
} queue;

void initQueue(queue *q, pos p)
{
    queueNode *node = (queueNode *)malloc(sizeof(queueNode));
    node->position = p;
    node->next = 0;
    q->front = node;
    q->rear = node;
}

void push(queue *q, pos p)
{
    queueNode *node = (queueNode *)malloc(sizeof(queueNode));
    node->position = p;
    node->next = 0;
    q->front->next = node;
    q->front = q->front->next;
    q->len++;
}

pos pop(queue *q)
{
    queueNode *node = q->rear;
    q->rear = q->rear->next;
    pos popNode = node->position;
    free(node);
    q->len--;
    return popNode;
}

struct Snake
{
    pos head;
    queue body;
} snake;

void initSnake()
{
    m = map;
    snake.head.x = 1;
    snake.head.y = 1;
    initQueue(&snake.body, snake.head);
}

int gameover()
{
    printf("oh no!");
    return 114514;
}

void addSteps(char cmd)
{
    *sp = cmd;
    sp++;
}

// void printMap(){
//     system("clear");
//     for(int i = 0;i <10;i++){
//         for(int j = 0;j < 20;j++){
//             printf("%c",map[i*20+j]);
//         }
//         printf("\n");
//     }
// }

int __attribute__((optimize("O0"))) step(char cmd)
{
    pos nextPos = snake.head;
    switch (cmd)
    {
    case 'a':
        nextPos.x--;
        break;
    case 's':
        nextPos.y++;
        break;
    case 'd':
        nextPos.x++;
        break;
    case 'w':
        nextPos.y--;
        break;
    default:
        printf("\n%c\n", cmd);
        return gameover();
    }
    addSteps(cmd);
    char nextSym = m[nextPos.x + nextPos.y * 20];
    if (nextSym != '*')
    {
        pos popNode = pop(&snake.body);
        m[popNode.x + popNode.y * 20] = ' ';
    }
    nextSym = m[nextPos.x + nextPos.y * 20];
    if (nextSym == '#' || nextSym == 'S')
    {
        // printf("%d %d",snake.head.x,snake.head.y);
        // printf("u dead\n");
        return gameover();
    }
    else
    {
        m[nextPos.x + nextPos.y * 20] = 'S';
        snake.head = nextPos;
        push(&snake.body, nextPos);
    }
    // printMap();
    // printf("%s\n",steps);
    for (int i = 0; i < strlen(m); i++)
    {
        if (m[i] == '*')
            return 0;
    }
    return strlen(steps);
}

void printFlag()
{
    __asm__ __volatile__(
        "jz label1;"
        "jnz label1;"
        "nop;"
        "label1:;");

    for (int i = 0; i < 25; i++)
    {
        __asm__ __volatile__(
            "jz label2;"
            "jnz label2;"
            "nop;"
            "label2:;");
        printf("%c",((ciper[i] ^ key[i])+255)%255);
    }
    return;
}

int main()
{
    int result;
    initSnake();
    // printMap();
    while (1)
    {
        char cmd;
        scanf("%c", &cmd);
        if (cmd == 10)
            continue;
        result = step(cmd);
        if (result)
            break;
    }
    // printf("%d",result);
    if (result <= 127)
    {
        printFlag();
    }
    return 0;
}

// dsddddssaaaasawdddddssssaaawaasdddddwdddddddddsdddwaaaaaaaaawwwdwwwddddddddssssaaawwasawassddddddwwwwaaaaaaaasssasssaaawwwwwwdd