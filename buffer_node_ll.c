#include "buffer_node_ll.h"
#include <stdio.h>
#include <stdlib.h>

/*******Buffer Linked List implementation********/
//Create new node and insert to the linked list at last.
bool insert_node(Nodeptr *stnode ,  void *buffer_pool_ptr , void *buffer_page_handle)
{
    bool ret=FALSE;
    Nodeptr new_ptr;
    Nodeptr prev_ptr;
    Nodeptr crnt_ptr;
  
    new_ptr =(Nodeptr)malloc(sizeof(BufferPool_Node));
    if(new_ptr!=NULL)
    {
        new_ptr->buffer_pool_ptr=buffer_pool_ptr;
        new_ptr->buffer_page_dtl=buffer_page_handle;
        new_ptr->numreadIO=0;
        new_ptr->numwriteIO=0;
        new_ptr->nextBufferNode=NULL;

        prev_ptr=NULL;
        crnt_ptr =*stnode;

        while (crnt_ptr!=NULL )
        {
            prev_ptr=crnt_ptr;
            crnt_ptr=crnt_ptr->nextBufferNode;
        }
        if(prev_ptr==NULL)
        {
            *stnode=new_ptr;
        }
        else
        {
            prev_ptr->nextBufferNode=new_ptr;
        }
        ret=TRUE;
    }
   else
    {
        printf("Memory not available");
    }
  
    return ret;
}
//Used to print values in linked list.
void print_list(Nodeptr startptr)
{
    Nodeptr prev_ptr;
    Nodeptr crnt_ptr;
    BM_BufferPool *buffnode;
    prev_ptr=NULL;
    crnt_ptr=startptr;
    if(crnt_ptr==NULL)
    {
        printf("List is empty");
    }
    else
    {
        while(crnt_ptr!=NULL)
        {
            prev_ptr=crnt_ptr;
            crnt_ptr=crnt_ptr->nextBufferNode;
            buffnode=(BM_BufferPool *)prev_ptr->buffer_pool_ptr;

            printf("%s ->  ",buffnode->pageFile);
        }
    }
}
//Delete the node from the linked list using given data.
bool delete_node(Nodeptr *nodeptr ,  void *buffer_pool_ptr )
{
    Nodeptr temptr;
    Nodeptr prev_ptr;
    Nodeptr crnt_ptr;
    prev_ptr=NULL;
    crnt_ptr=*nodeptr;


    while(crnt_ptr!=NULL && crnt_ptr->buffer_pool_ptr!=buffer_pool_ptr)
    {
        prev_ptr=crnt_ptr;
        crnt_ptr=crnt_ptr->nextBufferNode;
    }

    if(crnt_ptr!=NULL)
    {
        temptr=crnt_ptr;

        if(prev_ptr== NULL)
        {
            *nodeptr=crnt_ptr->nextBufferNode;
        }
        else
        {
            prev_ptr->nextBufferNode=crnt_ptr->nextBufferNode;
        }
        free(temptr);
    }
    else
    {
        printf("Item not in the list");
    }

    return TRUE;

}
//Get the corresponding node for the given buffer pool pointer.
BufferPool_Node *search_data(Nodeptr nodeptr,  void *buffer_pool_ptr )
{
    Nodeptr prev_ptr=NULL;
    Nodeptr crnt_ptr=nodeptr;
    while(crnt_ptr!=NULL)
    {
        if(crnt_ptr->buffer_pool_ptr==buffer_pool_ptr)
        {
            break;
        }
        prev_ptr=crnt_ptr;
        crnt_ptr=crnt_ptr->nextBufferNode;
    }
    if(crnt_ptr==NULL)
    {
        printf("Item not available");
    }
    return crnt_ptr;
}
//Check number of pools for same files.
BufferPool_Node *checkActiveBufferPools(Nodeptr stnode,char *filename)
{
    Nodeptr prev_ptr=NULL;
    Nodeptr crnt_ptr=stnode;
    BM_BufferPool *bufferpool;
    while(crnt_ptr!=NULL)
    {
        bufferpool=(BM_BufferPool *)crnt_ptr->buffer_pool_ptr;
        if(bufferpool->pageFile==filename)
        {
           break;
        }
        prev_ptr=crnt_ptr;
        crnt_ptr=crnt_ptr->nextBufferNode;
    }
    return crnt_ptr;
}
//Get the number of pools totally for a same file.
int getfilebeingused(Nodeptr stnode,char *filename)
{
    Nodeptr prev_ptr=NULL;
    Nodeptr crnt_ptr=stnode;
    BM_BufferPool *bufferpool;
    int count=0;
    while(crnt_ptr!=NULL)
    {
        bufferpool=(BM_BufferPool *)crnt_ptr->buffer_pool_ptr;
        if(bufferpool->pageFile==filename)
        {
         count++;
        }
        prev_ptr=crnt_ptr;
        crnt_ptr=crnt_ptr->nextBufferNode;
    }
    return count;
}
