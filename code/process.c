#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere.
    //remainingtime = ??;

   remainingtime= atoi(argv[1]);
   int shmid = shmget(key_id_sch_proc, 4096, IPC_CREAT | 0644);
     if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    void *shmaddr = shmat(shmid, (void *)0, 0);
   
    int dummy = -1;//just for testing

    char snum[5];
    while (remainingtime > 0)
    {
      strcpy(snum,(char*) shmaddr);
      remainingtime = atoi(snum);

      ///////////////////////////////////TESTING///////////////////////////////////////////////
      if(dummy!=remainingtime)
      {
        printf(" >>[Message from process.c] ------------------ My remaining time is: %d\n",remainingtime);
        dummy=remainingtime;
      }
      /////////////////////////////////////////////////////////////////////////////////////////

    }

    int val= shmdt (shmaddr);   // deattach 

    destroyClk(false);

    return 0;
}
