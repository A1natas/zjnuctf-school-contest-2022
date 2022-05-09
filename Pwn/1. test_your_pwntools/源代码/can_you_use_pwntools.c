#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

char symbols[4] = {'+', '-', 'x', '/'};

void welcome() {
    printf("Welcome to ZJNU CTF, try this game ?!\n");
    printf("Can you do 100 math calculations in 30 seconds ?\n");
    printf("Let's have a try!\n");
}

int readFile(const char *fname, char *buffer, int bufferLen) {
	int retLen = 0;
	FILE *f = fopen(fname, "r");
	if (f) {
		retLen= fread(buffer, 1, bufferLen, f);
		buffer[retLen] = 0;
		fclose(f);
	}
	if (retLen <= 0){ 
		printf("Read flag error! Try once more!\n");
		return 0;
	}
	return retLen;
}

void win() {
    printf("You win! Here is flag!\n");
    system("cat /flag.txt");
}

void fail() {
    printf("You lose! Try a little faster!\n");
}

int init(){
    signal(SIGALRM,fail);
    fflush(stdin);
    fflush(stdout);
    fflush(stderr);
	setvbuf(stdout,0,2,0);
    setvbuf(stdin,0,_IONBF,0);
    setvbuf(stdin,0,_IONBF,0);
    setvbuf(stderr,0,_IONBF,0);
    return alarm(30);
}

int match() {
    int input = 0;
    int res = 0;
    int flag = 100;
    for (int i = 0; i < 100; i++) {
        srand((unsigned int)time(NULL) * (i + 114));
        int random1 = rand() % 1919 + 1;

        srand((unsigned int)time(NULL) * (i + 514));
        int random2 = rand() % 810 + 1;

        srand((unsigned int)time(NULL) * (i + 1919810));
        char symbol = symbols[rand() % 3];
        printf("Here is symbol --> %c\n", symbol);
        printf("Here are nums --> num1 = %d num2 = %d \n", random1, random2);
        printf("So what's the answer? %d %c %d = what?\n",random1, symbol, random2);

        scanf("%d", &input);

        switch(symbol) {
            case '+':
            res = random1 + random2;
            break;
            case '-':
            res = random1 - random2;
            break;
            case 'x':
            res = random1 * random2;
            break;
            case '/':
            res = random1 / random2;
        }
        if (res == input) {
            printf("Right! Next! \n\n");
            flag--;
        } else {
            fail();
            return 0;
        }
    }
    if (!flag) {
        win();
    } else {
        fail();
    }
}

int main() {

    init();
    welcome();
    match();

    return 0;
}