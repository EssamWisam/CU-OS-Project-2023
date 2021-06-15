#include "headers.h"
#include "minHeap.h"
void FCFS();
int CreateNewProcess(struct msgbuff *);
void *connectsharedmem();
void ShortestJobFirst(minHeap *, void *); //This is non-preemptive, simply choose the process with lowest run time.
boolean hasPassed(int, int, int);         //t seconds or more have passed from previousTime to now.
void handler();
boolean Recieveifany();
void RR();
boolean Drop(struct process *);
void InsetAtEnd(struct process *);
void Preemept(struct process *, int);
void Continue(struct process *, int);
int Min(struct process *);
void HighestPrioriyFirst(); //This is preemptive, simply choose the process with highest priority.
void ShortestRemainingTimeNext();

boolean processRunning = 0; //Used in SJF.
struct process hasTurn;
struct process *HasTurn; //Pointer version for algorithms 3, 4

//For the doubly-linked list (RR)
struct process *head = NULL;
struct process *tail = NULL;
struct process *iterator = NULL;

//Tracking Processes
int TotalActiveProcessNum = 0;
int TotalProcessesNum = 0;
int CurrProcess = 0;
int Graveyard = 0;
int ii = 0, To = 0; //ii is the last received process, To is the inital time.

//Input
int algorithm = 0;
int argument = 0; //We'll be back.
int quanta = 0;

struct process *procs = NULL; //For the dynamic array.
minHeap HP;                   //Only one of these will be initialized and used depending on the algorithm.
int q_id = 0;                 //Receiving incoming processes.
void *shmaddrr = NULL;        //Shared Memory for the current running process.
int shmid ;                    //Check           
int i = 0; //For populating the dynamic array.s

int main(int argc, char *argv[])
{
    signal(SIGINT, handler);

    initClk();
    To = getClk();

    //argv[1] will be the passed algorithm.
    algorithm = atoi(argv[1]);
    argument = -1;
    //In case that's Round-Robin then the next parameter will be the quantum.
    if (algorithm == 5)
    {
        argument = atoi(argv[2]);
        quanta = argument;
        printf("RR is chosen with quantum = %d...\n", argument);
    }
    printf("algorithm %d \n", algorithm);

    //TODO: upon termination release the clock resources.
    //Debugging:
    int ppid = getppid();

    printf("\n Scheduler is starting: \n");

    //The scheduler should receive processes from the process generator through a message queue.
    int q_key = 22;
    int rec_val = 0;
    q_id = msgget(q_key, 0666 | IPC_CREAT);

    //It should also communicate with process.c to share remaining time.
    shmaddrr = connectsharedmem();

    //The proces that has turn in algorithms 3 and 4.
    HasTurn = malloc(sizeof(struct process));
    HasTurn->id = -1;
    HasTurn->status = 6980;

    //We'll be receiving processes in either a minheap or an array depending on the algorithm (the ready queue).

    procs = (struct process *)malloc(1024 * sizeof(struct process)); //dynamic array.

    switch (algorithm) //SJF uses a minheap.
    {
    case 2:
    {
        HP = initMinHeap(2);
        break;
    }
    }

    while (true) //The scheduler starts here.
    {
        //Receiving form the process generator:
        struct msgbuff message;
        rec_val = msgrcv(q_id, &message, sizeof(message.mtext), 5, IPC_NOWAIT); //Receiving form the process generator

        while (rec_val != -1 && algorithm != 0) //While there's a process in the pipe:
        {
            TotalProcessesNum++;
            procs[i] = message.mtext;
            procs[i].process_id = CreateNewProcess(&message);
            procs[i].status = WAITING;
            if (algorithm == 5)
                InsetAtEnd(&procs[i]);
            i++;
            if (algorithm == 2)
            {
                message.mtext.process_id = CreateNewProcess(&message);
                insertNode(&HP, message.mtext);
            }
            rec_val = msgrcv(q_id, &message, sizeof(message.mtext), 5, IPC_NOWAIT); //Receiving form the process generator
        }                                                                         

        switch (algorithm)
        {
        case 1:
        {

            if (CurrProcess < TotalProcessesNum)
            {
                FCFS(CurrProcess, procs, shmaddrr);
                CurrProcess++;
                break;
            }
        }

        case 2:
        {
            if (HP.size)                            //HP should have at least one process.
            {
                if (To != getClk() || ii != i)      //A second has passed or a process has just enetered (premepetion)
                {
                    ShortestJobFirst(&HP, shmaddrr);
                    To = getClk();
                    ii = i;
                }
            }
            break;
        }

        case 3:
        {

            if ((Graveyard != i))                   //As long as not all of them are dead.
            {
                if (To != getClk() || ii != i)          
                {
                    HighestPrioriyFirst();
                    To = getClk();
                    ii = i;
                }
            }
            break;
        }

        case 4:
        {
            if ((Graveyard != i))
            {
                if (To != getClk() || ii != i)
                {
                    ShortestRemainingTimeNext();
                    To = getClk();
                    ii = i;
                }
            }
            break;
        }

        case 5:
        {
            if (algorithm == 5)
            {

                if (iterator == NULL)
                    iterator = head;
                else
                    iterator = iterator->back;
                RR();
                break;
            }
        }
        }
    }

    destroyClk(true);
    return 0;
}

