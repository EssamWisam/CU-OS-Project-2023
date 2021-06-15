#include "headers.h"
#include "minHeap.h"
void FCFS();
int CreateNewProcess(struct process *);
void *connectsharedmem();
void ShortestJobFirst(minHeap *, void *);            //This is non-preemptive, simply choose the process with lowest run time.
boolean hasPassed(int , int , int ); //t seconds or more have passed from previousTime to now.
void handler();
boolean Recieveifany();
void RR();
boolean Drop(struct process*);
void InsetAtEnd(struct process* );


void Preemept(struct process *, int);
void Continue(struct process *, int);
int Min(struct process *);
void HighestPrioriyFirst(); //This is preemptive, simply choose the process with highest priority.
void ShortestRemainingTimeNext();


int BestFit(struct process* newComer);
boolean GetNextSize(int start, int procsize,int* slotSize,int* nextStart);
int GetMinimum(int x,int y);
int FirstFit(struct process* newComer);
int NextFit (struct process* newComer);
int allocateMemory(struct process* newComer);
int deallocateMemory(struct process* leaving);
boolean DeleteFromWaitingList(struct process * leavingone);
int InsertAtWaitingList(struct process * newcomer);
struct process * ExtractOneFromWaitingList();






boolean processRunning = 0;             //Used in SJF.
struct process hasTurn;
struct process *HasTurn; //Pointer version for algorithms 3, 4



int Graveyard = 0;
int ii = 0, To = 0; //ii is the last received process, To is the inital time.

// for waiting list phase 2
struct process* First_Wait=NULL;
struct process* End_Wait=NULL;
int numberOfWaiting=0;
int j=0;
struct process * waitarr=NULL;


//For the doubly-linked list (RR)
struct process* head=NULL;              
struct process* tail=NULL;             
struct process* iterator=NULL;      


//Tracking Processes
int TotalActiveProcessNum=0;         
int TotalProcessesNum=0;
int CurrProcess=0;

//Input
int algorithm=0;
int argument=0;                 //We'll be back.
int quanta=0;

 struct process * procs=NULL;   //For the dynamic array.
 minHeap HP; //Only one of these will be initialized and used depending on the algorithm.
 int q_id=0;                    //Receiving incoming processes.
 void *shmaddrr=NULL;           //Shared Memory for the current running process.

 int i = 0;                     //For populating the dynamic array.s


int MemoryAlgo;
int Memory[1024];
int memIter=0;


