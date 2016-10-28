#include "buffer_mgr.h"
#include "buffer_node_ll.h"
#include "storage_mgr.h"
#include "dt.h"
#include <stdio.h>
#include <stdlib.h>

/**Global Variables**/
static Nodeptr BP_StNode_ptr=NULL;//Linked List Start Ptr

static long double unitime=-32674;

/**Static Prototypes**/
static Buffer_page_Dtl *init_bfr_dtl(const int numPages);
static char *init_pg_frm(const int numPages);
static Buffer_page_Dtl *sortWeights(BM_BufferPool *const bm, BufferPool_Node *bf_node);

/***************************************Pool handling*************************************/

/*InitBufferPool*/
 
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,const int numPages, ReplacementStrategy strategy,void *stratData)
{
    RC ret=RC_OK;
    Buffer_page_Dtl *bfr_detail;
    BufferPool_Node *bfrnode;
    BM_BufferPool *tempPool;
    SM_FileHandle *sh;
    bfrnode=checkActiveBufferPools(BP_StNode_ptr,pageFileName);
    if(bfrnode==NULL)
    {
        sh=(SM_FileHandle *)malloc(sizeof(SM_FileHandle));
        if(sh==NULL)
        {
            return RC_FILE_HANDLE_NOT_INIT;
        }
        openPageFile(pageFileName,sh);
        bm->pageFile=pageFileName;
        bm->numPages=numPages;
        bm->strategy=strategy;
        bm->mgmtData=sh;

        bfr_detail=init_bfr_dtl(numPages);
        ret=bfr_detail!=NULL?RC_OK:RC_FILE_HANDLE_NOT_INIT;
        if(insert_node(&BP_StNode_ptr,bm,bfr_detail)==FALSE)
        ret=RC_FILE_HANDLE_NOT_INIT;
    }
    else
       {
            tempPool=bfrnode->buffer_pool_ptr;
            bm->pageFile=pageFileName;
            bm->numPages=numPages;
            bm->strategy=strategy;
            bm->mgmtData=tempPool->mgmtData;
            bfr_detail=bfrnode->buffer_page_dtl;
            if(insert_node(&BP_StNode_ptr,bm,bfr_detail)==FALSE)
            ret=RC_FILE_HANDLE_NOT_INIT;
       }

return ret;
}
/*ShutDown BufferPool*/
RC shutdownBufferPool(BM_BufferPool *const bm)
{
    RC ret=RC_WRITE_FAILED;
    int i = 0,*pgnumbers, *fixounts;
    bool ispinned=FALSE;
    Buffer_page_Dtl *pg_dtl;
    BufferPool_Node *bufnode;
    char *frm;
    int currentaccessed;
    fixounts=getFixCounts(bm);
    pgnumbers=getFrameContents(bm);
    while (i < bm->numPages )
    {
        if(fixounts[i] > 0)
        {
            ispinned=TRUE;			
        }
		i++;
    }
    free(fixounts);
    if(ispinned==FALSE)
    {
        ret=RC_OK;
        bufnode=search_data(BP_StNode_ptr,bm);//Get the active buffer pool from collection of pools.
        currentaccessed=getfilebeingused(BP_StNode_ptr,bm->pageFile);//Check if the same pool is being shared.
        if(bufnode!=NULL)
        {
            pg_dtl=bufnode->buffer_page_dtl;
            if(pg_dtl!=NULL)
            for(i=0;i<bm->numPages;i++)
            {
                frm=pg_dtl[i].pageframes;
                if(pg_dtl[i].isdirty==TRUE)
                {
                   ret=writeBlock(pg_dtl[i].pagenums,bm->mgmtData,frm)==RC_OK?RC_OK:RC_WRITE_FAILED;//Write the content with dirty to disk.
                }
                currentaccessed == 1 ?free(frm):' ';
            }
            currentaccessed == 1 ?free(pg_dtl):' ';
            pg_dtl=NULL;
            delete_node(&BP_StNode_ptr,bm);
        }
    currentaccessed == 1 ?closePageFile(bm->mgmtData):' ';
    currentaccessed == 1 ?free(bm->mgmtData):' ';
  }

    return ret;
}
/*ForceFlush BufferPool*/
RC forceFlushPool(BM_BufferPool *const bm)
{
    RC ret=RC_OK;
    int i = 0;
    Buffer_page_Dtl *pg_dtl;
    BufferPool_Node *bufnode;
    char *frm;

    bufnode=search_data(BP_StNode_ptr,bm);
    if(bufnode!=NULL)
        {
            pg_dtl=bufnode->buffer_page_dtl;

		    while(i< bm->numPages)		
            {
                frm=pg_dtl[i].pageframes;
                if(pg_dtl[i].isdirty==TRUE && pg_dtl[i].fixcounts==0)//Writes the data with fixcount=0
                {
                   ret=writeBlock(pg_dtl[i].pagenums,bm->mgmtData,frm)==RC_OK?RC_OK:RC_WRITE_FAILED;
                   pg_dtl[i].isdirty=FALSE;
                   bufnode->numwriteIO++;
                }
			i++;	
            }
        }

    return ret;
}
/******************************************Statistics interfaces******************************/
/*getFrameContents*/
PageNumber *getFrameContents (BM_BufferPool *const bm)
{
    PageNumber i,*pgnumber=NULL;
    Buffer_page_Dtl *bfr_detail;
    Nodeptr _bufferNode=search_data(BP_StNode_ptr,bm);
    if(_bufferNode!=NULL)
    {
        pgnumber=(PageNumber *)calloc(bm->numPages,sizeof(PageNumber));
        bfr_detail=_bufferNode->buffer_page_dtl;
        if(bfr_detail!=NULL){
        for(i=0;i < bm->numPages;i++)
        {
            pgnumber[i]=bfr_detail[i].pagenums;
        }
        }else
          {
              free(bfr_detail);
              bfr_detail=NULL;
          }

    }
    return pgnumber;
}

