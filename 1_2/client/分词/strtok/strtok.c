#include <stdio.h>
#include <string.h>

int main()
{
    char str[] = "hello world good morning";
    char* token = strtok(str, " ");
    while(token != NULL){
        printf("%s\n", token);
        token = strtok(NULL, " ");
    }
    return 0;
}

