#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
  
#define Max 1048576  
typedef struct freeList Flist;  
typedef struct node {  
  int id;        //装入此内存块进程的id；  
  int startAdd;  //内存块的首地址  
  int length;    //内存块的长度  
  unsigned char status;  //状态位，记录该分区的状态，默认为0，表示未分配，不为0表示已经分配了  
} Node;  
//记录内存的链表  
typedef struct list {  
  struct list *last;  //链表记录的上一个节点  
  struct list *next;  //链表记录的下一个节点  
  Flist *flist;  
  Node *cur;  //链表记录的当前节点  
} List;  
struct freeList {  
  struct freeList *last;  
  struct freeList *next;  
  List *list;  
};  
  
void Menu();      //界面函数  
List *InitMat();  //初始化mat表的函数，必须在空闲分区表之前  
Flist *InitFreeTable(List *Mat);  //初始化空闲分区表  
int Allocate(Flist *fl, int id, int psize,Flist **fhead);  //分配方法，具体参数定义见函数体 ,返回值未定义，可自定义  
void Momory_recycle(List **mat, Flist **Free); //回收方法   
Flist* First_fit(Node* tempnode,Flist *fl); //首次适应算法
Flist* Best_fit(Node* tempnode,Flist*fl);  //最佳适应算法
Flist* Worst_fit(Node* tempnode,Flist *fl);//最坏适应算法  
Flist* Circl_fit(Node* tempnode,Flist *fl,Flist *Last_fl); //循环适应算法
void ShowMomory(List *head);//输出MAT表所有内存占有情况
void AllChoice(Flist *fhead);//提供内存分配的选择