/*getNumReadIO*/
int getNumWriteIO (BM_BufferPool *const bm){
    int  nOwriteIO=0;
    Nodeptr _buffernode=search_data(BP_StNode_ptr,bm);
    if(_buffernode!=NULL)
    {
        nOwriteIO=_buffernode->numwriteIO;
    }
    return nOwriteIO;
}


/*getNumReadIO*/
int getNumReadIO (BM_BufferPool *const bm)
{
    int  nOreadIO=0;
    Nodeptr _bufferNode=search_data(BP_StNode_ptr,bm);
    if(_bufferNode!=NULL)
    {
        nOreadIO=_bufferNode->numreadIO;
    }

    return nOreadIO;
}

/*getFixCounts*/
int *getFixCounts (BM_BufferPool *const bm)
{
int i,*fixcounts;
Buffer_page_Dtl *bfr_detail;
Nodeptr _buffernode=search_data(BP_StNode_ptr,bm);
    if(_buffernode!=NULL)
      {
          fixcounts=(int *)calloc(bm->numPages,sizeof(int));
          bfr_detail=_buffernode->buffer_page_dtl;
          if(bfr_detail!=NULL){
          for(i=0 ; i < bm->numPages ; i++)
          {
              fixcounts[i]=bfr_detail[i].fixcounts;
          }
          }
          else
          {
              free(bfr_detail);
              bfr_detail=NULL;
          }

      }
return fixcounts;
}

/*getDirtyFlags*/
bool *getDirtyFlags (BM_BufferPool *const bm)
{
    int i;
    bool  *dirtyflags;
    Buffer_page_Dtl *bfr_detail;
    Nodeptr _bufferNode=search_data(BP_StNode_ptr,bm);
    if(_bufferNode!=NULL)
    {
        dirtyflags=(bool *)calloc(bm->numPages,sizeof(bool));
        bfr_detail=_bufferNode->buffer_page_dtl;
        if(bfr_detail!=NULL){
        for(i=0;i < bm->numPages;i++)
        {
            dirtyflags[i]=bfr_detail[i].isdirty;
        }
        }else
          {
              free(bfr_detail);
              bfr_detail=NULL;
          }
    }
return dirtyflags;
}


/********Helper Functions*********/

/*init_bfr_dtl*/
static Buffer_page_Dtl *init_bfr_dtl(const int numPages)
{
    int i;
    Buffer_page_Dtl *temp_pg_dtl=NULL;
    temp_pg_dtl=(Buffer_page_Dtl *)calloc(numPages,sizeof(Buffer_page_Dtl));//dynamic allocation.
    if(temp_pg_dtl!=NULL)
    {
        for(i=0;i < numPages;i++)
        {
            (temp_pg_dtl+i)->pageframes=init_pg_frm(numPages);
            temp_pg_dtl[i].fixcounts=0;
            temp_pg_dtl[i].isdirty=FALSE;
            temp_pg_dtl[i].pagenums=NO_PAGE;
        }
    }
    return temp_pg_dtl;
}

/*initializePageframes*/
static char *init_pg_frm(const int numPages)
{
    int i;
    char *data;
    data=(char *)malloc(PAGE_SIZE);
    if(data!=NULL)
    {
        for(i=0;i < PAGE_SIZE ; i++)
            data[i]='\0';
    }
    return data;
}

 /* *****************************Buffer Manager Interface Access Pages***************************************************************/

