#pragma once
#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
typedef short boolean;
#define true 1
#define false 0
#define EXPECTED -1
#define WAITING 7
#define STOPPED 0
#define STARTED 1
#define RESUMED 2
#define FINISHED 3 

FILE* OUTPUTFILE=NULL;
FILE* OUTPUTF=NULL;             //For the perf file.
boolean OutputFileIsOpened=false;

#define key_id_sch_proc 305
#define SHKEY 300
struct process {
   int id;
   int arrival;                     //Whenever it entered the ready queue.
   int runtime;
   int priority;
   int remTime;
   int startingTime;                //Time in which its chosen by the scheduler (new.)
   int finsihing_time;
   int execTime;
   int status;                     
   int wait;                       //Total waiting time since it entered the system (includes response time*)
   int stop_moment;                 //The time in which the process paused.
   int process_id;
   struct process *next;
   struct process *back;   
};

//Turnaround, Wait and Run arrays.
int Turnaround[1024];
int Wait[1024];
int Run[1024];          
int k=0;
//The actual size of each of the arrays.

struct msgbuff
{
    long mtype;
    struct process mtext;
};
///==============================
//don't mess with this variable//
int *shmaddr; 
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(boolean terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}



///This must be revised well by every member in the team
void ModifyOutputFile(struct process * P,int CurrentState)
{
    if(!OutputFileIsOpened)
    {
        OUTPUTFILE=fopen(realpath("Scheduler.log.txt",NULL),"w"); 
        OutputFileIsOpened=true;
        fprintf(OUTPUTFILE,"#At\ttime\tx\tprocess\ty\tstate       arr\tw\ttotal\tz\tremain\ty\twait\tk\n");
    }

    if(CurrentState==STARTED || (P->remTime==P->runtime && P->runtime!=0))             //that special handling is done for RR
    {
            P->wait+=(getClk() - P->arrival);                           // if it comes at 1 and started running  at 3 is wait should be 2
            P->status=STARTED;
            fprintf (OUTPUTFILE, "At\ttime\t%d\tprocess\t%d\tstarted\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),P->id,P->arrival,P->runtime,P->remTime, P->wait);
    }
    else if(CurrentState==STOPPED){
        P->stop_moment=getClk();
        P->status=STOPPED;
        fprintf(OUTPUTFILE,"At\ttime\t%d\tprocess\t%d\tstopped\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),P->id,P->arrival,P->runtime,P->remTime,P->wait);
    }
    else if(CurrentState==RESUMED)
    {
        P->status=RESUMED;
        P->wait+=getClk()-P->stop_moment;
        fprintf(OUTPUTFILE, "At\ttime\t%d\tprocess\t%d\tresumed\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",getClk(),P->id,P->arrival,P->runtime,P->remTime,P->wait);
    }
    else if(CurrentState==FINISHED)
    {
        P->status=FINISHED;
        P->finsihing_time = getClk();
         fprintf(OUTPUTFILE, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%.2f\n",P->finsihing_time,P->id,P->arrival,P->runtime,P->remTime,P->wait,P->finsihing_time-P->arrival,(float)(P->finsihing_time-P->arrival)/P->runtime);
        Turnaround[k]=P->finsihing_time-P->arrival;
        Wait[k]=P->wait;
        Run[k]=P->runtime;
        k++;
    }

}



void performanceLog()
{

    float AVGTA=0;
    float AVGWait=0;
    float CPUtil=0;
    for(int i=0;i<k;i++)
    {
        AVGWait+=Wait[i];
        AVGTA+=Turnaround[i];
        CPUtil+=Run[i];
    }
    CPUtil=((CPUtil)/(float)getClk())*100;
    AVGTA=((AVGTA)/(float)k);
    AVGWait=((AVGWait)/(float)k);

    OUTPUTF=fopen(realpath("Scheduler.perf.txt",NULL),"w"); 
    fprintf(OUTPUTF,"CPU utilization = %.2f %%\n",CPUtil);
    fprintf(OUTPUTF,"Avg WTA= %.2f \n",AVGTA);
    fprintf(OUTPUTF,"Avg Waiting = %.2f \n",AVGWait);
    fclose(OUTPUTF);


}