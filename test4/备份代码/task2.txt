#include <iostream>
#include<cstring>
#include<unistd.h>
#include<pthread.h>
using namespace std;

const int frame_size = 100;
//记录的页表项
struct Page
{
    //页表项
    int page_id = -1;  //逻辑页面号
    bool state = 0;    //状态位
    bool visit = 0;    //访问位
    bool change = 0;   //修改位
    // struct Page* next=NULL;
};
//存储的内存帧
struct Page_frame
{
    int tid = -1; //进程号
    int frame_id; //占用进程的页号
    Page* ppage = NULL;
    struct Page_frame* map = NULL;
    struct Page_frame* next = NULL;
};
//线程（进程）的载体Choose_occ
struct PCB 
{
    int tid = -1;
    int page_num;
    int bmap_num;
    struct Page_frame* next = NULL;
    int index = 0; //记录当前访问到的位置
    int process[frame_size];
};
Page_frame* frame_list = new Page_frame; //内存的物理帧位图
Page *page; //内存的位示图
int number[10];//待访问页面号序列
int nc_size = 0;//内存大小
int page_size = 0;//页面大小
int number_size = 0; //线程/进程大小
int buffer_size;//定义的缓冲区的大小
pthread_mutex_t list_mutex;
pthread_mutex_t page_mutex;

void Choose_occ(PCB* pcb,int code);//选择置换算法对PCB进行操作
void fifo(PCB* pcb, Page_frame* list);//FIFO算法的实现
void* CreatPCB(void* ary); //多线程调用创建新PCB
void InsertList(bool Ishead, Page_frame* frame, Page_frame* list); //帧页的list插入
void MoToHead(Page_frame* frame, Page_frame* list); //链表元素的前调
int GetBmapNum(int size); //根据进程大小分配占用多少内存的函数
int GetCurrentNum(PCB* pcb); //获取当前进程的内存大小
bool IsinList(PCB* pcb, int frame_id); //判断某一页是否在内存里

int main()
{
    /*阶段一：初始化内存大小、页面大小，计算内存帧大小*/
    cout<<"请输入内存大小：";
    cin>>nc_size;//40
    cout<<"请输入页面大小：";
    cin>>page_size;//8
    /*计算内存帧的大小*/
    nc_size /= page_size;
    buffer_size = nc_size;
    pthread_t pid[3];
    page = new Page[buffer_size]; //初始化大小
    cout << "内存帧数量：" << buffer_size << endl;
    cout << "初始化成功" << endl;

    /*阶段二：创建线程，这里选择创建三个线程，设置对应的id、访问元素个数、具体顺序*/
    pthread_mutex_init(&list_mutex,NULL);
    pthread_mutex_init(&page_mutex,NULL);
    int pc1[] = { 1,3,4,0,1,2,1 };
    int pc2[] = { 2,3,4,0,1,2,2 };
    // int pc3[] = { 3,5,5,0,1,2,3,3 };
    int pc3[] = { 3,4,5,0,3,1,4,0 };
    pthread_create(&pid[0], NULL, CreatPCB, pc1);
    pthread_create(&pid[1], NULL, CreatPCB, pc2);
    pthread_create(&pid[2], NULL, CreatPCB, pc3);
    /*阶段三：所有线程执行完毕后释放*/
    for(int i = 0; i <3;i++)
    {
        pthread_join(pid[i], NULL);
    }
    return 0;
}

