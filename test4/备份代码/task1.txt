#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<sys/time.h>
#include<unistd.h>
#include<stdlib.h>
using namespace std;
#define BUSY 1//占用块标志 
#define IDLE 0//空闲块标志 
#define COLOR_Exist 0//存在该页面，命中 
#define COLOR_NotExist_IDLE 1//不存在该页面，但有空闲块 
#define COLOR_NotExist_NoIDLE 2//不存在该页面，且不存在空闲块 
int pages[1000];
//页面数据结构
typedef struct _Page {
    int pageID;//页号
} Page; 
//页面队列
typedef struct _PageQueue {
    Page page;
    struct _PageQueue *next;//指针代表下一页面
} PageQueue;
//块队列中基本元素
typedef struct _Block { 
    Page *page; //页框 
    long time; //最后访问时间
    int state; //页块是否空闲，0代表未使用，1代表已被使用
	int A;// 访问位，如果访问位为1则再给一次机会，如果为0则直接替换 
	int visit;
} Block;
//块队列以链表表示
typedef struct _BlockQueue {
    Block block;
    struct _BlockQueue *next;
} BlockQueue;
//描述整体的进程结构
typedef struct process {
    PageQueue pages; //页面
    unsigned int pageLength; // 页面数
} process;//进程

int t = 0;

BlockQueue *InitBlockQueue(int size);//初始化内存物理块队列，把首地址返回，如果分配失败返回NULL
int GetBlockQueueSize(BlockQueue *blockQueue);//获取块长度
void ClearBlockQueue(BlockQueue *blockQueue);//清空块内容
PageQueue *InitializePageQueueWithInput(unsigned int pageLength);//初始化页面，把首地址返回。如果分配失败，返回NULL
void InitializeProcessWithInput(process *proc, unsigned int pageSize);//初始化进程，接收手动输入的页面访问序列
BlockQueue *SearchPage(BlockQueue *blockQueue, Page page);//搜索目标指定页面
BlockQueue *SearchIdleBlock(BlockQueue *blockQueue);//搜索空闲块
int GetBlockLable(BlockQueue *blockQueue, BlockQueue *goalBlock);//返回块号
BlockQueue *GetOldestBlock(BlockQueue *blockQueue);//查找停留最久页面 
BlockQueue *GetLongestWithoutAccess(BlockQueue *blockQueue, PageQueue *currentPage);//OPT算法
BlockQueue *GiveSecondChance(BlockQueue *blockQueue);//SCR算法
BlockQueue *GetClockBlock(BlockQueue *blockQueue,PageQueue *currentPage);//获取clock算法中的块队列元素信息
void PrintBlockList(BlockQueue *blockQueue, int pageID, int color);//打印物理块块信息
void FIFO(BlockQueue *blockQueue, process *proc);//先进先出算法，选择在内存中驻留时间最久的页面予以淘汰，即调用GetOldestBlock函数 
void SCR(BlockQueue *blockQueue, process *proc);//SCR算法
void LRU(BlockQueue *blockQueue, process *proc);//LRU最近最久未使用
void OPT(BlockQueue *blockQueue, process *proc);//OPT最佳页面替换算法
void Clock(BlockQueue *blockQueue, process *proc);//时钟置换算法
void Menu();//基本展示菜单
int True,False;

