#include "headers.h"
#define MAX_CHARS 3000


struct process *head = NULL;
struct process *current = NULL;
struct process *new = NULL;
int q_id;       //Checkkkk.
void clearResources(int);

int main(int argc, char *argv[])
{
   signal(SIGINT, clearResources);
    // 1. Read the input files:
    int lines = 0;
    char* file_name = "processes.txt";
    FILE *file_line  = fopen(file_name, "r");
  
    char line[MAX_CHARS];
    if (file_line == NULL) 
    {   
              printf("Error! Could not open file\n"); 
              exit(-1); 
    } 
    int ID, ar , ru , pr, k = 0;
    while ( fgets(line, sizeof line, file_line) ){

        if(*line != '#'){
            sscanf(line, "%d  %d  %d   %d",&ID, &ar , &ru , &pr);
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
    


    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list:
    // e.g. ./process_generator.o processes.txt -sch 1

    int algo , argument= -1;
    algo = atoi(argv[3]);

    printf ("%d \n" , algo);
    if (algo == 5) argument = atoi (argv[5]);
    //printf ("%d" , argument);

    // 3. Initiate and create the scheduler and clock processes (as children of the process generator.)
    int pid ,j = 0;
    for (int i = 0 ; i < 2 ; i ++){
        pid = fork();
        if (pid == 0 ) break;
        j++;
    }
    
    if (pid == 0)
    {
        if (j == 0)
        {
            if (algo == 5)
            {
             char* args[] = {"scheduler.o" ,argv[3] , argv[5] , NULL};
               execv(realpath("scheduler.o",NULL),args);
            }
          
            else
            {
                char* args[] = {"scheduler.o" ,argv[3] , NULL};
                execv(realpath("scheduler.o",NULL),args );
            }
        }
        else 
        {
            char* args[] = {"clk.o", NULL};
              execv(realpath("clk.o",NULL),args);
        }
    
    }
    // 4. Use this function after creating the clock process to initialize clock.

    initClk();

    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);

    //Initializing the pipe.
     int q_key = 22;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);
     if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
   current = head; int send_val;
    struct msgbuff message;

    //Sending the process to the scheduler.
    while (true)
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

void clearResources(int signum)                     //Checkkk
{

    msgctl(q_id, IPC_RMID, (struct msqid_ds *)0);
    killpg(getpgrp(), SIGINT);                      //Killing the children.
    free(new);
    exit(0);
    

}



