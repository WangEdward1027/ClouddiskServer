#include <stdio.h>
#include <string.h>

void test1(){
    char str[] = "hello world good morning";
    char* token = strtok(str, " ");
    while(token != NULL){
        printf("%s\n", token);
        token = strtok(NULL, " ");
    }
}

void test2(){
    char str[] = "hello world good morning";
    char* token = strtok(str, " ");
    char buff[1000];
    strcpy(buff, strtok(NULL, ""));
    printf("%s\n", buff);
}

int main()
{
    /* test1(); */
    test2();
    return 0;
}