/*markDirty*/
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {
	int i;
	BufferPool_Node *pg_node;
	Buffer_page_Dtl *bfr_detail;
	pg_node = search_data(BP_StNode_ptr, bm);
	bfr_detail = pg_node->buffer_page_dtl;
	for (i = 0; i < bm->numPages; i++) {
		if (((bfr_detail + i)->pagenums) == page->pageNum) {
			(bfr_detail + i)->isdirty = TRUE;
			return RC_OK;
		}
	}
return RC_WRITE_FAILED;
}

/*unpinPage*/
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) {

	int totalPages = bm->numPages, i, k;
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *pg_dtl;
	pg_dtl = bf_node->buffer_page_dtl;
	for (i = 0; i < totalPages; i++) {
		if ((pg_dtl + i)->pagenums == page->pageNum) {
			if ((pg_dtl + i)->isdirty == TRUE) {
			}
			(pg_dtl + i)->fixcounts -= 1;
			return RC_OK;
		}
	}

	return RC_WRITE_FAILED; 
}

/*forcePage*/
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	if(bf_node!=NULL)
    {

	if (bm->mgmtData!= NULL) {
		writeBlock(page->pageNum, bm->mgmtData, page->data);
		bf_node->numwriteIO++;
	} else {
		return RC_FILE_NOT_FOUND;
	}
    }
	return RC_OK;
}

/*pinPage*/
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) {
	int replacementFrameNum, totalPages, i;
	int statusFlag;
	ReplacementStrategy strategy = bm->strategy;
	Buffer_page_Dtl *lowestPossiblePage;
	totalPages = bm->numPages;
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *pg_dtl;
	pg_dtl = bf_node->buffer_page_dtl;
	RC writeVal=1, readVal=1;
	int emptyFlag = 1;
	if (pg_dtl != NULL) {

		/*check if the page is already present in the buffer*/
		for (i = 0; i < totalPages; i++) {
					if ((pg_dtl + i)->pagenums > -1) { 
						emptyFlag = 0;
				/*check if the page is already present in buffer*/
				if ((pg_dtl + i)->pagenums == pageNum) {
                    (pg_dtl + i)->timeStamp=unitime++;
                    page->pageNum = pageNum;
					page->data = (pg_dtl + i)->pageframes;
					(pg_dtl + i)->fixcounts+=1;
					return RC_OK; 
				}

			}
		} 

		/*Getting the first lowestPossible page and assigns it*/
		for (i = 0; i < totalPages; i++) {
			if ((pg_dtl + i)->pagenums == -1){
				lowestPossiblePage = ((pg_dtl + i)); 
				emptyFlag = 1;
				break;
			}
		}

	} else { 
		lowestPossiblePage = (pg_dtl + 0); 
		lowestPossiblePage->replacementWeight = lowestPossiblePage->replacementWeight + 1;

		(pg_dtl + i)->timeStamp=unitime++;
		emptyFlag = 1;
	}

		if (emptyFlag == 1) {
		page->pageNum = pageNum;
		page->data = lowestPossiblePage->pageframes;
		
		writeVal = RC_OK;
		
		if (lowestPossiblePage->isdirty == TRUE) {
			writeVal = writeBlock(pageNum, bm->mgmtData,
					lowestPossiblePage->pageframes);
					bf_node->numwriteIO++;
		}
		if(readBlock(pageNum, bm->mgmtData,lowestPossiblePage->pageframes)==RC_READ_NON_EXISTING_PAGE)
				{
				    readVal=appendEmptyBlock(bm->mgmtData);
				}
				else
                    readVal=RC_OK;
        bf_node->numreadIO++;
		lowestPossiblePage->fixcounts += 1;
		lowestPossiblePage->pagenums = pageNum;

	} else { 
		if (strategy == RS_FIFO) {
			statusFlag = applyFIFO(bm, page, pageNum);
		} else if (strategy == RS_LRU) {
			statusFlag = applyLRU(bm, page, pageNum);
                } else {
			return RC_WRITE_FAILED; // should probably return "Invalid strategy"
		}
	} 
	
	if (statusFlag == 1 || (writeVal == RC_OK && readVal == RC_OK)) {
		return RC_OK;
	} else {
		return RC_WRITE_FAILED;
	}
}