int main() {
    while(1){
    	int t=0; 
        True=False=0;
        unsigned int blockNum;//物理块数
        unsigned int pageNum;//进程页面数 
        printf("请输入主存的物理块数(页框个数)：");
        scanf("%u", &blockNum);//3
        printf("请输入程序长度(页面数)：");
        scanf("%u", &pageNum);//8
        BlockQueue *blocks;
        process proc;
        cout<<"请输入页面序列："<<endl; 
         for (int i = 0; i < pageNum; ++i) {
                scanf("%d",&pages[i]);//0 7 6 5 7 4 7 6
            }
        InitializeProcessWithInput(&proc,pageNum);
        blocks = InitBlockQueue(blockNum);
        Menu();
        int oper;
        while (scanf("%d",&oper)){
            int flag = 0;
            printf("\n===========================================================\n");
            for (int i = 1; i<=blockNum; i++) {
            	printf("| 物理块%d  | ",i);
            }
            printf("\n");
            switch (oper){
                case 1:
                    OPT(blocks, &proc);
                    break;
                case 2:
                    FIFO(blocks, &proc);
                    break;
                case 3:
                    LRU(blocks, &proc);
                    break;
                case 4:
                	SCR(blocks, &proc);
                    break;
                case 5:
                	Clock(blocks, &proc);
                	break;
                case 0:
                    flag = 1;
                    break;
                default:
                    Menu();
                    printf("非法输入!请重新输入：");
                    break;
            }
            if(flag!=1) printf("缺页率为%.2f\n",(1.0*False)/(True+False));
            if (flag == 1) break;
            ClearBlockQueue(blocks);
            Menu();
        } 
        printf("是否重新输入程序页面序列? y/n\n");
        getchar();
        char c=getchar();
        if(c=='y') continue;
		else break;		 
    }
}

