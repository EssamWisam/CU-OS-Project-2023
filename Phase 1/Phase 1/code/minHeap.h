#include <stdio.h>
#include <stdlib.h>
#include "headers.h"
//https://robin-thomas.github.io/min-heap/ 
//This guide has helped implement a signification part of this.

//We can deduce the following from the array representation of the minHeap:
#define left(i) 2*i + 1
#define right(i) 2*i + 2
#define parent(i) (i-1) / 2

//One node of the minheap
typedef struct node 
{
struct process Proc;
int data;

} node ;

//The whole minheap
typedef struct minHeap 
{
    int size ;
    int criterion;
    node *curr ;
} minHeap ;

boolean lessThan(struct process a, struct process b, int criterion)
{
    switch(criterion)
    {
        case 2:
        {
            return (a.runtime<b.runtime);

        }
        case 3:
        {
            return (a.priority<b.priority);
        }

        case 4:
        {
            return (a.remTime<b.remTime);

        }
    }
}

//The minHeap's constructor.
minHeap initMinHeap(int crit) {
    minHeap HP ;
    HP.size = 0 ;
    HP.criterion = crit;
    return HP ;
}

struct process extractMin(minHeap *HP)
{
    return (HP->curr[0].Proc);
}


void insertNode(minHeap *HP, struct process P) 
{
    
    HP->curr = (HP->size==0)? malloc(sizeof(node)):realloc(HP->curr, (HP->size + 1) * sizeof(node)) ;
    //Initially it has zero, so we malloc otherwise, we need to expand so we realloc what we previously malloced and increment size.

    // initializing the node:
    node insertMe ;
    insertMe.Proc = P ;

    //Finding the right location for the node.
    int i = (HP->size)++;                                           //The node should start at the new location, but this might not be the right spot.
    while(i && lessThan(P, HP->curr[parent(i)].Proc, HP->criterion) )  //As long as we're not in the root yet and parent is larger (not min). 
    {
        HP->curr[i] = HP->curr[parent(i)] ;                         //Then the parent doesn't deserve to be there, nerf it (our node is more worthy.)
        i = parent(i) ;                                             //so we reserve its location and see if we can go up any further.
    }
    //Inserting the note at the right location (we can't go back any further, the right location was found.)
    HP->curr[i] = insertMe ;

}



//Will be needed for Heapify and decrease key.
void swap(node *n1, node *n2) 
{
    node temp = *n1 ;
    *n1 = *n2 ;
    *n2 = temp ;
}



//Just like we did for insertion, but no holes this time so we need to swap.
void decreaseKey(minHeap *HP,int i, struct process P)                           //decrease the value of some node.
{
    HP->curr[i].Proc = P;
    while (i != 0 && lessThan(P,HP->curr[parent(i)].Proc, HP->criterion))
    {
       swap(&HP->curr[i], &HP->curr[parent(i)]);
       i = parent(i);
    }
}


//continuously swap the parent larger child if it's smaller.
void heapify(minHeap *HP, int i)                            //i is the root of the sub tree we want to sort.
{
    //Picking the right or the left child (the smaller one.), the first condition is to check existence.
    int smallest = ( (left(i) < HP->size) && lessThan(HP->curr[left(i)].Proc, HP->curr[i].Proc, HP->criterion) ) ? left(i) : i ;
    //Time to check the right child:
    smallest = ( (right(i) < HP->size) && lessThan(HP->curr[right(i)].Proc , HP->curr[smallest].Proc,HP->criterion))?right(i):smallest;

    if(smallest != i)                                       //If one of the above were satisfied >> time to fix.
    {
        swap(&(HP->curr[i]), &(HP->curr[smallest])) ;
        heapify(HP, smallest);                              //See if the rest of the subtree is fine with the change.
    }
}


void deleteNode(minHeap *HP)                                        //deletes the root.
{
    if(HP->size) 
    {
        HP->curr[0] = HP->curr[--(HP->size)] ;                      //Replace what we have deleted  with the last node in the heap.
        HP->curr = realloc(HP->curr, HP->size * sizeof(node)) ;     //reduce the size of the heap.
        heapify(HP, 0);                                            //What we did two steps ago might require a fix.
    } else 
    {
        free(HP->curr);
    }
}


int dynamicDelete(minHeap *HP, int i, struct process P)       //i is the root to start from, this does inOrder traversal printing runtime.
{
    //(Left subtree -> Root -> Right subtree)
    //Check Left.
    if(left(i) < HP->size) 
    {
        dynamicDelete(HP, left(i),P) ;
    }
    //Node to be deleted found:
    if(HP->curr[i].Proc.id==P.id)
    {   
        if(HP->criterion==2)
    {
        HP->curr[i].Proc.runtime=-1;
        decreaseKey(HP,i,HP->curr[i].Proc );           //Now it's the root.
        deleteNode(HP);
    }
      
    }
    //Check Right.
    if(right(i) < HP->size) 
    {
        dynamicDelete(HP, right(i),P) ;
    }
}