/* ********************************** Custom - Page Replacement methods********************************************/
/*applyFifo*/
int applyFIFO(BM_BufferPool * const bm, BM_PageHandle * const page, const PageNumber pageNum) {
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *lowestPossiblePage=NULL;
	lowestPossiblePage = sortWeights(bm, bf_node);

	if(lowestPossiblePage==NULL) return 0;
	char *replacementAddress;
	replacementAddress = lowestPossiblePage->pageframes;

	RC writeVal = RC_OK;
	RC readVal = RC_OK;
	if (lowestPossiblePage->isdirty == TRUE) {
		writeVal = writeBlock(lowestPossiblePage->pagenums, bm->mgmtData, replacementAddress);
		lowestPossiblePage->isdirty=FALSE;
		bf_node->numwriteIO++;
	}
	readVal = readBlock(pageNum, bm->mgmtData, replacementAddress);
	if(readVal==RC_READ_NON_EXISTING_PAGE) readVal =appendEmptyBlock(bm->mgmtData);
    bf_node->numreadIO++;
	page->pageNum  = pageNum;
	page->data = lowestPossiblePage->pageframes;
    lowestPossiblePage->pagenums = pageNum;
    lowestPossiblePage->fixcounts+=1;
	lowestPossiblePage->replacementWeight = lowestPossiblePage->replacementWeight + 1;
	if (readVal == RC_OK && writeVal == RC_OK)
		return 1; //success flag
	else
		return 0;
}

/*applyLRU*/
int applyLRU(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *lowestPossiblePage;
	lowestPossiblePage = sortWeights(bm, bf_node);
	char *replacementAddress;
	replacementAddress = lowestPossiblePage->pageframes;

	RC writeVal = RC_OK;
	RC readVal = RC_OK;
	if (lowestPossiblePage->isdirty == TRUE) {
		writeVal = writeBlock(page->pageNum, bm->mgmtData, replacementAddress);
		lowestPossiblePage->isdirty=FALSE;
		bf_node->numwriteIO++;
	}
	readVal = readBlock(pageNum, bm->mgmtData, replacementAddress);
	page->pageNum  = pageNum;
	page->data = lowestPossiblePage->pageframes;
    lowestPossiblePage->pagenums = pageNum;
    lowestPossiblePage->fixcounts+=1;
	lowestPossiblePage->replacementWeight =
    lowestPossiblePage->replacementWeight + 1;
	lowestPossiblePage->timeStamp =(long double)unitime++;
    bf_node->numreadIO++;
	if (readVal == RC_OK && writeVal == RC_OK)
		return 1; //success flag
	else
		return 0;
}


/*sortWeights*/

Buffer_page_Dtl *sortWeights(BM_BufferPool * const bm, BufferPool_Node *bf_node) {
	int i= 0;
	
	int j =0;
	int k = 0;
	int totalPagesInBlock = bm->numPages;
	Buffer_page_Dtl *bf_page_dtl = bf_node->buffer_page_dtl;
	Buffer_page_Dtl *bf_page_dtl_with_zero_fixcount[totalPagesInBlock];
	int cnt = 0;

	while(i < totalPagesInBlock) {
		
		bf_page_dtl_with_zero_fixcount[i] = NULL;
		i++;
	}	
			
	while (j < totalPagesInBlock){
		if ((bf_page_dtl[j].fixcounts) == 0) {
		     bf_page_dtl_with_zero_fixcount[cnt] = (bf_page_dtl+j);
		           cnt++;
		}
		j++;
	}

	/*Macro to determine the size of the newly created pointer array*/
    #define sizeofa(array) sizeof array / sizeof array[ 0 ]
    int sizeOfPagesWithFixcountZero = sizeofa(bf_page_dtl_with_zero_fixcount);

	/*find the page with lowest replacementWeight*/
	Buffer_page_Dtl *next_bf_page_dtl;
	Buffer_page_Dtl *replacement_bf_page_dtl ;
	replacement_bf_page_dtl = bf_page_dtl_with_zero_fixcount[0];

      while (k < sizeOfPagesWithFixcountZero)
		{
		next_bf_page_dtl = bf_page_dtl_with_zero_fixcount[k];
		if(next_bf_page_dtl!=NULL){
			if(bm->strategy == RS_FIFO){
				if ((replacement_bf_page_dtl->replacementWeight) > (next_bf_page_dtl->replacementWeight))
						replacement_bf_page_dtl = next_bf_page_dtl;
			}
			else if(bm->strategy==RS_LRU){
	            if(replacement_bf_page_dtl->timeStamp > next_bf_page_dtl->timeStamp)
				{
					replacement_bf_page_dtl = next_bf_page_dtl;
				}
			}
		}
		k++;
	}
return replacement_bf_page_dtl;
}