int main() {  
  List *Lhead = InitMat();  
  Flist *Fhead = InitFreeTable(Lhead);  
  int cid = 0;  
  while(1)  
  {  
    Menu();  
    printf("\t请输入操作指令: ");  
    scanf("%d",&cid);  
    getchar();  
    switch(cid)  
      {  
        case 1:  
            AllChoice(Fhead);  
            break;  
        case 2:  
            Momory_recycle(&Lhead,&Fhead);  
            break;  
        case 3:  
            ShowMomory(Lhead);  
            break;  
        case 4:  
            printf("\t感谢使用！\n");
            return 0;
            break;  
        default:  
            printf("\t输入指令无效");  
            break;  
      }  
      getchar();  
      // system("cls");  
  }  
//  Allocate(Fhead,100,2000,&Fhead);  
//  Allocate(Fhead,80,2000,&Fhead);  
//  //Allocate(fl,n.id,n.length,&fhead);  
//  ShowMomory(Lhead);  
  return 0;  
}  
//菜单界面
void Menu() {  
  printf("\n\n");  
  printf("\t┏--------------------------------------------------┓\n");  
  printf("\t┃           欢迎使用动态多分区模拟程序             ┃\n");  
  printf("\t┃  组员：张杰宁 李坤璘 方凡       小组：15组       ┃\n");  
  printf("\t┃  1、进程分配                                     ┃\n");  
  printf("\t┃  2、内存回收                                     ┃\n");  
  printf("\t┃  3、显示内存                                     ┃\n");  
  printf("\t┃  4、退出                                         ┃\n");  
  printf("\t┃                                                  ┃\n");  
  printf("\t┗--------------------------------------------------┛\n\n");  
} 
//MAT表初始化
List *InitMat() {  
  List *head;  
  head = (List *)malloc(sizeof(List) * 1);  
  head->last = NULL;  
  head->next = NULL;  
  Node *n = (Node *)malloc(sizeof(Node) * 1);  
  n->id = -1;  
  n->startAdd = 0;  
  n->length = Max;  
  n->status = 0;  
  head->cur = n;  
  return head;  
}  
//空闲分区表初始化
Flist *InitFreeTable(List *Mat) {  
  Flist *fhead;  
  fhead = (Flist *)malloc(sizeof(Flist) * 1);  
  fhead->last = NULL;  
  fhead->next = NULL;  
  fhead->list = Mat;  //指向首元素节点  
  (fhead->list)->flist = fhead;  
  return fhead;  
}  
//分配内存，具体参数定义见函数体
int Allocate(Flist *fl, int id, int psize, Flist **fhead) {  
  Node *ntemp = (fl->list)->cur;  
  ntemp->id = id;  
  ntemp->status = 1;  // 1表示被占用了  
  //进程大小正好与当前分区相同,不用分区,但要移出空闲分区表  
  if (ntemp->length == psize) {  
    if (*fhead == fl)  //说明被分配走的是表头  
    {  
      *fhead = (*fhead)->next;  
      (fl->list)->flist = NULL;  
      free(fl);  
    } else  //不是表头  
    {  
      Flist *ftemp = fl->last;  
      ftemp->next = fl->next;  
      if ((fl->next) != NULL) (fl->next)->last = ftemp;  
      (fl->list)->flist = NULL;  
      free(fl);  
    }  
    return 0;  
  } else  //此处肯定是小于的情况
  {  
    //小于情况下，需要新建Mat，修改空闲分区  
    Node *newNode = (Node *)malloc(sizeof(Node) * 1);  
    List *newList = (List *)malloc(sizeof(List) * 1);  
    newNode->length = ntemp->length - psize;  
    ntemp->length = psize;  //修改老节点和新节点的长度  
    newNode->id = -1;       //初始化id  
    //则其起始地址为 老节点起始地址加进程大小  
    newNode->startAdd = ntemp->startAdd + psize;  
    newNode->status = 0;  
    //节点初始化完毕，开始初始化链表  
    newList->cur = newNode;  //设置新链表节点的内存节点  
    newList->last = fl->list;  //设置新链表节点的上一个节点为  被分割节点  
    newList->next = (fl->list)->next;  
    (fl->list)->next = newList;  //设置先后关系  
    (fl->list)->flist = NULL;  
    fl->list = newList;  
    newList->flist = fl;  
    return 1;  
  }  
  return 2;  
}  
//内存的回收
void Momory_recycle(List **mat, Flist **Free)
{  
  List* now = *mat;  
  /*输入要撤销的进程id，遍历mat表去查询对应的id号*/  
  while (1) {  
    /*输入要撤销的进程id*/  
    int id;  
    printf("\t请输入要撤销的进程号：");  
    scanf("%d", &id);  
    /*遍历mat表*/  
    if(id<0){  
        printf("\t非法的id\n");  
        getchar();  
        return ;  
    }  
    now = *mat;  
    while (now!=NULL&&now->cur->id!=id) {  
//      printf("aa\n");  
      now = now->next;  
    }  
    if (now == NULL) {  
      printf("\t未找到对应进程\n");  
      break;  
    } else {  
      break;  
    }  
  }  
  /*检查该结点now前一地址块和后一地址块是否是空闲的*/  
  int flag = 0;  //前后有需要合并的就为0，不需要合并则为1  
  if (now->last == NULL)  //链表首部  
  {  
    printf("a\n");  
    List *Next = now->next;   
    if (Next->cur->status == 0)  //后一块地址空闲  
    {  
      //更改空闲分区链里指向的结点，以及让now指向原Next指向的分区链结点  
      Next->flist->list = now;  
      now->flist = Next->flist;  
      //在mat表里删除后面的结点，将后者长度归并到前者里  
      now->cur->length += Next->cur->length;  
      now->cur->status = 0;  
      now->next = Next->next;  //后指针连起来  
      Next->next->last = now;  //前指针连起来  
      free(Next);  
    } else  //后一块地址不空闲  
    {  
      flag=1;  
    }  
  }   
  else if (now->next == NULL)  //链表尾部  
  {  
    printf("b\n");  
    List *Last = now->last;  
    if (Last->cur->status == 0)  //前一块地址空闲  
    {  
      //更改空闲分区链里指向的结点，以及让now指向原Last指向的分区链结点  
      Last->flist->list = now;  
      now->flist = Last->flist;  
      //删除尾结点，更新尾部的结点内存地址长度  
      now->cur->length+=Last->cur->length;  
      now->cur->status=0;  
      now->cur->startAdd=Last->cur->startAdd;  
      Last->last->next=now;  
      now->last=Last->last;  
      free(Last);  
    }   
    else  //前一块地址不空闲不需要合并  
    {  
      flag = 1;  
    }  
  }   
  else  //链表中间的常规结点  
  {  
    printf("c\n");  
    List *Next = now->next;  
    List *Last = now->last;  
    if (Last->cur->status == 0)  //前一块地址空闲  
    {  
      //更改空闲分区链里指向的结点，以及让now指向原Last指向的分区链结点  
      Last->flist->list = now;  
      now->flist = Last->flist;  
      //更新尾部的结点内存首地址  
      now->cur->status=0;  
      now->cur->startAdd=Last->cur->startAdd;  
      Last->last->next=now;  
      now->last=Last->last;  
      if(Next->cur->status==0)//后一块地址也空闲  
      {  
        //内存长度为三块内存的和  
        now->cur->length+=(Last->cur->length+Next->cur->length);  
        //还要额外删除后结点  
        Next->next->last=now;  
        now->next=Next->next;  
        //在空闲分区链里，Next对应的结点也要删除  
        Flist *temp=Next->flist;//Next指向的空闲分区链结点  
        temp->last->next=temp->next;  
        temp->next->last=temp->last;  
        free(temp);  
      }  
      else//后一块内存不空闲  
      {  
        //内存长度为两块内存的和  
        now->cur->length+=Last->cur->length;  
      }  
      free(Last);  
      free(Next);  
    }  
    else if(Next->cur->status==0) //只有后一块地址空闲  
    {  
      //更改空闲分区链里指向的结点，以及让now指向原Next指向的分区链结点  
      Next->flist->list = now;  
      now->flist = Next->flist;  
      //在mat表里删除后面的结点，将后者长度归并到前者里  
      now->cur->length += Next->cur->length;  
      now->cur->status = 0;  
      now->next = Next->next;  //后指针连起来  
      Next->next->last = now;  //前指针连起来  
      free(Next);  
    }  
    else//这时候一定是前后内存都不空闲，也就是说不需要合并  
    {  
      flag=1;  
    }  
  }  
  if(flag)//内存不需要合并，所以需要在空闲分区链里插入一个新结点  
  {  
      //在空闲分区链和mat表里结点联系  
      Flist *tem = (Flist *)malloc(sizeof(struct freeList));  
      tem->list = now;  
      now->flist = tem;  
      //头插  
      tem->next = *Free;  
      (*Free)->last = tem;  
      (*Free)=tem;  
  }  
}  
//首次适应算法
Flist *First_fit(Node* tempnode,Flist *fl){  
    Flist *temp_fl= fl;  
    Flist *first_fl = NULL;  
    if(temp_fl==NULL){  
        printf("NULL");  
    }  
    int i=1;  
    while(temp_fl!=NULL){//遍历空白链表，直到NULL  
        int temp_fl_length =temp_fl->list->cur->length;  
        if(temp_fl_length>tempnode->length){  
            first_fl = temp_fl;  
            break;  
        }  
        else{  
            temp_fl=temp_fl->next;  
        }  
        i++;  
    }  
    return first_fl;  
}  
//最佳适应算法
Flist *Best_fit(Node* tempnode,Flist*fl){  
    int max = 0;  
    Flist *temp_fl = fl;  
    Flist *max_fl =NULL;  
    //链表寻找最大节点  
    while(temp_fl){  
        int temp_fl_length =temp_fl->list->cur->length;  
        if(temp_fl_length>max){  
            max = temp_fl_length;  
            max_fl = temp_fl;  
        }  
        else{  
            temp_fl = temp_fl->next;  
        }  
    }  
    if(max_fl->list->cur->length<tempnode->length){  
        printf("最大空闲块<进程\n");  
        return NULL;//最大空闲块仍无法放入   
    }  
    else{  
        return max_fl;  
    }  
}  
//最坏适应算法  
Flist *Worst_fit(Node* tempnode,Flist *fl){  
    int min=0 ;  
    Flist *temp_fl = fl;  
    Flist *min_fl =NULL;  
    //链表寻找最大节点  
    while(temp_fl){  
        int temp_fl_length =temp_fl->list->cur->length;  
        int tempnode_length = tempnode->length;  
        if(temp_fl_length<min&&temp_fl_length>=tempnode_length){//双重限制可装入进程前提下的最小空白块  
            min = temp_fl_length;  
            min_fl = temp_fl;  
            break;  
        }  
        else{  
            temp_fl = temp_fl->next;  
        }  
    }  
    return min_fl;  
}  
 //循环适应算法
