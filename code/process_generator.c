#include "headers.h"
#define MAX_CHARS 100


struct process *head = NULL;
struct process *current = NULL;
struct process *new = NULL;

void clearResources(int);

int main(int argc, char *argv[])
{
   // signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    int lines = 0;
    char* file_name = "processes.txt";

    FILE *file_line  = fopen(file_name, "r");
    /*for (char c = getc(file_line); c != EOF; c = getc(file_line))
        if (c == '\n') lines ++;*/

    char line[MAX_CHARS];
    if (file_line == NULL) {   
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
            new -> remTime = ru;
            new -> priority = pr;
            if (k == 0){
                head = new;
            }
            else{
                current ->next = new;
            }
            current = new;
            k++;
        }
       /* else {
            //fscanf(file_line, "%*[^\n]");
            printf("hoho\n");
        }*/
    }
    
    //fscanf(inputFile, "%*[^\n]"); // Read and discard a line
  //  printf("%d \n" , head->next->next->runtime);
    //scanf("%d" , &ru);


    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    ////////////  ./process_generator.o testcase.txt -sch 5 -q 2,
    int algo , argument= -1;
    algo = atoi(argv[3]);
    printf ("%d \n" , algo);
    if (algo == 5) argument = atoi (argv[5]);
    //printf ("%d" , argument);

    // 3. Initiate and create the scheduler and clock processes.
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
               execl(realpath("scheduler.o",NULL),"scheduler.o" ,argv[3] , argv[5] , NULL);
            else
                execl(realpath("scheduler.o",NULL),"scheduler.o",argv[3] , NULL);
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
    // TODO Generation Main Loop
    //pid = getpid();
     //printf ("I'm %d \n" , pid);
     int q_key = 22;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);
     if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
   current = head; int send_val;
    struct msgbuff message;

    while (1){
        if (current){
            if (current->arrival == getClk()){
                struct msgbuff message;
                message.mtext = *current;
                //strcpy(message.mtext, "hhhhhhhhh");
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

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}



