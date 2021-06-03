#include "headers.h"

void FCFS(int Currprocess,struct process * procs,void* shmaddrr )
{
    int temptime = getClk();
    while(procs[Currprocess].remTime>0)
    {
        char snum[5];
        if(getClk()!=temptime)
        {
            printf(" >>[inside FCFS]---i am process number :%d ----- My Remainig time is: %d \n",procs[Currprocess].id,procs[Currprocess].remTime);
            temptime=getClk();
            procs[Currprocess].remTime--;
        }
       
         sprintf(snum,"%d", procs[Currprocess].remTime);
         strcpy((char*) shmaddrr,snum);
         
    }
}

void CreateNewProcess( struct msgbuff * message)
{
    char str[5];
    int pid = fork();
    if(pid==0)
    {
        sprintf(str,"%d",message->mtext.remTime);
        message->mtext.process_id = getpid();
        int value = execl(realpath("process.o",NULL),"process.o",str, NULL);
        printf("the Forking of process.c returns:  %d and the Remaining time sent as a paramater is%s\n",value,str);
    }

}

void* connectsharedmem()
{
    int shmid = shmget(key_id_sch_proc, 4096, IPC_CREAT | 0644);
     if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    void *shmaddrr = shmat(shmid, (void *)0, 0);
    return shmaddrr;
}


int main(int argc, char *argv[])
{
    initClk();
    int totalprocs=0;
    int CurrProcess=0;
    void *shmaddrr=NULL;    // serves for FCFS

    int algorithm = atoi (argv[1]);
    int argument = -1;
    if (algorithm == 5){
        argument = atoi (argv [2]);
    }
    printf("algorithm %d \n" , algorithm);
    struct process * procs= (struct process*) malloc(1024*sizeof(struct process)) ;

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    
    int ppid = getppid();
    printf("Hey I'm the scheduler  with ppid = %d \n" , ppid);

    int q_key = 22; int rec_val; int i = 0;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);


    if(algorithm==1)
    shmaddrr=connectsharedmem();


     while (1){

        struct msgbuff message;
        rec_val = msgrcv(q_id,  &message, sizeof(message.mtext), 5, IPC_NOWAIT);
       
        if (rec_val != -1)
        {
           CreateNewProcess(&message);
           totalprocs++;
           procs[i] = message.mtext;
           printf("i am process#%d arrived at: %d \n",procs[i].id, procs[i].arrival);
           printf("total alive processes : %d\n",totalprocs);
           i++;

        }

       if(algorithm==1 && CurrProcess<totalprocs)
       {
           FCFS(CurrProcess,procs,shmaddrr);
           CurrProcess++;
       }
        
    }
    
    
    destroyClk(true);
}
