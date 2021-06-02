#include "headers.h"

void FCFS()
{

}



int main(int argc, char *argv[])
{
    initClk();
    int totalprocs=0;
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
     while (1){

        struct msgbuff message;
        rec_val = msgrcv(q_id,  &message, sizeof(message.mtext), 5, IPC_NOWAIT);

        if (rec_val != -1){
            totalprocs++;
            procs[i] = message.mtext;
            printf(" i am a new process arrived at : %d \n", procs[i].arrival);
            printf("total live processes : %d\n",totalprocs);
            i++;
        }

        


    }
    
    
    destroyClk(true);
}
