/*
	任务一：利用信号量模拟“多生产者-多消费者”对同一缓冲区的访问
	使用到自己改编重写的P/V操作
*/
#include<iostream>
#include<cstdio>//基本格式的输入输出
#include<cstdlib>//使用了malloc申请全局内存，以及调用了信号量互斥访问的函数
#include<pthread.h>//线程库函数
#include<unistd.h>//系统调用API接口用于获取当前目录
#include<ctime>//统计时间以及生成种子以便随机生成生产者待生产的字符
#include<exception>//便于C++的try-catch-throw的异常
using namespace std;


//宏定义一些变量和信号量，方便进行调试
#define N 10			 //缓冲区的大小
#define ProNum 3		 //生产者的数量
#define ConNum 3		 //消费者的数量
#define SleepTime 3		 //生产者消费者操作完后的暂停时间
typedef int semaphore;	 //定义信号量的类型均为整数
typedef char element;	 //定义生产者生产的元素都是字符
element buffer[N] = {0}; //定义长度为 N 的缓冲区，内容均是字符型
int in = 0;				 //缓冲区下一个存放产品的索引
int out = 0;			 //缓冲区下一个可以使用的产品索引
int ProCount = 0;		 //生产者序号统计
int ConCount = 0;		 //消费者序号统计
double StartTime;		 //程序开始时间记录，方便输出执行语句的时间
/*
	定义并初始化信号量
	mutex 用来控制生产和消费每个时刻只有一个在进行，初始化为 1，用来实现互斥
	empty 表示剩余的可用来存放产品的缓冲区大小，用来实现同步，刚开始有 N 个空位置
	full 表示当前缓冲区中可用的产品数量，用来实现同步，刚开始没有产品可以使用
*/
semaphore mutex = 1, empty = N, full = 0;

//生产者线程
void *Producer(void *args)
{
	int *x = (int *)args;
	element temp;//目前待生产的字符
	int sum = 0;
	while (1)
	{
		/*同步操作：P(empty),判断是否有空位置供存放生产的产品，没有空位则等待并输出提示语句*/
		while (empty <= 0) // 
			printf("%.4lfs| 缓 冲 区 已 满 ! 生 产 者 :%d 等待中......\n", (clock() - StartTime) / CLOCKS_PER_SEC, *x);
		empty--; //等到一个空位置，我们先把他占用

		/*互斥操作： // P(mutex),判断当前临界区中是否有进程正在生产或者消费,如果有则等待并输出提示语句*/
		while (mutex <= 0)
			printf("%.4lfs|	缓冲区有进程正在操作 !	生产者 :%d 等待中......\n", (clock() - StartTime) / CLOCKS_PER_SEC, *x);
		mutex--; //等到占用进程出临界区，进入临界区并占用

		/*
			生产者的生产过程：
			生产了一个产品后，输出生产成功的字样;
			然后将其放入buffer缓冲区中，in代表循环队列队尾的指针移动，以便下一个产品的生产
		*/
		temp = (rand() % 26) + 65; //生产一个产品，即任意产生一个 A～Z 的字母
		printf("%.4lfs| 生产者: %d 生产一个产品: %c ,并将其放入缓冲区: %d 位置\n", (clock() - StartTime) / CLOCKS_PER_SEC, *x, temp, in);//输出生产成功的字样
		buffer[in] = temp; //将生产的产品存入缓冲区
		sum++;
		in = (in + 1) % N; //缓冲区索引更新

		/*互斥操作：V(mutex),临界区使用完成释放信号*/
		mutex++;

		/*同步操作：V(full),实现同步，释放 full*/
		full++;

		/*生产者生产之后让它休息一下，采用sleep实现*/
		sleep(SleepTime); //休眠时间设为3个clock
	}
}

//消费者线程
void *Consumer(void *args)
{
	int *x = (int *)args;
	int sum = 0;
	while (1)
	{
		/*同步操作：P(full),判断缓冲区当中是否有产品可以消费，同步如果没有产品则等待同时输出提示语句*/
		while (full <= 0) 
			printf("%.4lfs| \t\t\t\t\t\t\t 缓冲区为空！消费者: %d 等待中......\n", (clock() - StartTime) / CLOCKS_PER_SEC, *x);
		full--; //等到有一个产品，消费这个产品

		/*互斥操作：P(mutex),判断临界区是否有进程在处理,如果有则等待并输出提示语句*/
		while (mutex <= 0) 
			printf("%.4lfs| \t\t\t\t\t\t\t 缓冲区有进程正在操作! 消费者 : %d 等待中......\n", (clock() - StartTime) / CLOCKS_PER_SEC, *x);
		mutex--; //等到临界区为空，则进入缓冲区消费

		/*
			消费者的消费过程：
			发现有能够消费的产品，我们就输出消费成功的字样;
			然后将其从buffer缓冲区中取出，out代表循环队列队首的指针移动，以便下一个产品的消费
		*/
		printf("%.4lfs| \t\t\t\t\t\t\t 消费者: %d 消费一个产品: %c , 将其从缓冲区取出 : %d 位置 \n", (clock() - StartTime) / CLOCKS_PER_SEC, *x, buffer[out], out);//输出消费成功的语句
		buffer[out] = 0; //更新缓冲区，把消费掉的产品清空
		sum++;
		out = (out + 1) % N; //更新缓冲区索引

		/*互斥操作：V(mutex)，消费完成退出缓冲区，释放资源*/
		mutex++;

		/*同步操作：V(full)，消费完成产生一个空位，进行同步*/
		empty++;

		/*消费者消费之后让它休息一下，采用sleep实现*/
		sleep(SleepTime); //休眠时间设为3个clock
	}
}

int main()
{
	/*
		阶段一：一系列的初始化，包括开始时钟的记录，随机种子预备以及输出界面初始化
	*/
	StartTime = clock(); //记录程序开始的时间，方便记录消费者和生产者活动的时间
	srand((int)time(NULL));//产生随机种子，生产的时候随机生产一个字符
	printf(" 计时");
	printf("    \t\t 生产者动态显示");
	printf("          \t\t\t\t 消费者动态显示\n");
	printf("======================================================================================================================\n");
	/*
		阶段二：进行线程的创建，在本题中有3个生产者3个消费者，因此需要大小为6的线程数组
		分别创建三个生产者线程以及三个消费者线程
	*/
	pthread_t threadPool[ProNum + ConNum];//定义一个线程数组，存储所有的消费者和生产者线程,
	//创建生产者
	int t1=1,t2=2,t3=3;
	pthread_create(&threadPool[0], NULL, Producer, (void *)&t1);
	pthread_create(&threadPool[1], NULL, Producer, (void *)&t2);
	pthread_create(&threadPool[2], NULL, Producer, (void *)&t3);
	//创建消费者
	pthread_create(&threadPool[3], NULL, Consumer, (void *)&t1);
	pthread_create(&threadPool[4], NULL, Consumer, (void *)&t2);
	pthread_create(&threadPool[5], NULL, Consumer, (void *)&t3);

	/*
		阶段三：线程结束资源的回收
		在这里因为涉及到线程回收因此要添加异常检测
	*/
	void *result;
	for (int i = 0; i < ProNum + ConNum; i++) //等待线程结束并回收资源，线程之间同步
	{
		int Return=pthread_join(threadPool[i], &result);
		try{
			if (Return== -1) throw Return;
		}
		catch(int){
			cerr<<"检测到线程回收失败"<<endl;//回收失败的错误提示
			exit(1);//结束线程
		}
	}
}
