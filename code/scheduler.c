#include "headers.h"
#include "minHeap.h"
void FCFS(int Currprocess, struct process *, void *);
void CreateNewProcess(struct msgbuff *);
void *connectsharedmem();
void ShortestJobFirst(minHeap *, void *);            //This is non-preemptive, simply choose the process with lowest run time.
boolean hasPassed(int t, int previousTime, int now); //t seconds or more have passed from previousTime to now.
boolean processRunning = 0;
struct process hasTurn;
int main(int argc, char *argv[])
{
    initClk();
    //To keep track of processes:
    int totalprocs = 0;
    int CurrProcess = 0;
    void *shmaddrr = NULL;

    //argv[1] will be the passed algorithm.
    int algorithm = atoi(argv[1]);
    int argument = -1;
    //In case that's Round-Robin then the next parameter will be the quantum.
    if (algorithm == 5)
    {
        argument = atoi(argv[2]);
    }
    printf("algorithm %d \n", algorithm);

    //TODO: upon termination release the clock resources.
    //Debugging:
    int ppid = getppid();
    printf("\n Hey I'm the scheduler  with ppid = %d \n", ppid);

    //The scheduler should receive processes from the process generator through a message queue.
    int q_key = 22;
    int rec_val = 0;
    int i = 0;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);
    struct msqid_ds status;
    if (algorithm == 1)
        shmaddrr = connectsharedmem();

    //We'll be receiving processes in either a minheap or an array depending on the algorithm (the ready queue).
    struct process *procs;
    minHeap HP; //Only one of these will be initialized and used depending on the algorithm.
    if (algorithm == 1 || algorithm == 5)
    {
        procs = (struct process *)malloc(1024 * sizeof(struct process));            //dynamic array.
    }
    else
    {
        switch (algorithm)                                                          //The priority is different for the other three algorithms.
        {
        case 2:
        {
            HP = initMinHeap(2);
            break;
        }

        case 3:
        {
            HP = initMinHeap(3);
            break;
        }
        case 4:
        {
            HP = initMinHeap(4);
            break;
        }
        }
    }

    //Time to receive:
    while (true)
    {
        msgctl(q_id, IPC_STAT, &status); //Checking the status of the queue's struct
        while (status.msg_qnum >= 1)     //To collect all that's in the pipe.
        {
            printf(" \n Scheduler has collected: %lu processes \n", status.msg_qnum);
            struct msgbuff message;
            rec_val = msgrcv(q_id, &message, sizeof(message.mtext), 5, IPC_NOWAIT); //Receiving form the process generator
                                                                                    //If there's a process in the pipe:
            if (rec_val != -1 && algorithm != 0)
            {
                CreateNewProcess(&message);
                totalprocs++;
                if (algorithm == 1 || algorithm == 5) //Depending on the algorithm we choose the appropriate data structure.
                {
                    message.mtext.status = 'W';       //It's waiting.
                    procs[i] = message.mtext;
                }
                else
                {
                    message.mtext.status = 'W';
                    insertNode(&HP, message.mtext);
                }
                printf("\n Process %d has arrived at: %d \n", message.mtext.id, message.mtext.arrival);
                //printf("total alive processes : %d\n",totalprocs);
                i++;
                msgctl(q_id, IPC_STAT, &status); //Updating the status of the queue's struct
            }
        }

        if (algorithm == 1 && CurrProcess < totalprocs)
        {
            FCFS(CurrProcess, procs, shmaddrr);
            CurrProcess++;
        }

        else if (algorithm == 2 && HP.size) //HP should have at least one process.
        {
            ShortestJobFirst(&HP, shmaddrr);
        }
    }

    destroyClk(true);
    return 0;
}

boolean hasPassed(int t, int previousTime, int now) //t seconds have passed from previousTime to now.
{
    return ((now - previousTime) >= t);
}

void ShortestJobFirst(minHeap *HP, void *shmaddrr) //This is non-preemptive, simply choose the process with lowest run time.
{
    int to;
    if (!processRunning) //If some process is running then it shouldn't change, this is non-preemptive.
    {
        hasTurn = (extractMin(HP));
        hasTurn.status = 'S';
        processRunning = 1;                   //So no one preempts the process.
        hasTurn.startingTime = to = getClk(); //We'll need to to check if a second has passed.
        printf("\n SJF: At time %d process %d started arr %d total %d remain %d \n", getClk(), hasTurn.id, hasTurn.arrival, hasTurn.runtime, hasTurn.remTime);
        //Can't see that wait agrees with out definition from the lecture.
    }

    if (hasTurn.remTime > 0 && processRunning) //Running the process by updating its time parameters.
    {
        //printf("\n Second hasn't passed yet. \n");
        if (hasPassed(1, to, getClk())) //If a second has passed, update remaining and executing time.
        {
            //printf("\n SJF: Process No. : %d has Remainingtime: %d \n",hasTurn.id,hasTurn.remTime);
            to = getClk();
            (hasTurn).remTime--;
            (hasTurn).execTime++;
        }
    }
    //According to the doc. the scheduler deletes the process' data from the process control block and its datastructures.
    if (hasTurn.remTime == 0)
    {
        hasTurn.finsihing_time = getClk();
        int TA = hasTurn.finsihing_time - hasTurn.arrival; //Turn around time.
        printf("\n SJF: At time %d process %d finished arr %d total %d remain %d TA %d WTA %.2f \n", getClk(), hasTurn.id, hasTurn.arrival, hasTurn.runtime, hasTurn.remTime, TA, (float)TA / (float)hasTurn.runtime);
        hasTurn.status = 'F';
        dynamicDelete(HP, 0, hasTurn); //Deleting the node that has turn so we never see it again.
        processRunning = 0;             //Now a process can enter.
    }
}

void FCFS(int Currprocess, struct process *procs, void *shmaddrr)
{
    int temptime = getClk();
    while (procs[Currprocess].remTime > 0)
    {
        char snum[5];
        if (getClk() != temptime)
        {
            printf(" >>[inside FCFS]---i am process number :%d ----- My Remainig time is: %d \n", procs[Currprocess].id, procs[Currprocess].remTime);
            temptime = getClk();
            procs[Currprocess].remTime--;
            procs[Currprocess].execTime++;
        }

        sprintf(snum, "%d", procs[Currprocess].remTime);
        strcpy((char *)shmaddrr, snum);
    }
}

void CreateNewProcess(struct msgbuff *message)
{
    char str[5];
    int pid = fork();
    if (pid == 0)
    {
        sprintf(str, "%d", message->mtext.remTime);
        message->mtext.process_id = getpid();
        int value = execl(realpath("process.o", NULL), "process.o", str, NULL);
        printf("the Forking of process.c returns:  %d and the Remaining time sent as a paramater is%s\n", value, str);
    }
}

void *connectsharedmem()
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