void FCFS()
{
    ModifyOutputFile(&procs[CurrProcess], STARTED);
    kill(procs[CurrProcess].process_id, SIGCONT); //The process was initially STOPPED

    int ExitTime = getClk() + procs[CurrProcess].remTime;

    int temptime = -1;
    while (procs[CurrProcess].remTime > 0)
    {

        char snum[12];
        if (getClk() != temptime)
        {

            printf("\n >>[inside FCFS]---i am process number :%d ----- My Remainig time is: %d \n", procs[CurrProcess].id, procs[CurrProcess].remTime);
            temptime = getClk();
            procs[CurrProcess].remTime--;
        }

        sprintf(snum, "%d", procs[CurrProcess].remTime);
        strcpy((char *)shmaddrr, snum);

        ////////////////////////////////////////To recieve any coming processes////////////////////////////
        Recieveifany();
        ////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    while (getClk() < ExitTime);


    procs[CurrProcess].finsihing_time = getClk();
    ModifyOutputFile(&procs[CurrProcess], FINISHED);
    int R;
    wait(&R);   
}


void ShortestJobFirst(minHeap *HP, void *shmaddrr) //This is non-preemptive, simply choose the process with lowest run time.
{
    if (!processRunning)            //If some process is running then it shouldn't change, this is non-preemptive.
    {
        hasTurn = (struct process)extractMin(HP);
        kill(hasTurn.process_id, SIGCONT);
        processRunning = 1;              //So no one preempts the process.
        ModifyOutputFile(&hasTurn, STARTED);
    }

    else if (hasTurn.remTime > 0 && processRunning) //Running the process by updating its time parameters.
    {
        if (To != getClk())                         //If a second has passed, update remaining and executing time.
        {
            (hasTurn).remTime--;
            (hasTurn).execTime++;
        }
        //Communicating with process.c
        char snum[12];
        sprintf(snum, "%d", hasTurn.remTime);
        strcpy((char *)shmaddrr, snum);
    }
    //According to the doc. the scheduler deletes the process' data from the process control block and its datastructures.
    if (hasTurn.remTime == 0)
    {
        ModifyOutputFile(&hasTurn, FINISHED);
        dynamicDelete(HP, 0, hasTurn); //Deleting the node that has turn so we never see it again.
        processRunning = 0;            //Now a process can enter.
        int R;
        wait(&R);                                           //So it doesn't go defunct.
        if (HP->size)   ShortestJobFirst(HP, shmaddrr);     //Check if there can be a next process right away.
    }
}

//Check SRTN for the comments.
void HighestPrioriyFirst() //This is preemptive, simply choose the process with highest priority.
{
    if (procs[Min(procs)].id != HasTurn->id) //Someone is more important than the current process, or there isn't one yet.
    {
        if (processRunning) 
        {
            if (To != getClk())
            {
                (HasTurn)->remTime--;
                (HasTurn)->execTime++;
            }
            char snum[12];
            sprintf(snum, "%d", HasTurn->remTime);
            strcpy((char *)shmaddrr, snum);

            if (HasTurn->remTime != 0)
            {
                Preemept(HasTurn, HasTurn->process_id);
            }
            else
            {
                ModifyOutputFile(HasTurn, FINISHED);
                HasTurn->remTime = 1e9;
                HasTurn->priority = 1e9;
                Graveyard++;
                processRunning = 0;
                HasTurn->id = -1;
                int R;
                wait(&R);
            }
        }

        HasTurn = &procs[Min(procs)];  
        if (HasTurn->status == STOPPED) //It's either that it was preempted eariler.
        {
            Continue(HasTurn, HasTurn->process_id);
        }
        else if (HasTurn->status == WAITING) //Or that it's brand new (this or the if has to be true)
        {
            kill(HasTurn->process_id, SIGCONT);
            ModifyOutputFile(HasTurn, STARTED);
        }
        processRunning = true;
       
    }

    else if (HasTurn->remTime > 0 && processRunning) //Running the process by updating its time parameters.
    {
        if (To != getClk())
        {
            (HasTurn)->remTime--;
            (HasTurn)->execTime++;
        }
        char snum[12];
        sprintf(snum, "%d", HasTurn->remTime);
        strcpy((char *)shmaddrr, snum);
    }
    if (HasTurn->remTime == 0)
    {
        ModifyOutputFile(HasTurn, FINISHED);
        HasTurn->id = -1;
        HasTurn->priority = 1e9;
        HasTurn->remTime = 1e9;
        Graveyard++;
        processRunning = 0;
        int R;
        wait(&R);
        if (Graveyard != i)     HighestPrioriyFirst();
    }
}

void ShortestRemainingTimeNext()                //This is preemptive, simply choose the process with lowest remaining time.
{
    if (procs[Min(procs)].id != HasTurn->id)    //Someone is more important than the current process, or there isn't one yet.
    {
        if (processRunning)                     //Some process ir running, we got to preempt it, the condition is guaranteed to be false for the first process.
        {
            if (To != getClk())
            {
                (HasTurn)->remTime--;
                (HasTurn)->execTime++;
            }
            char snum[12];
            sprintf(snum, "%d", HasTurn->remTime);
            strcpy((char *)shmaddrr, snum);

            if (HasTurn->remTime != 0)      //Before we preempt we give it its last second (the current one).
            {
                Preemept(HasTurn, HasTurn->process_id);
            }
            else                            //Looks like it died
            {
                ModifyOutputFile(HasTurn, FINISHED);
                //So we never see them again.
                HasTurn->remTime = 1e9;
                HasTurn->priority = 1e9;
                Graveyard++;
                //Back to the initial value.
                processRunning = 0;
                HasTurn->id = -1;
                //So it does go defunct.
                int R;
                wait(&R);
            }
        }

        HasTurn = &procs[Min(procs)];       //The higher priority process, also two cases here.
        if (HasTurn->status == STOPPED)     //It's either that it was preempted eariler, so we just continue it.
        {
            Continue(HasTurn, HasTurn->process_id);
        }
        else if (HasTurn->status == WAITING) //Or that it's brand new (this or the if has to be true)
        {
            kill(HasTurn->process_id, SIGCONT);
            ModifyOutputFile(HasTurn, STARTED);
        }
        processRunning = true;
       
    }

    else if (HasTurn->remTime > 0 && processRunning) //Running the process by updating its time parameters.
    {

        if (To != getClk())
        {
            (HasTurn)->remTime--;
            (HasTurn)->execTime++;
        }
        char snum[12];
        sprintf(snum, "%d", HasTurn->remTime);
        strcpy((char *)shmaddrr, snum);
    }
    if (HasTurn->remTime == 0)              //The process died.
    {
        ModifyOutputFile(HasTurn, FINISHED);
        //So we never see it again.
        HasTurn->id = -1;
        HasTurn->priority = 1e9;
        HasTurn->remTime = 1e9;
        Graveyard++;
        processRunning = 0;
        int R;
        wait(&R);
        //Check if there's opprtunity to run the next one.
        if (Graveyard != i)     ShortestRemainingTimeNext();
    }
}


boolean hasPassed(int t, int previousTime, int now) //t seconds have passed from previousTime to now.
{
    return ((now - previousTime) >= t);
}





int CreateNewProcess(struct msgbuff *message)
{
    char str[5];
    int pid = fork();

    if (pid == 0)
    {
        sprintf(str, "%d", message->mtext.remTime);

        int value = execl(realpath("process.o", NULL), "process.o", str, NULL);

        printf("the Forking of process.c returns: %d\n", -1); // this return -1 if execl fails
    }

    kill(pid, SIGTSTP);
    return pid; // here we r returning the pid of the forked process
}

void *connectsharedmem()
{
    shmid = shmget(key_id_sch_proc, 4096, IPC_CREAT | 0644);
    if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    void *shmaddrr = shmat(shmid, (void *)0, 0);
    return shmaddrr;
}

void InsetAtEnd(struct process *newone)
{

    if (head == NULL)
    {
        head = newone;
        tail = head;           //<-[]->
        newone->next = newone; //Doubly circular linked list
        newone->back = newone;
    }
    else
    { //the true meaning of hell
        newone->next = tail;
        newone->back = head;
        tail = newone;
        newone->next->back = newone;
        head->next = newone;
    }

    TotalActiveProcessNum++;
    printf("\nNotification:\n");
    printf("Node# %d  arrived at %d and inserted to the linked list AND the total active processes number is %d after calling InsertAtEnd\n\n", newone->id, newone->arrival, TotalActiveProcessNum);
}

boolean Drop(struct process *leavingone)
{
    if (TotalActiveProcessNum == 0)
        return false;

    if (head == leavingone)
    {
        head->back->next = tail; //Deleting the head and connect the one before the head by the tail
        tail->back = head->back;
        head = head->back;
    }
    else if (tail == leavingone)
    {
        tail->next->back = head;
        head->next = tail->next;
        tail = tail->next;
    }
    else
    {
        leavingone->back->next = leavingone->next;
        leavingone->next->back = leavingone->back;
    }

    TotalActiveProcessNum--;

    leavingone->finsihing_time = getClk();
    printf("\nNotification:\n");
    printf("Node# %d  has finished at %d and droped from the linked list and The total active process num is %d after calling Drop\n\n", leavingone->id, leavingone->finsihing_time, TotalActiveProcessNum);

    return true;
}

boolean Recieveifany()
{
    struct msgbuff message;
    int rec_val = msgrcv(q_id, &message, sizeof(message.mtext), 5, IPC_NOWAIT);

    while (rec_val != -1)
    {

        if (algorithm != 2) //Depending on the algorithm we choose the appropriate data structure.
        {
            TotalProcessesNum++;
            message.mtext.status = WAITING; //It's waiting.
            procs[i] = message.mtext;
            procs[i].process_id = CreateNewProcess(&message);
            if (algorithm == 5)
                InsetAtEnd(&procs[i]);
            i++;
        }
        else
        {
            message.mtext.process_id = CreateNewProcess(&message);
            insertNode(&HP, message.mtext);
        }
        rec_val = msgrcv(q_id, &message, sizeof(message.mtext), 5, IPC_NOWAIT); //Receiving form the process generator
    }

    return true;
}

void handler()
{
    fclose(OUTPUTFILE);
    performanceLog();
    int val= shmdt (shmaddrr);   // deattach 
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    exit(0);
    //kill(getpid(), SIGKILL);
}

void RR()
{

    if (iterator == NULL || TotalActiveProcessNum == 0)
        return;

    ModifyOutputFile(iterator, RESUMED); // Resumed?? it may be first time to enter that section ?      ->   handled :)
    kill(iterator->process_id, SIGCONT);

    printf("Process # %d is started/continued with Remaining time %d and at clk : %d..\n", iterator->id, iterator->remTime, getClk());

    int temptime = -1;
    int counter = 0; // to monitor the number of quantums consumed

    int ExitTime = 0;
    if (iterator->remTime < quanta)
        ExitTime = getClk() + iterator->remTime;
    else
        ExitTime = getClk() + quanta;

    while (counter < quanta && iterator->remTime > 0)
    {
        char snum[12]; //temporary string that will hold the remainig time.
        if (getClk() != temptime)
        {
            temptime = getClk();
            iterator->remTime--;
            printf(" >>[inside RR]---i am process number :%d ----- My Remainig time is: %d and the clk is: %d\n", iterator->id, iterator->remTime, getClk());
            counter++;
        }

        sprintf(snum, "%d", iterator->remTime);
        strcpy((char *)shmaddrr, snum);

        ////////////////////////////////////////To recieve any coming processes////////////////////////////
        Recieveifany();
        ////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    while (ExitTime > getClk());


    if (iterator->remTime == 0)
    {

        Drop(iterator); // Drop from the linked list
        ModifyOutputFile(iterator, FINISHED);
        //int R;
        //wait(&R);
    }
    else
    {
        printf("Process# %d is stopped with remaining time %d and the clk :%d.. \n\n", iterator->id, iterator->remTime, getClk());
        kill(iterator->process_id, SIGTSTP); // Stop the process till it get turn again
        ModifyOutputFile(iterator, STOPPED);
    }
}

int Min(struct process *P)
{
    switch (algorithm) //The priority is different for the other three algorithms.
    {
    case 2:
    {

        int Q = 0; //Index at which the minimum occurs.
        for (int j = 0; j < i; j++)
            if (P[j].runtime < P[Q].runtime)
                Q = j;
        return Q;
    }

    case 3:
    {
        int Q = 0; //Index at which the minimum occurs.
        for (int j = 0; j < i; j++)
            if (procs[j].priority < procs[Q].priority)
                Q = j;

        return Q;
    }
    case 4:
    {
        int Q = 0; //Index at which the minimum occurs.
        for (int j = 0; j < i; j++)
            if (P[j].remTime < P[Q].remTime)
                Q = j;
        return Q;
    }
    }
}

void Preemept(struct process *P, int Proc_ID)
{
    kill(P->process_id, SIGTSTP); // Stop the process till it get turn again
    ModifyOutputFile(P, STOPPED);
}

void Continue(struct process *P, int Proc_ID)
{
    kill(P->process_id, SIGCONT);
    ModifyOutputFile(P, RESUMED);
}