Flist* Circl_fit(Node* tempnode,Flist *fl,Flist *Last_fl){//Last_fl:上次分配出去的空闲节点  
    Flist *temp_fl = Last_fl;  
    Flist *fit_fl = NULL;  
    while(temp_fl->next!=Last_fl){//循环至初始位置结束  
        int temp_fl_length =temp_fl->list->cur->length;//当前遍历到的空闲块大小  
        int tempnode_length = tempnode->length;//当前进程大小  
        if(temp_fl_length>=tempnode_length){//可装入当前进程  
            fit_fl = temp_fl;  
            break;  
        }  
        else{  
            if(temp_fl->next==NULL){  
                temp_fl = fl;  
            }  
            else{  
                    temp_fl = temp_fl->next;  
            }  
        }  
    }  
    return fit_fl;  
}  
//输出MAT表所有内存占有情况
void ShowMomory(List *head)  
{  
    Node *nt;  
    List *temp = head;  
    while(temp!=NULL)  
    {  
        nt = temp->cur;  
        printf("\t内存地址范围：%d - %d，占用状态：%d 进程id: %d\n ",nt->startAdd,nt->startAdd+nt->length,nt->status,nt->id);  
        temp = temp->next;  
    }  
}   
//提供内存分配的选择
void AllChoice(Flist *fhead)  
{  
    int id;  
    printf("\t请选择分配方式:1.FirstFit 2.BestFit 3.WorstFit 4.CircleFit\n\t");  
    scanf("%d",&id);  
    Node n;  
    printf("\t输入进程id:");  
    scanf("%d",&n.id);  
    getchar();  
    printf("\t输入进程大小:");  
    scanf("%d",&n.length);  
    getchar();  
    Flist *fl = NULL;  
    switch(id)  
    {  
        case 1:  
            fl = First_fit(&n,fhead);  
            break;  
        case 2:  
            fl = Best_fit(&n,fhead);  
            break;  
        case 3:  
            fl = Worst_fit(&n,fhead);  
            break;  
        default:  
            printf("\t输入指令无效");  
            getchar();  
            break;  
    }  
    if(fl==NULL)  
    {  
        printf("\t没有可分配内存\n");  
    }  
    else  
    {  
        Allocate(fl,n.id,n.length,&fhead);  
        printf("\t分配内存成功\n");  
    }  
}  
  