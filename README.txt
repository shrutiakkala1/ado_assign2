                                                                      README
                                                                  BUFFER MANAGER

CONTENTS:-

1.Design overview
2.Implementation overview
3.Description of fuctions

/****Design overview****/

The following structures are used and its functions are:
BM_BufferPool-It is used to initiate a pool for a given file.
BufferPool_Node-Linked list node to insert ,delete and search .It even serves to map the pool and its frames.
Buffer_Page_Dtl-This structure is used to contain the page frames to a buffer pool.
Linked list is created to maintain the active buffer pools.Each of the linked list node,we have buffer pool pointer and buffer page dtlepointer so that it serves as map between frame and the pool.Other functions with respect to frames such as pgnum,dirty flag fixcount are also present in buffer page dtl.

/****implementation overview****/
While the buffer pool is created,it is inserted into the buffer node of a linked list.
At any time ,Linkedlist nodes will contain all active buffer pools.
If two or more pools are initiated for the same file,we contain two different buffer pool pointers and share the same page frames.
On shutdown operation,we delete the node that contains the active pool.
FIFO-This technique is done using sorted weights.
LRU-This technique is done using timestamp.

/****Description of functions****/

Name-markDirty
Expected arguments-BM_BufferPool *const bm,BM_PageHandle *const page
Behavior-Will mark a particular page from a particular BufferPool as dirty

1)Get page handle
2)Mark the frame/page at the given handle as dirty.
3)Return the appropriate message.

Name-unpinPage
Expected Arguments:BM_BufferPool  *const bm,BM_PageHandle *const page
Behavior-Would check whether a  page is dirty .If its dirty ,this fuction would write the change back to the file and unpin it from the buffer pool.If its not dirty,this function will simply write the change back to the file and unpin the page from buffer pool.

1)Check whether the page is to be unpinned has fixcounts==0
2)If not throw error
3)If fixcounts==0,check if isDirty==TRUE
4)If isDirty==TRUE,forcePage
5)Free the page from the memory reference

Name-forcePage
Expected arguments-BM_Bufferpool *const bm,BM_PageHandle *const page

1)Get the to be written to the fiel on disk
2)Write the contents of the page to the file on disk.
3)Return the appropriate message

Name-pinPage
Expected arguments-BM_BufferPool *const bm,BM_PageHandle *const page,covnst pageNumber pageNum
Behavior-Perform the applicable page replacement stratergy on the buffer pool and pin the page from file at the appropriate position.

1)Check whether the frames are empty.
2)If any page is found as empty ,use that for the replacement strategy.If not check if the page is already present in buffer.
3)If not found empty add appropriate replacement weight/timestamp to each page.
4)Get the appropriate replacement page by invoking the corresponding startegy.
5)Replace that page with the new page and update the replacement parameters.

/****Custom-Page Replacement methods****/

Name-applyFIFO
Expected arguments-BM_BufferPool
Behavior-Apply FIFO as page replacement strategy on the buffer pool and return us the appropriate page to be replaced.

1)Add replacement weights whenever a page is added into frame.
2)The page which gets added at the initial has the least replacemnet weights and the page added last would have the heighest weight.
3)Iterate this process in such a manner that maximum weight of the page equals the number of frames.
4)Replace the page with the least weight which also means it is the first in page.

Name-applyLRU
Expected Arguments-BM_BufferPool
Behavior-Apply LRU as page replacement startegy on the buffer pool and return us the appropiate page to be replaced.

1)Add  timestamps whenever a page is added to the frame or accessed.
2)The page which is added first would have oldest timestamp and the page added last would have the latest timestamp.
3)Replace the page with the oldest timestamp which also means that it is the least recetly used page.


/****Pool Handling****/

Name-InitBufferPool
Behavior-To initialize a new buffer pool and if buffer pool exists,i share with new pool handler.
1)Dynamic allocation of a linked list node.
2)Initialize the page frames depending on the buffer pool num pages.
3)Insert the active pool and page frames into the linked list node.

Name-ShutDown BufferPool
Behavior-To shutdown an existing buffer pool
1)Get the buffer page dtl for the pool to be shutdown.
2)If false ,check for any dirty page frames.If true,write it to the disc.
4)Free the resources associated with the pool.

Name-ForceFlush BufferPool
Behavior-Write the data to the disk forecefully with fix count=0

1)Get the buffer page dtle for the current pool 
2)Check for any dirty page frames.If present and has got fixcount=0,write it to the disc.


/*****Statistics interfaces*****/

Name-getFrameContents
Behavior-It would return an array of pointers with array size equals total number of frames in buffer pool.Each array value equals page numbers.

1)Get all the page frames for the given buffer pool.
2)Read the page number of corresponding frames,put them in an array and return to the client.

Name-getDirtyFlags
Behavior-It will return an array pointer with array size equals toatl number of frames in buffer pool.Each array value equals true or false with respect to corresponding page.

1)Get all the page frames for the given buffer pool.
2)Read the dirty flag of corresponding frames,put them in an array and return to the client.

Name-getNumReadIO
Behavior-Will return total number of reads that has taken place.

1)Get the buffer node containing the buffer pool.
2)Return the total write IO of the pool from buffer node.

Name-getNumWriteIO
Behavior-will return total number of reads that has taken place.

1)Get the buffer node containing the buffer pool.
2)Return the total write IO of the poool from buffer node.

Name-getFixCounts
Behavior-Will return an array pointer with array size equals total number of frames in buffer pool.Each array value equals fixcounts of a frame.

1)Getall the pages frames for the given buffer pool.
2)Read the fixcounts of corresponding frames,put them in an array and return to the client .
	