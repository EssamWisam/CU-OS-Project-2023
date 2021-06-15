#include <stdio.h>
//#include "headers.h"

// int MemoryAlgo;
// char Memory[10]={0,0,0,0,0,1,0,0,0,0};
// int arrsize=10;
// int memIter=0;

// struct process * First_Wait=NULL;
// struct process * End_Wait=NULL;

// int number=0;

// int InsertAtWaitingList(struct process * newone)
// {
//     if(First_Wait==NULL)
//     {
//         First_Wait= newone;
//         End_Wait=First_Wait;                                        //<-[]->
//         newone->next_wait=newone;                             //Doubly circular linked list
//         newone->back_wait=newone;
//     }
//     else
//     {                                                         //the true meaning of hell
//         newone->next_wait=End_Wait;
//         newone->back_wait=First_Wait;
//         End_Wait=newone;
//         newone->next_wait->back_wait=newone;
//         First_Wait->next_wait=newone;
//     }
//     number++;
// }

// boolean DeleteFromWaitingList(struct process * leavingone)
// {
   
//     if(First_Wait == leavingone)
//     {
//         First_Wait->back_wait->next_wait=End_Wait;                 //Deleting the head and connect the one before the head by the tail
//         End_Wait->back_wait=First_Wait->back_wait;
//         First_Wait=First_Wait->back_wait;
//     }
//     else if(End_Wait== leavingone)
//     {
//         End_Wait->next_wait->back_wait=First_Wait;
//         First_Wait->next_wait=End_Wait->next_wait;
//         End_Wait=End_Wait->next_wait;
//     }
//     else
//     {
//         leavingone->back_wait->next_wait=leavingone->next_wait;
//         leavingone->next_wait->back_wait=leavingone->back_wait;
//     }

// number--;
//     return true;

// }







// void printarr()
// { 
//    struct process * iter= First_Wait;
//    for(int i =0;i<number;i++)
//    {
//       printf("%d ",iter->id);
//       iter=iter->back_wait;
//    }
//    printf("\n");
  
// }

int GetNP2(int x)         //nearset power of 2
{
    int p =1;

    while(p<=256)
    {
        if(p>=x)
            return p;
        p=p*2;
    }
    
    return -1;

}
int main() 
{
    //   struct process p1 ;
    //   struct process p2 ;
    //   struct process p3 ;
    //   struct process p4 ;
    //   struct process p5 ;
        
    //   p1.id=1;
    //   p2.id=2;
    //   p3.id=3;
    //   p4.id=4;
    //   p5.id=5;

    //   InsertAtWaitingList(&p1);
    //   InsertAtWaitingList(&p2);
    //   InsertAtWaitingList(&p3);
    //   InsertAtWaitingList(&p4);
    //   InsertAtWaitingList(&p5);

    //   printarr();
    //   DeleteFromWaitingList(&p3);
    //   printarr();
    //   DeleteFromWaitingList(&p1);
    //   printarr();
    //   DeleteFromWaitingList(&p5);
    //   printarr();




    
   while(1)
   {
       int x=0;
       scanf("%d",&x);
       printf("\nnp2: %d\n",GetNP2(x));
   }
      
    









   return 0;
}
