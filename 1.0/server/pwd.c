
/*
 *@author zzj created
 *
 *
 *
 */

#include "thread_pool.h"

void pwdCommand(task_t * task)
{
    
    char* cwd;
    if ((cwd = getcwd( NULL, 0)) == NULL) {
        error( 1, errno, "getcwd failed");
    }
    
    puts(cwd);
    free(cwd);

}

