#include "headers.h"
#define MAX_CHARS 1000


struct process *head = NULL;
struct process *current = NULL;
struct process *new = NULL;

int q_id;

void clearResources(int);

int main(int argc, char *argv[])
{

   signal(SIGINT, clearResources);
  
    int lines = 0;
    char* file_name = "processes.txt";
    FILE *file_line  = fopen(file_name, "r");


    char line[MAX_CHARS*300];
    if (file_line == NULL) 
    {   
              printf("Error! Could not open file\n"); 
              exit(-1); 
    } 
    int ID, ar , ru , pr, k ,mem;
    while ( fgets(line, sizeof line, file_line) ){

        if(*line != '#'){
            sscanf(line, "%d  %d  %d   %d  %d",&ID, &ar , &ru , &pr,&mem);
            
 
           // printf("%d \n" , ID);
            new = (struct process*) malloc(sizeof(struct process));
            new -> id = ID;
            new -> arrival = ar;
            new -> runtime = ru;
            new -> priority = pr;
            new -> remTime=ru;
            new ->execTime=0;
            new ->startingTime=-1;
            new -> wait =0;
            new -> status =EXPECTED;            //There are #define for every state in headers.h
            new->memorySize=mem;
             

            if (k == 0){
                head = new;
            }
            else{
                current ->next = new;
            }

            current = new;
            k++;
        }
   
    }
    
    int memoryalgo=0;

    int algo , argument= -1;
    algo = atoi(argv[3]);

    printf ("%d \n" , algo);
    if (algo == 5) 
    {
        argument = atoi (argv[5]);
        memoryalgo= atoi(argv[7]);
    }
    else
        memoryalgo=atoi(argv[5]);


    // 3. Initiate and create the scheduler and clock processes (as children of the process generator.)
    int pid ,j = 0;
    for (int i = 0 ; i < 2 ; i ++){
        pid = fork();
        if (pid == 0 ) break;
        j++;
    }
    
    //pid = fork();
    if (pid == 0){
        if (j == 0){
            if (algo == 5)
               execl(realpath("scheduler.o",NULL),"scheduler.o" ,argv[3] , argv[5] ,argv[7], NULL);
            else
                execl(realpath("scheduler.o",NULL),"scheduler.o",argv[3] ,argv[5], NULL);
        }
        else {
              execl(realpath("clk.o",NULL),"clk.o",  NULL);
        }
    
    }
    // 4. Use this function after creating the clock process to initialize clock.

    initClk();

    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
 
     int q_key = 22;
     q_id = msgget(q_key, 0666 | IPC_CREAT);
     if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
   current = head; int send_val;
    struct msgbuff message;

    while (1)
    {
        if (current){
            if (current->arrival == getClk()){
                struct msgbuff message;
                message.mtext = *current;

                message.mtype = 5;
                send_val = msgsnd(q_id,  &message, sizeof(message.mtext), !IPC_NOWAIT);
                if (send_val == -1)
                    perror("Errror in send");
                else current = current->next;
            }
        }
        else {
            
        }
    }
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)                              //Checkkk
{

    msgctl(q_id, IPC_RMID, (struct msqid_ds *)0);
    killpg(getpgrp(), SIGINT);                              //Killing the children.
    free(new);
    exit(0);
    

}