//初始化内存物理块队列，把首地址返回，如果分配失败返回NULL
BlockQueue *InitBlockQueue(int size) {
    BlockQueue *block = NULL, *p = NULL, *q = NULL;
    for (int i = 0; i < size; i++) {
        p = (BlockQueue *) malloc(sizeof(BlockQueue));
        p->block.page = NULL;//页面 
        p->block.state = 0;//页块空闲 
        p->block.time = 0;//最后访问时间 
        p->block.A = 0;
        p->next = NULL;
        if (block == NULL) block = p;//头结点 
        else q->next = p;
        q = p;
    } 
    return block;//返回首地址 
}
//获取块长度
int GetBlockQueueSize(BlockQueue *blockQueue) {
    BlockQueue *currentBlock = blockQueue;
    int blockQueueSize = 0;
    while (currentBlock != NULL) {
        blockQueueSize++;
        currentBlock = currentBlock->next;
    }
    return blockQueueSize;
}
//清空块内容
void ClearBlockQueue(BlockQueue *blockQueue) {
    BlockQueue *currentBlock = blockQueue;
    while (currentBlock != NULL) {
        currentBlock->block.page = NULL;
        currentBlock->block.state = IDLE;
        currentBlock->block.time = 0;
        currentBlock = currentBlock->next;
    }
}
//初始化页面，把首地址返回。如果分配失败，返回NULL
PageQueue *InitializePageQueueWithInput(unsigned int pageLength) {
    PageQueue *head = NULL, *p = NULL, *q = NULL;
    for (int i = 0; i < pageLength; i++) {
        p = (PageQueue *) malloc(sizeof(PageQueue));
        p->page.pageID = pages[i];
        p->next = NULL;
        printf("%d ", p->page.pageID);
        if (head == NULL) head = p;
        else q->next = p;
        q = p;
    }
    printf("\n");
    return head;
}
//初始化进程，接收手动输入的页面访问序列
void InitializeProcessWithInput(process *proc, unsigned int pageSize) {
    printf("进程初始化：\n");
    proc->pageLength = pageSize;
    proc->pages.next = InitializePageQueueWithInput(pageSize);
}
//搜索目标指定页面
BlockQueue *SearchPage(BlockQueue *blockQueue, Page page) {
    BlockQueue *p = blockQueue;
    while (p != NULL) {
        if (p->block.page != NULL) {
            if (p->block.page->pageID == page.pageID)
                return p;
        }
        p = p->next;
    }
    return NULL;
}
//搜索空闲块
BlockQueue *SearchIdleBlock(BlockQueue *blockQueue) {
    BlockQueue *p = blockQueue;
    while (p != NULL) {
        if (p->block.state == IDLE) return p;//IDLE空闲标志 
        else p = p->next;
    }
    return NULL;
}
//返回块号
int GetBlockLable(BlockQueue *blockQueue, BlockQueue *goalBlock) {
    BlockQueue *p = blockQueue;
    int count = 1;
    while (p != goalBlock) {
        p = p->next;
        count++;
    }
    return count;
}
//查找停留最久页面 
BlockQueue *GetOldestBlock(BlockQueue *blockQueue) {
    BlockQueue *p = blockQueue, *oldestAddress;
    if (p == NULL) return p;
    oldestAddress = p;
    long long min = p->block.time;
    while (p != NULL) {
        if (p->block.time < min) {
            min = p->block.time;
            oldestAddress = p;
        }
        p = p->next;
    }
    return oldestAddress;
}
//检索页面序列，找出未来最长时间内不再被访问的页面（OPT算法）
BlockQueue *GetLongestWithoutAccess(BlockQueue *blockQueue, PageQueue *currentPage){ 
    BlockQueue *p = blockQueue, *longestAddress;//p：内存的物理块 
    PageQueue *q = currentPage->next;//q:进程页面 
    if (p == NULL) return p;
    longestAddress = p;
    int max= 0;
    while (p != NULL){
        int count = 0;
        while(q != NULL){
            count++;//记录该页面未来不会被访问的时长 
            /*找到未来的进程将访问该物理块存放的页面，记录终止，
			当前计数为未来将不会使用的最长时间 */ 
            if(p->block.page->pageID == q->page.pageID) break;
			
            q = q->next;
        }
        if(count > max){ //记录未来最长时间不会访问的页面 
            max = count;
            longestAddress = p;
        }
        q = currentPage->next;
        p = p->next;
    }
    return longestAddress;
}
//SCR算法
BlockQueue *GiveSecondChance(BlockQueue *blockQueue){ 
    BlockQueue *p=blockQueue, *oldestAddress,*head;
    head=p;
    oldestAddress = p;
    int len=GetBlockQueueSize(blockQueue);
    if (p == NULL) return p;
    int j=1,flag=0;
    long long min= 65535;
    do
    {
    	if(flag==1)
    	{ 
    		oldestAddress->block.A=0;//得到第二次机会，清0
    		p=head;
    		j++;//开始找下一个FIFO页面 
    		min=65535;
		}
		if(j>len)// 所有页面都找过一轮 
		{
			j=1;
			p=head;
			for (int i = 0; i < len; i++)
			{
				p->block.visit=0;
				p=p->next;
			}
			p=head;
		}
    	while (p != NULL) {
        if (p->block.time < min&&p->block.visit!=1) {
            min = p->block.time;
            oldestAddress = p;
        }
        p = p->next;
    }
    	oldestAddress->block.visit=1;
    	flag=1;
	}while(oldestAddress->block.A==1);
	return oldestAddress;
}
//获取clock算法中的块队列元素信息
BlockQueue *GetClockBlock(BlockQueue *blockQueue,PageQueue *currentPage){
	BlockQueue *p = blockQueue,*clockAddress;
	PageQueue *q = currentPage;//q 
    clockAddress = p;
	while(1){
		if (p == NULL) p = blockQueue;
		if (p->block.A == 1){
			p->block.A = 0;
			p = p->next;
		}
		else{
			clockAddress = p;
			return clockAddress;
		}	
	}
}
//打印物理块块信息
void PrintBlockList(BlockQueue *blockQueue, int pageID, int color) {
    int flag=0;
    BlockQueue *currentBlock = blockQueue;
    int noteblock;//记录命中、替换及直接加入的块号 
    for (int i = 0; i < GetBlockQueueSize(blockQueue); i++) {
        if (currentBlock == NULL) break;
        if (currentBlock->block.state == IDLE) {
            printf("|          | ");
        } else {
            if (currentBlock->block.page->pageID != pageID) {
                printf("|     %d    | ", currentBlock->block.page->pageID);
            } else {
            	noteblock= GetBlockLable(blockQueue, currentBlock); 
                switch (color) {
                    case COLOR_Exist:
                    	flag=1;
                        printf("|     %d    | ", pageID); 
                        break;
                    case COLOR_NotExist_IDLE:
                    	flag=2;
                        printf("|     %d    | ", pageID);
                        break;
                    case COLOR_NotExist_NoIDLE:
                    	flag=3;
                        printf("|     %d    | ", pageID);
                        break;
                    default:
                        break;
                }
            }
        }
       // cout<<currentBlock->block.A<<" ";
        currentBlock = currentBlock->next;
    }
    if(flag==1)
    {
        True++;
    	printf("\n  说明：%d 号进程页面命中，主存已存在该页面，物理块号为 %d \n",pageID,noteblock);
	}
	 if(flag==2)
    {
        False++;
    	printf("\n  说明：%d 号进程缺页，主存中不存在该页面，调入 %d 号空闲物理块\n",pageID,noteblock);
	}
	if(flag==3){
        False++;
		printf("\n  说明：%d 号进程缺页，主存中无空闲空间，置换物理块号为 %d 号的页面\n",pageID,noteblock);
	}
    printf("\n===========================================================\n");
}
//先进先出算法，选择在内存中驻留时间最久的页面予以淘汰，即调用GetOldestBlock函数 
void FIFO(BlockQueue *blockQueue, process *proc) {
    PageQueue *currentPage = proc->pages.next;//当前程序要使用的页面 
    while (currentPage != NULL) {
        if (SearchPage(blockQueue, currentPage->page) ) {//存在该页面，命中！ 
            PrintBlockList(blockQueue, currentPage->page.pageID, COLOR_Exist);
        } else {//不存在该页面 
            BlockQueue *idleBlock = SearchIdleBlock(blockQueue);//查找是否有空闲的物理块，并将地址返回给idleBlock 
            if (idleBlock != NULL) {//有空闲物理块 
            	idleBlock->block.page = (Page *) malloc(sizeof(Page));
                idleBlock->block.state = BUSY;
                idleBlock->block.time = t++;//最后访问时间++ 
                idleBlock->block.page->pageID = currentPage->page.pageID;//存入当前页面号 
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_IDLE);//输出物理块占用情况 
            } else {
                idleBlock = GetOldestBlock(blockQueue);//没有空闲物理块，则调用替换在内存中驻留最久的页面的算法（FIFO） 
                idleBlock->block.time = t++;//最后访问时间++ 
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue,currentPage->page.pageID, COLOR_NotExist_NoIDLE);
            }
        }
        currentPage = currentPage->next;
    }
}
//SCR算法
void SCR(BlockQueue *blockQueue, process *proc) {
    PageQueue *currentPage = proc->pages.next;
    while (currentPage != NULL) {
        BlockQueue *searchedBlock = SearchPage(blockQueue, currentPage->page);//查找当前页面是否存在 
        if (searchedBlock != NULL) {
            searchedBlock->block.A = 1;//命中该页面，访问位置1（与FIFO区别之处) 
            PrintBlockList(blockQueue, currentPage->page.pageID, COLOR_Exist);
        } else {
            BlockQueue *idleBlock = SearchIdleBlock(blockQueue);
            if (idleBlock != NULL) {
                idleBlock->block.state = BUSY;
                idleBlock->block.time = t++;
                idleBlock->block.A = 1;
                idleBlock->block.page = (Page *) malloc(sizeof(Page));
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_IDLE);
            } else {
                idleBlock = GiveSecondChance(blockQueue);
                idleBlock->block.time = t++;
                idleBlock->block.A = 1;
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue,currentPage->page.pageID, COLOR_NotExist_NoIDLE);
            }
        }
        currentPage = currentPage->next;
    }
}
//LRU最近最久未使用
void LRU(BlockQueue *blockQueue, process *proc) {
    PageQueue *currentPage = proc->pages.next;
    while (currentPage != NULL) {
        BlockQueue *searchedBlock = SearchPage(blockQueue, currentPage->page);//查找当前页面是否存在 
        if (searchedBlock != NULL) {
            searchedBlock->block.time = t++;//未发生替换，但使用该页面，最后使用时间++（与FIFO区别之处) 
            PrintBlockList(blockQueue, currentPage->page.pageID, COLOR_Exist);
        } else {
            BlockQueue *idleBlock = SearchIdleBlock(blockQueue);
            if (idleBlock != NULL) {
                idleBlock->block.state = BUSY;
                idleBlock->block.time = t++;
                idleBlock->block.page = (Page *) malloc(sizeof(Page));
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_IDLE);
            } else {
                idleBlock = GetOldestBlock(blockQueue);
                idleBlock->block.time = t++;
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue,currentPage->page.pageID, COLOR_NotExist_NoIDLE);
            }
        }
        currentPage = currentPage->next;
    }
}
//OPT最佳页面替换算法
void OPT(BlockQueue *blockQueue, process *proc){
    PageQueue *currentPage = proc->pages.next;
    while (currentPage != NULL) {
        if (SearchPage(blockQueue, currentPage->page) != NULL) {//命中 
            PrintBlockList(blockQueue, currentPage->page.pageID, COLOR_Exist);
        } else {
            BlockQueue *idleBlock = SearchIdleBlock(blockQueue);
            if (idleBlock != NULL) {//未命中，但有空闲块 
                idleBlock->block.state = BUSY;
                idleBlock->block.time = t++;
                idleBlock->block.page = (Page *) malloc(sizeof(Page));
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_IDLE);
            } else {//需要使用替换算法 
                idleBlock = GetLongestWithoutAccess(blockQueue,currentPage);//找到未来最长时间内不再被访问的页面进行替换 
                idleBlock->block.time = t++;
                idleBlock->block.page->pageID = currentPage->page.pageID;
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_NoIDLE);
            }
        }
        currentPage = currentPage->next;
    }
}
//时钟置换算法
void Clock(BlockQueue *blockQueue, process *proc){
    PageQueue *currentPage = proc->pages.next;
    BlockQueue *currentBlock = blockQueue;
    while (currentPage != NULL) {
    	BlockQueue *temp = SearchPage(blockQueue, currentPage->page);
        if (temp != NULL) {//命中 
        	temp->block.A = 1;
        	if(temp->next == NULL) currentBlock = blockQueue;
            else currentBlock = temp->next;
            PrintBlockList(blockQueue, currentPage->page.pageID, COLOR_Exist);
        } else {
            BlockQueue *idleBlock = SearchIdleBlock(blockQueue);
            if (idleBlock != NULL) {//未命中，但有空闲块 
                idleBlock->block.state = BUSY;
                idleBlock->block.time = t++;
                idleBlock->block.page = (Page *) malloc(sizeof(Page));
                idleBlock->block.page->pageID = currentPage->page.pageID;
                idleBlock->block.A = 1;
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_IDLE);
            } else {//需要使用替换算法 
                idleBlock = GetClockBlock(currentBlock,currentPage);//找到最近未使用的页面进行替换 
                idleBlock->block.time = t++;
                idleBlock->block.page->pageID = currentPage->page.pageID;
                idleBlock->block.A = 1;
                if(idleBlock->next == NULL) currentBlock = blockQueue;
                else currentBlock = idleBlock->next;
                PrintBlockList(blockQueue, 
                               currentPage->page.pageID, COLOR_NotExist_NoIDLE);
            }
        }
        currentPage = currentPage->next;
    }
}
//基本展示菜单
void Menu(){
    printf("=================欢迎使用请求分页存储管理系统=================\n");  
    printf("==================组员：张杰宁 李坤璘 方凡  小组：15组========\n");  
    printf("====================请选择页面置换算法========================\n");
    
    printf("       \t1.OPT\t2.FIFO\t3.LRU\t4.SCR\t5.Clock\t0.exit\n");
    printf("==============================================================\n");
    printf("请输入选项\n");
}
