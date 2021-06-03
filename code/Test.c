#include <stdio.h>
#include "minHeap.h"

int main() 
{
    
   minHeap HP = initMinHeap(1024,3);
    struct process X;
    X.runtime=12;
    X.arrival=15;
    X.priority=6;
    X.remTime=18;

    struct process Y;
    Y.runtime=1;
    Y.arrival=-15;
    Y.priority=18;
    Y.remTime=1;

    struct process Z;
    Z.runtime=5;
    Z.arrival=-15;
    Z.priority=-6;
    Z.remTime=12;

    struct process Q;
    Q.priority=30;



   insertNode(&HP, X);
   insertNode(&HP, Y);
   insertNode(&HP, Z);
   insertNode(&HP,Q);
   deleteNode(&HP);
   //inorderTraversal(&HP,0);
    
    printf("%d", (extractMin(&HP).priority)); ;

   return 0;
}
