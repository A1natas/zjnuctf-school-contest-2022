#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void trick(int &a){
    char flag[] = "flag{1_am_useless}";
    if(a % 2)
        a += flag[2] - 'a' + 1;
    else
        a += flag[12] - 'a' - 3;
}

void convert(char temp[],char p[],int &cnt){
    //printf("%c",temp[7]);
    int pos = 0;
    while(p[cnt] && temp[7] == '}'){
        //printf("%d\n",cnt);
        //printf("%c",p[cnt]);
        if(p[cnt] == 'H'){
            temp[pos] = '0';
            pos++;
            trick(cnt);
        }
        else if(p[cnt] == 'T'){
            temp[pos] = '1';
            pos ++;
            trick(cnt);
        }
        else
            trick(cnt);
    }
    
}

int str2num(char a[]){
    int temp[9]={-1,-1,-1,-1,-1,-1,-1,-1};
    int cnt = 0;
    int ch=0;

    //printf("%s\n",a);
    while(a[cnt]){
        if(a[cnt] == '0'){
            temp[cnt] = 0;
            cnt ++;
        } 
        else{
            temp[cnt] = 1;
            cnt ++;
        }
    }

    for(int i=0;i<8;i++){
        if(temp[i] != -1){
            ch += temp[i];
            ch *= 2;
        }
        else{
            break;
        }
    }
    ch/=2;

    return ch;
}



int main(){
   // printf("shit");

    FILE *fp;
    char ch;
    fp = fopen("shit_code","r");
    char p[300] = {0};
    fscanf(fp,"%s",&p);
    //int cnt  = 0;
    //printf("%d\n",sizeof(p));
  /*  while(p[cnt]){
        printf("%c\n",p[cnt]);
        cnt ++;
    }  */

    int cnt = 0,pos = 0;
    char input[100];
    printf("Please input your flag:  ");
    scanf("%s",&input);
    //printf("%c\n",input[2]);
    while(p[cnt]){
        char temp[9] = "flag{hh}";
        convert(temp,p,cnt);
        //printf("%c\n",str2num(temp));
        if(str2num(temp) != input[pos]){
            printf("Wrong wrong!!\n");
            exit(0);
        }
        pos++;
    }
    printf("Right right!!\n");
}