//选择置换算法对PCB进行操作
void Choose_occ(PCB* pcb, int code)
{
    if(IsinList(pcb, pcb->process[pcb->index]))
    {
        printf("进程：%d，其%d页已在内存\n", pcb->tid, pcb->process[pcb->index]);
        pcb->index++;
        return;
    }
    //判断自己占用的内存块是否已经大于分配的bmap
    int cnum = GetCurrentNum(pcb);
    if (cnum >= pcb->bmap_num || cnum >= buffer_size) //和自己置换
    {
        Page_frame* pre_pcb = NULL;
        Page_frame* tmp_pcb = pcb->next;
        while (tmp_pcb->next != NULL)
        {
            pre_pcb = tmp_pcb;
            tmp_pcb = tmp_pcb->next;
        }
        /*把pcb表里的新访问项添加到表头，移到头部保证兼容*/
        MoToHead(tmp_pcb, pcb->next); 
        pthread_mutex_lock(&list_mutex);
        /*list表中的该项记录添加到表头，此时该项记录也需要移动到表头*/
        MoToHead(tmp_pcb->map, frame_list); 
        pthread_mutex_unlock(&list_mutex);
        printf("进程：%d，页号：%d号，所占用的物理帧号:%d,替换的页号：%d\n", pcb->tid, pcb->process[pcb->index], (tmp_pcb->ppage - page), tmp_pcb->frame_id);
        tmp_pcb->frame_id = pcb->process[pcb->index];
        tmp_pcb->map->frame_id = pcb->process[pcb->index];

    }
    else //从内存中
    {
        //pthread_mutex_lock(&page_mutex);
        //找到第一个空闲的帧
        int flag = 0; //用作是否找到空闲帧的标记
        for(int i=0;i<buffer_size;i++)
        {
            if(page[i].state == 0)
            {
                page[i].page_id = pcb->tid; //该帧被pcb占用
                page[i].state = 1;
                Page_frame*nf_pcb = new Page_frame;
                Page_frame* nf_main = new Page_frame;
               nf_pcb->tid = pcb->tid;
                nf_main->tid = pcb->tid; //设置进程id
               nf_pcb->ppage = &page[i];
                nf_main->ppage = &page[i]; //设置对应物理帧
               nf_pcb->frame_id = pcb->process[pcb->index];
                nf_main->frame_id = pcb->process[pcb->index]; //设置对应进程的页号
               nf_pcb->map = nf_main;
                nf_main->map =nf_pcb; //相互映射
                printf("%d号进程的%d号页占用了物理帧:%d\n", pcb->tid, pcb->process[pcb->index], i);
                InsertList(true,nf_pcb, pcb->next); //对pcb的访问表修改
                pthread_mutex_lock(&list_mutex);
                InsertList(true, nf_main, frame_list); //对总表进行修改
                pthread_mutex_unlock(&list_mutex);
                flag = 1;
                break;
            }
        }
        //pthread_mutex_unlock(&page_mutex);
        if (flag == 0) //内存满了,进行置换,添加顺序是头插法
        {
            if (code == 0) //选择fifo
            {
                fifo(pcb, frame_list);
            } 
        }
    }
    pcb->index++;
}
//判断某一页是否在内存里
bool IsinList(PCB* pcb, int frame_id)
{
    pthread_mutex_lock(&list_mutex);
    Page_frame* tmp = pcb->next;
    while (tmp->next != NULL)
    {
        if (tmp->frame_id == frame_id && tmp->tid == pcb->tid)
        {
            pthread_mutex_unlock(&list_mutex);
            return true;
        }
        tmp = tmp->next;
    }
    pthread_mutex_unlock(&list_mutex);
    return false;
}
//多线程调用创建新PCB
void* CreatPCB(void* ary)
{
    /*
        ary[0]为线程的id
        ary[1]为访问顺序元素的个数
        ary[2]及其以后为访问的具体顺序
    */
    PCB* p = new PCB;
    int* array = (int*)ary;
    p->tid = array[0];
    p->page_num = array[1];
    p->bmap_num = GetBmapNum(p->page_num);
    p->next = new Page_frame;
    int count = array[2];
    for (int i = 0; i < count; i++)
    {
        p->process[i] = array[3 + i]; 
    }
    while(p->index!=count)
    {
        sleep(3);
        Choose_occ(p, 0);
    }
    return NULL;
}
//根据进程大小分配占用多少内存的函数
int GetBmapNum(int size)
{
    if(size==1)
         return 1;
    else if(size>=2&&size<=6) return 2;
    else
    {
        return size/3;
    }
}
//获取当前进程的内存大小
int GetCurrentNum(PCB* pcb)
{
    int count = 0;
    pthread_mutex_lock(&list_mutex);
    Page_frame* tmp = pcb->next->next;
    Page_frame* pre = pcb->next;
    while (tmp != NULL) 
    {
        /*当进程占用模块时，将不会被舍弃*/
        if(tmp->tid==pcb->tid) //进程占用
        {
            pre = pre->next;
            tmp = tmp->next;
            count++;
        }
        /*如果遇到了争夺的行为，此时PCB不是自己的，需要舍弃*/
        else //被争夺了
        {
            pre->next = tmp->next;
            Page_frame* dtmp = tmp; //获取错乱记录表项
            tmp = tmp->next;
            delete dtmp;
        }
    }
    pthread_mutex_unlock(&list_mutex);
    return count;
}
//帧页的list插入
void InsertList(bool Ishead, Page_frame* frame, Page_frame* list)
{
    if(Ishead)
    {
        frame->next = list->next;
        list->next = frame;
    }
    else
    {
        Page_frame* tmp = list;
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }
        tmp->next = frame;
    }
}
 //链表元素的前调
void MoToHead(Page_frame* frame, Page_frame* list)
{
    Page_frame *pre = NULL;
    Page_frame *tmp = list->next;
    while(tmp->next!=NULL)
    {
        pre = tmp;
        tmp = tmp->next;
        if(tmp==frame)
        {
            pre->next = NULL;
            InsertList(true, tmp, list);
            break;
        }
    }
}
//FIFO算法的实现
void fifo(PCB* pcb, Page_frame* list)
{
    pthread_mutex_lock(&list_mutex);
    pthread_mutex_lock(&page_mutex);
    Page_frame* tmp = frame_list->next; //在总表上找最后一个表项
    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }
    tmp->frame_id = pcb->process[pcb->index]; //修改该list表项对应的进程页号
    Page* opage = tmp->ppage; //获取其占用的物理帧
    printf("进程：%d  抢夺：%d号进程  ，帧号：%d ， 放入页号：%d\n", pcb->tid, opage->page_id, (opage - page), pcb->process[pcb->index]); //第几号物理帧是计算的偏移
    opage->page_id = pcb->tid; //修改物理帧的被占用属性     
    Page_frame* newpcb_frame = new Page_frame;
    newpcb_frame->frame_id = pcb->process[pcb->index];
    newpcb_frame->map = tmp;
    newpcb_frame->ppage = opage;
    newpcb_frame->tid = pcb->tid;
    MoToHead(tmp, frame_list);
    pthread_mutex_unlock(&list_mutex);
    pthread_mutex_unlock(&page_mutex);
    InsertList(true, newpcb_frame, pcb->next); //把抢夺到的帧添加到自己的链表里  
}
