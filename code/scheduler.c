#include "headers.h"
#include "minHeap.h"
void FCFS(int Currprocess,struct process* ,void*  );
void CreateNewProcess( struct msgbuff *);
void * connectsharedmem();

int main(int argc, char *argv[])
{
    initClk();
    //To keep track of processes:
    int totalprocs=0;
    int CurrProcess=0;
    void *shmaddrr=NULL;    // Serves for FCFS

    //argv[1] will be the passed algorithm.
    int algorithm = atoi (argv[1]);
    int argument = -1;
    //In case that's Round-Robin then the next parameter will be the quantum.
    if (algorithm == 5)
    {
        argument = atoi (argv [2]);
    }
    printf("algorithm %d \n" , algorithm);

    //TODO: upon termination release the clock resources.
    //Debugging:
    int ppid = getppid();
    printf("Hey I'm the scheduler  with ppid = %d \n" , ppid);

    //The scheduler should receive processes from the process generator through a message queue.
    int q_key = 22; int rec_val; int i = 0;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);
    if(algorithm==1)    shmaddrr=connectsharedmem();

    //We'll be receiving processes in either a minheap or an array depending on the algorithm.
    struct process * procs;
    minHeap HP;                                     //Only one of these will be initialized and used depending on the algorithm.
    if(algorithm==1 || algorithm==5)
    {
       procs= (struct process*) malloc(1024*sizeof(struct process)) ;
    }
    else
    {
        switch(algorithm)
        {
            case 2:
                HP=initMinHeap(1024,2);
            case 3:
                HP=initMinHeap(1024,3);
            case 4:
                HP=initMinHeap(1024,4);
        }

        
    }

    //Time to receive:
     while (true)
     {

        struct msgbuff message;
        rec_val = msgrcv(q_id,  &message, sizeof(message.mtext), 5, IPC_NOWAIT);
       //If there's a process in the pipe:
        if (rec_val != -1)
        {
                CreateNewProcess(&message);
                totalprocs++;
                if(algorithm==1 || algorithm==5)
                    procs[i] = message.mtext;
                else
                    insertNode(&HP, message.mtext);
                printf("i am process#%d arrived at: %d \n",message.mtext.id, message.mtext.arrival);
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

int* ShortestJobFirst(struct process * P)
{
    
}


void FCFS(int Currprocess,struct process * procs, void* shmaddrr )
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
            procs[Currprocess].execTime++;

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