int main(int argc, char *argv[])
{
    signal(SIGINT,handler);

    initClk();


    //argv[1] will be the passed algorithm.
    algorithm = atoi(argv[1]);
    

    argument = -1;
    //In case that's Round-Robin then the next parameter will be the quantum.
    if (algorithm == 5)
    {
        MemoryAlgo=atoi(argv[3]);
        argument = atoi(argv[2]);
  
        quanta=argument;  printf("RR is chosen with quantum = %d...\n",argument);
    }
    else
        MemoryAlgo=atoi(argv[2]);
        


    printf("algorithm %d \n", algorithm);
    printf("Memory algorithm %d \n", MemoryAlgo);



    //TODO: upon termination release the clock resources.
    //Debugging:
    int ppid = getppid();
    printf("\n Hey I'm the scheduler  with ppid = %d \n", ppid);

    //The scheduler should receive processes from the process generator through a message queue.
    int q_key = 22;
    int rec_val = 0;
    q_id = msgget(q_key, 0666 | IPC_CREAT);
    struct msqid_ds status;
    shmaddrr = connectsharedmem();


//The proces that has turn in algorithms 3 and 4.
    HasTurn = malloc(sizeof(struct process));
    HasTurn->id = -1;
    HasTurn->status = 6980;


    //We'll be receiving processes in either a minheap or an array depending on the algorithm (the ready queue).
  
 procs = (struct process *)malloc(1024 * sizeof(struct process)); //dynamic array.
 waitarr= (struct process *)malloc(1024 * sizeof(struct process)); //dynamic array.
    
   switch (algorithm) //SJF uses a minheap.
    {
    case 2:
    {
        HP = initMinHeap(2);
        break;
    }
    }

    //Time to receive:
    while (true)
    {
        
            struct msgbuff message;
            rec_val = msgrcv(q_id, &message, sizeof(message.mtext), 5, IPC_NOWAIT); //Receiving form the process generator

            while(rec_val!=-1)
            {
               waitarr[j]=message.mtext;
                 printf("waitarr[j]: %p\n",&waitarr[j]);
                InsertAtWaitingList(&waitarr[j]);
                j++;
                rec_val = msgrcv(q_id,&message, sizeof(message.mtext), 5, IPC_NOWAIT);
            }
              struct process* newComer = ExtractOneFromWaitingList();

             while(newComer!=NULL)
            {
        
                if (algorithm!=2) //Depending on the algorithm we choose the appropriate data structure.
                {
                        TotalProcessesNum++;
                        (*newComer).status = WAITING; //It's waiting.
                        procs[i] = *newComer;
                        procs[i].process_id=CreateNewProcess(newComer);
                        if(algorithm==5)    InsetAtEnd(&procs[i]);
                        i++;
                    
                }
                else
                {
                    (*newComer).status = WAITING;
                    (*newComer).process_id=CreateNewProcess(newComer);
                    insertNode(&HP,(*newComer));
                }

                newComer = ExtractOneFromWaitingList();
                    
            }

        switch (algorithm)
        {
        case 1:
        {

            if (CurrProcess < TotalProcessesNum)
            {
                FCFS();
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

boolean hasPassed(int t, int previousTime, int now) //t seconds have passed from previousTime to now.
{
    return ((now - previousTime) >= t);
}


void ShortestJobFirst(minHeap *HP, void *shmaddrr) //This is non-preemptive, simply choose the process with lowest run time.
{
    if (!processRunning)            //If some process is running then it shouldn't change, this is non-preemptive.
    {
        hasTurn = (struct process)extractMin(HP);
        kill(hasTurn.process_id, SIGCONT);
        processRunning = 1;              //So no one preempts the process.
        ModifyOutputFilememory(&hasTurn, STARTED);
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
        ModifyOutputFilememory(&hasTurn, FINISHED);
        dynamicDelete(HP, 0, hasTurn); //Deleting the node that has turn so we never see it again.
        processRunning = 0;            //Now a process can enter.
        deallocateMemory(&hasTurn);
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
                ModifyOutputFilememory(HasTurn, FINISHED);
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
            ModifyOutputFilememory(HasTurn, STARTED);
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
        ModifyOutputFilememory(HasTurn, FINISHED);
        HasTurn->id = -1;
        HasTurn->priority = 1e9;
        HasTurn->remTime = 1e9;
        Graveyard++;
        processRunning = 0;
        deallocateMemory(HasTurn);
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
                ModifyOutputFilememory(HasTurn, FINISHED);
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
            ModifyOutputFilememory(HasTurn, STARTED);
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
        ModifyOutputFilememory(HasTurn, FINISHED);
        //So we never see it again.
        HasTurn->id = -1;
        HasTurn->priority = 1e9;
        HasTurn->remTime = 1e9;
        Graveyard++;
        processRunning = 0;
        deallocateMemory(&hasTurn);
        int R;
        wait(&R);
        //Check if there's opprtunity to run the next one.
        if (Graveyard != i)     ShortestRemainingTimeNext();
    }
}
void FCFS()
{
    ModifyOutputFilememory(&procs[CurrProcess],STARTED);
    kill(procs[CurrProcess].process_id,SIGCONT);                                             //The process was initially STOPPED 

    int ExitTime=getClk()+procs[CurrProcess].remTime;

    int temptime =-1;
    while(procs[CurrProcess].remTime>0)
    {

        char snum[12];
        if(getClk()!=temptime)                            
        {

            printf(" >>[inside FCFS]---i am process number :%d ----- My Remainig time is: %d \n",procs[CurrProcess].id,procs[CurrProcess].remTime);
            temptime=getClk();
            procs[CurrProcess].remTime--;
        }

         sprintf(snum,"%d", procs[CurrProcess].remTime);
         strcpy((char*) shmaddrr,snum);

        ////////////////////////////////////////To recieve any coming processes////////////////////////////
        Recieveifany();
        ////////////////////////////////////////////////////////////////////////////////////////////////////
         
    }

    while(getClk()<ExitTime){};

    procs[CurrProcess].finsihing_time=getClk();
    ModifyOutputFilememory(&procs[CurrProcess],FINISHED);
    deallocateMemory(&procs[CurrProcess]);
    int R;
    wait(&R);

}

int CreateNewProcess( struct process * newcomer)          
{
    char str[12];
    int pid = fork();
   
    if(pid==0)
    {
        sprintf(str,"%d",newcomer->remTime);
       
        int value = execl(realpath("process.o",NULL),"process.o",str, NULL);

        printf("the Forking of process.c returns: %d\n",-1);                    // this return -1 if execl fails
    }
       
       kill(pid,SIGTSTP);
       return pid;                                                              // here we r returning the pid of the forked process
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

void InsetAtEnd(struct process* newone)
{

    if(head==NULL)
    {
        head= newone;
        tail=head;                                  //<-[]->
        newone->next=newone;                     //Doubly circular linked list
        newone->back=newone;
    }
    else
    {                                            //the true meaning of hell
        newone->next=tail;
        newone->back=head;
        tail=newone;
        newone->next->back=newone;
        head->next=newone;
    }

    TotalActiveProcessNum++;
    printf("\nNotification:\n");
    printf("Node# %d  arrived at %d and inserted to the linked list AND the total active processes number is %d after calling InsertAtEnd\n\n",newone->id,newone->arrival,TotalActiveProcessNum);

}

boolean Drop(struct process* leavingone)
{
    if (TotalActiveProcessNum==0)
        return false;

    if (TotalActiveProcessNum==1)
    {
        head->next=NULL;
        tail->next=NULL;
        head=NULL;
        tail=NULL;
    }

    
    if(head == leavingone)
    {
        head->back->next=tail;                                         //Deleting the head and connect the one before the head by the tail
        tail->back=head->back;
        head=head->back;
    }
    else if(tail== leavingone)
    {
        tail->next->back=head;
        head->next=tail->next;
        tail=tail->next;
    }
    else
    {
        leavingone->back->next=leavingone->next;
        leavingone->next->back=leavingone->back;
    }

    TotalActiveProcessNum--;


     leavingone->finsihing_time=getClk();                      
     printf("\nNotification:\n");
     printf("Node# %d  has finished at %d and droped from the linked list and The total active process num is %d after calling Drop\n\n",leavingone->id,leavingone->finsihing_time,TotalActiveProcessNum);


    return true;

}

boolean Recieveifany()
{
    struct msgbuff message;
    int rec_val = msgrcv(q_id,&message, sizeof(message.mtext), 5, IPC_NOWAIT);
    while(rec_val!=-1)
    {
        waitarr[j]=message.mtext;
                 printf("waitarr[j]: %p\n",&waitarr[j]);
                InsertAtWaitingList(&waitarr[j]);
                j++;
        rec_val = msgrcv(q_id,&message, sizeof(message.mtext), 5, IPC_NOWAIT);
    }
      struct process* newComer = ExtractOneFromWaitingList();
    while(newComer!=NULL)
    {
        if (algorithm == 1 || algorithm == 5) //Depending on the algorithm we choose the appropriate data structure.
        {
                TotalProcessesNum++;
                (*newComer).status = WAITING; //It's waiting.
                procs[i] = *(newComer);
                procs[i].process_id=CreateNewProcess(newComer);
                if(algorithm==5)    InsetAtEnd(&procs[i]);
                i++;
            
        }
        else
        {
           (*newComer).status = WAITING;
            (*newComer).process_id=CreateNewProcess(newComer);
            insertNode(&HP,(*newComer));
        }
        newComer = ExtractOneFromWaitingList();
    }

return true;

}

void handler()
{
   fclose (OUTPUTFILE);
   performanceLog();
   shmctl(key_id_sch_proc, IPC_RMID, (struct shmid_ds *)0);
   kill(getpid(),SIGKILL);
}

void RR()
{
   
    if(iterator==NULL || TotalActiveProcessNum==0 )
        return;

    ModifyOutputFilememory(iterator,RESUMED);                                               // Resumed?? it may be first time to enter that section ?      ->   handled :)
    kill(iterator->process_id,SIGCONT);

    printf("Process # %d is started/continued with Remaining time %d and at clk : %d..\n",iterator->id,iterator->remTime,getClk());
   
    int temptime =-1;
    int counter=0;                                                                     // to monitor the number of quantums consumed
   
    int ExitTime =0;
    if(iterator->remTime<quanta)
        ExitTime=getClk()+iterator->remTime;
    else
        ExitTime=getClk()+quanta;

    while(counter<quanta &&iterator->remTime>0 )
    {
        char snum[12];                                                                    //temporary string that will hold the remainig time.
        if(getClk()!=temptime)
        {  
            temptime=getClk();
            iterator->remTime--;
            printf(" >>[inside RR]---i am process number :%d ----- My Remainig time is: %d and the clk is: %d\n",iterator->id,iterator->remTime,getClk());
            counter++;
        }

        sprintf(snum,"%d", iterator->remTime);
        strcpy((char*) shmaddrr,snum);

        ////////////////////////////////////////To recieve any coming processes////////////////////////////
        Recieveifany();
        ////////////////////////////////////////////////////////////////////////////////////////////////////

    }
    
    while(ExitTime>getClk()){};
   
    
    if(iterator->remTime==0) {

       Drop(iterator);                                                // Drop from the linked list
       ModifyOutputFilememory(iterator,FINISHED);     
       deallocateMemory(iterator);
        int R;
        wait(&R);
     
    }
    else
    {
      printf("Process# %d is stopped with remaining time %d and the clk :%d.. \n\n",iterator->id,iterator->remTime,getClk());
      kill(iterator->process_id,SIGTSTP);                            // Stop the process till it get turn again
      ModifyOutputFilememory(iterator,STOPPED);     
    }
     
}

int GetMinimum(int x,int y)
{
    if(x<y)
        return x;
    else 
        return y;
}

boolean GetNextSize(int start, int procsize,int* slotSize,int* nextStart)   //start : index at which we found zero  // change slotSize if and only if slotsize is greater than procsize 
{
   int i;
   boolean boolrtn=true;
    for( i=start;i<(start+procsize);i++)
        if( i>=MEMSIZE|| Memory[i]!=0 )
           {
              *nextStart=i;         // mkan ma ana w23t   19 or 1024
              *slotSize=1000000;
               return false;
           }

    while(i<MEMSIZE && Memory[i]==0)
        i=i+1;

    if(i>=MEMSIZE)
        *slotSize=(MEMSIZE-1)-start+1;
    else
        *slotSize=i-start;

      *nextStart=i;

    return true;
          
}


int BestFit(struct process* newComer)
{
    memIter=0;
    int bestStart=-1;
    int bestSize=1000000;                             //OO
    int procSize=newComer->memorySize;

    int slotSize=0;
    int nextStart=0;


    for(int i=0;i<MEMSIZE;i++)
    {
       if(i<0) continue;                             // :(

        if(Memory[i]!=0)
        {
            i=i+Memory[i]-1;                     ///AS it will be increased by the forloop just after that line by one
        }
        else
        {
            if(GetNextSize(i,procSize,&slotSize,&nextStart))
            {
                if(GetMinimum(bestSize,slotSize)==slotSize)
                    {
                        bestSize=slotSize;
                        bestStart=i;
                        i=nextStart-1;                ///ha5a
                    }
                    else
                    i=i+slotSize-1;
            }
            else
                i=nextStart-1;
        }
    }

if(bestStart!=-1){
   newComer->memoryIndex=bestStart;
   Memory[bestStart]=procSize;
   return 1;
}

return -1;

}

int FirstFit(struct process* newComer)
{
    memIter=0;
    int size=0;
    while(memIter<MEMSIZE)
    {
        if(Memory[memIter]==0)          
        {
           size++;
           if(size==newComer->memorySize)
            {
                newComer->memoryIndex=memIter-size+1;   
                Memory[memIter-size+1]=newComer->memorySize;
                return 1;
            }
           memIter++;
        }
        else
        {
            size=0;
            memIter=memIter+Memory[memIter];
        }


    }
    return -1;
}

int NextFit (struct process* newComer){
   boolean fristTime = true;
   int itr = memIter;
   while ((itr + newComer->memorySize-1)<  MEMSIZE || fristTime){
      if ((itr + newComer->memorySize-1)>= MEMSIZE ){
         fristTime = false;
         itr = 0;
       } 
      for (int i = itr ; i < (itr+ newComer->memorySize) ; i++){
         if (Memory[i] != 0){
            itr = itr+ Memory[i];
            break;
         }
         if (i == (itr+ newComer->memorySize-1)){
            newComer->memoryIndex = itr;
            Memory[itr] = newComer->memorySize;
            memIter = i+1;
            return 1;
         }
      }
   }
   return -1;
}

int allocateMemory(struct process* newComer)
{
    switch (MemoryAlgo)
    {

    case FIRSTFIT:
    return FirstFit(newComer);
        break;

    case NEXTFIT:
    return NextFit(newComer);
        break;

    case BESTFIT:
    return BestFit(newComer);
        break;
    
    case BUDDY:

    break;
    
    default:
        break;
    }
}

int deallocateMemory(struct process* leaving)
{
     Memory[leaving->memoryIndex]=0;
}

int InsertAtWaitingList(struct process * newone)
{
    if(First_Wait==NULL)
    {
        First_Wait= newone;
        End_Wait=First_Wait;                                        //<-[]->
        newone->next_wait=newone;                             //Doubly circular linked list
        newone->back_wait=newone;
    }
    else
    {                                                         //the true meaning of hell
        newone->next_wait=End_Wait;
        newone->back_wait=First_Wait;
        End_Wait=newone;
        newone->next_wait->back_wait=newone;
        First_Wait->next_wait=newone;
    }
    numberOfWaiting++;
}

boolean DeleteFromWaitingList(struct process * leavingone)
{

 if(numberOfWaiting==1 )
    {
        First_Wait->back_wait=NULL;
        First_Wait->next_wait=NULL;
        First_Wait=NULL;
        End_Wait=NULL;
        numberOfWaiting--;
        return true;
    }
    
   
    if(First_Wait == leavingone)
    {
        First_Wait->back_wait->next_wait=End_Wait;                 //Deleting the head and connect the one before the head by the tail
        End_Wait->back_wait=First_Wait->back_wait;
        First_Wait=First_Wait->back_wait;
    }
    else if(End_Wait== leavingone)
    {
        End_Wait->next_wait->back_wait=First_Wait;
        First_Wait->next_wait=End_Wait->next_wait;
        End_Wait=End_Wait->next_wait;
    }
    else
    {
        leavingone->back_wait->next_wait=leavingone->next_wait;
        leavingone->next_wait->back_wait=leavingone->back_wait;
    }

    numberOfWaiting--;
    return true;

}


struct process * ExtractOneFromWaitingList()
{
    struct process *IT = First_Wait;

    for(int i=0;i<numberOfWaiting;i++)
    {
        if(allocateMemory(IT)!=-1)
            {
                DeleteFromWaitingList(IT);
                return IT;
            }
    }

     return NULL;
    
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
    ModifyOutputFilememory(P, STOPPED);
}

void Continue(struct process *P, int Proc_ID)
{
    kill(P->process_id, SIGCONT);
    ModifyOutputFilememory(P, RESUMED);
}