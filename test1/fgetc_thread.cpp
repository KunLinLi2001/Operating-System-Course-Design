//fgetc+多线程
/*需要用到的C/C++头文件*/
#include<iostream>//用于cout和异常输出的cerr
#include<cstdio>
#include<fstream>//文件流读取
#include<map>//STL标准map库模拟平衡二叉树
#include<vector>//向量存储.txt文件的名称
#include<cstring>//使用了string流和标准字符串
#include<ctime>//统计时间
#include<exception>//便于C++的try-catch-throw的异常检测
#include<algorithm>//最后使map按照value排序
#include<set>//集合用于存储意义不大的介词、冠词等单词
using namespace std;

/*用到的linux系统内核/系统调用函数头文件*/
#include<cstdlib>//使用了malloc申请全局内存，以及调用了信号量互斥访问的函数
#include<unistd.h>//系统调用API接口用于获取当前目录
#include<pthread.h>//线程库函数
#include<dirent.h>//用于遍历linux系统中当前路径文件
#include<sys/stat.h>//定义linux文件状态

/*一些全局变量的初始化*/
const int MAX=1000;//定义最大路径名称长度和单词的最大长度
char dir[MAX];//存放.txt文件的文件夹名
int thread_num=0;//线程的个数，也就是文件夹里.txt文件的数目
vector<string> txtfiles;//保存文件的路径
map<string,int>mp;//创建结果的map

set<string> st;//存储意义不大的介词、冠词等单词
pthread_mutex_t mutex_txt;//对访问txt文件互斥的信号量
pthread_mutex_t mute_ans;//对计算写入结果互斥的信号量
const int print_num = 5;//定义每行打印多少单词
int *Count;//统计单篇文章的单词数目
int sum_num=0;//单词结果的总数
/*一些需要用到的函数声明*/
bool cmp(const pair<string,int> &p1,const pair<string,int> &p2);//排序所用的比较函数
int scandir(char *);//扫描文件夹搜索出所有的txt文件
bool Istxt(char *);//判断是否是txt文件
bool Isword(char ch);//判断是否是能组成单词的字符
void *find(void *id);//线程运行的函数
void Print();//打印结果
void InitWord();//词库初始化
bool Ismeaningful(string ch);//检查该单词是否是有意义的

int main(int argc,char* argv[]){
	/*----------阶段一：try-catch-throw进行一系列异常检测----------*/
	/*检测是否正确传入参数：程序+文件夹*/
	try{
		if(argc!=2) throw argc;
		strcpy(dir,argv[1]);
	}
	catch(int){
		cerr<<"检测到输入参数有异常"<<endl
			<<"正确使用：./xx程序 ./xx文件夹"<<endl
			<<"程序正在退出..."<<endl;
		exit(1);
	}
	/*检验当前文件夹是否能成功读入*/
	DIR *tem;
	try{
		if((tem=opendir(dir))==NULL) throw 1;
		scandir(dir);//扫描文件夹读取.txt文件进入vector
	}
	catch(int){
		cerr<<"文件读取失败或文件夹不存在"<<endl
			<<"请检查正确文件夹姓名"<<endl
			<<"程序正在退出..."<<endl;
		exit(1);
	}
	/*检验当前扫描的文件夹里是否有可读入的.txt文件*/
	try{
		if(txtfiles.size()==0) throw txtfiles.size();
	}
	catch(int){
		cerr<<"没有可读入的.txt文件"<<endl
			<<"请检查文件夹中的文件内容是否准确"<<endl
			<<"程序正在退出..."<<endl;
		exit(1);
	}

	/*----------阶段二：多线程进行计数----------*/
	/*信号量初始化+时间初始化+无意义词库初始化*/
	while(pthread_mutex_init(&mutex_txt,NULL)!=0);//信号量初始化
	while(pthread_mutex_init(&mute_ans,NULL)!=0);//信号量初始化
	InitWord();//词库初始化
	int start,end;//统计运行时间
	start = clock();//开始计时
	/*为线程开辟空间*/
	pthread_t *tid=(pthread_t*)malloc((thread_num+1)*sizeof(pthread_t));//为线程分配空间，thread_num个线程就分配thread_num块
	int* id=(int*)malloc((thread_num+1)*sizeof(int));//存储子进程的id
	Count=(int*)malloc((thread_num+1)*sizeof(int));//为统计单词数开辟空间
	for(int i=1;i<=thread_num;i++) Count[i]=0;
	/*遍历txt文件创建线程*/
	while(txtfiles.size()){
		/*创建线程*/
		for(int i=1;i<=thread_num;i++){
			id[i]=i;//记录线程id
			if(pthread_create(&tid[i],NULL,find,&id[i])==-1)
            	printf("线程创建失败\n");
		}
		/*等待所有线程均执行结束*/
		for(int i=1;i<=thread_num;i++)
			pthread_join(tid[i],NULL);
	}
	/*----------阶段三：摧毁信号量，释放内存----------*/
	pthread_mutex_destroy(&mutex_txt);
	pthread_mutex_destroy(&mute_ans);
	free(tid);	//释放内存
	free(id);	//释放内存
	/*----------阶段四：输出统计结果，记录所用时间----------*/
	Print();//输出计算结果
	end=clock();//结束计时
	printf("多线程运行时间为: %d 微秒\n", end-start);//输出运行时间
	return 0;
}
/*自定义cmp用于后期排序使用*/
bool cmp(const pair<string,int> &p1,const pair<string,int> &p2){
	if(p1.second==p2.second)
		return p1.first<p2.first;
	return p1.second>p2.second;
}
/*扫描文件夹*/
int scandir(char *dir){
	DIR *current_dir;
	struct dirent *catalogue;//当前所处的目录结点
	struct stat buffer_status;//暂存文件信息状态
	current_dir=opendir(dir);//打开文件夹
	int ignore_a=chdir(dir);//将当前的工作目录转换到dir存储的文件夹下

	/*不断读取目录中的信息直到没有文件/目录可以遍历（ls）*/
	while((catalogue=readdir(current_dir))!=NULL){
		lstat(catalogue->d_name,&buffer_status);//获取文件信息
		/*如果当前读到的是目录（文件夹）*/
		if(S_ISDIR(buffer_status.st_mode)){	
			/*如果目前访问目录是文件夹本身(".")或目录即将是上一级("..")*/
			if((!strcmp(".", catalogue->d_name) || (!strcmp("..",catalogue->d_name))))
				continue;//跳过这条目录
			/*目前扫描到目录是非自身的文件夹*/
			else 
				scandir(catalogue->d_name);//递归重新进行新文件夹的扫描
		}
		/*当前不是文件夹（是单个文件）*/
		else{
			/*如果是txt文件就读入*/
			if(Istxt(catalogue->d_name)){
				char current_path[MAX];//存储目前的路径
				/*写出将txt文件的完整路径*/
				char *ignore_c = getcwd(current_path,sizeof(current_path));//写入目前的绝对路径
                strcat(current_path,"/");
                strcat(current_path,catalogue->d_name);//将目录里的txt文件名插入到目前路径形成完整的txt绝对路径
				/*将路径写入vector*/
                string s(current_path);	//将current_path转换为字符串
                txtfiles.push_back(s);
                thread_num++;//是txt文件就+1
			}
		}
	}
	chdir("..");//返回上一级
	closedir(current_dir);//关闭文件夹
	return 0;
}
/*判断是否为txt文件*/
bool Istxt(char *ch){
	int len=strlen(ch);
	/*如果最后四个字符时.txt说明是txt文件*/
	if(strcmp(&ch[len-4],".txt")==0)
		return true;	//是txt文件
	/*不是txt文件的话默认为false*/
	return false;
}
/*判断是否是能组成单词的字符*/
bool Isword(char ch){
	if((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z'))//只有大小写字母才可以组成单词
		return true;
	return false;
}
void InitWord(){
	ifstream fp("words.txt");//打开文件
	char s[100];
	while(!fp.eof()){
		fp.getline(s,100);
		st.insert(s);
	}
}
bool Ismeaningful(string ch)
{
	/*如果单词不在集合里，说明是有意义的*/
	if(st.count(ch)==0) return true;
	return false;
}
/*核心函数：统计单词个数的find函数*/
void *find(void *id){
	/*容器没有元素了说明遍历完了*/
	if(!txtfiles.size()) 
		return NULL;
	int num=*(int*)id;
	printf("线程开始执行...,它的id为: %d\n",num);

	/*利用信号量进行加锁保证互斥*/
	while(pthread_mutex_lock(&mutex_txt)!=0);//P操作
	string s = txtfiles[txtfiles.size()-1];
	txtfiles.pop_back();//路径已经被选择，为了防止被重复取则移出容器
	pthread_mutex_unlock(&mutex_txt);//V操作

	/*打开相应的txt文件*/
	char current_path[MAX];
	strcpy(current_path, s.c_str());//转为字符串
	FILE *p=fopen(current_path,"r");//打开文件
	/*开始读入字符*/
	string word="";//记录单词
	char c;
	c=fgetc(p);//从文件里逐字读入字符
	while(c!=EOF){
		if(Isword(c))
			word+=c;
		else{
			/*如果当前单词串为空,直接读入字符但是不插入word（因为不是字母）*/
			if(word==""){
				c=fgetc(p);
				continue;
			}
			/*统计单词的时候需要信号量互斥操作*/
			while(pthread_mutex_lock(&mutex_txt)!=0);//P操作
			mp[word]++;
			sum_num++;
			word="";//重置单词
			pthread_mutex_unlock(&mutex_txt);//V操作
			Count[num]++;//单篇单词数+1
		}
		c=fgetc(p);
	}
	fclose(p);//读完这个文件了就关闭文件
	printf("id为 %d 的线程统计结束,单词总数为%d\n",num,Count[num]);
	return NULL;
}
/*输出统计结果*/
void Print(){
	printf("单词总数 : %d\n",sum_num);
	/*默认的map是按key排序的,输出之前换用value排序*/
	vector<pair<string,int>> arr;
   for(map<string,int>::iterator it=mp.begin();it!=mp.end();++it)
	arr.push_back(make_pair(it->first,it->second));
   sort(arr.begin(),arr.end(),cmp);//对vector排序
	/*遍历vector等效作为遍历map容器*/
	printf("现展示出现频率top20的单词如下:\n");
	FILE *fp;
	fp=fopen("res1.txt","w");
	int i,k=1;
	for(vector<pair<string,int>>::iterator it=arr.begin();it!=arr.end();++it){
		i++;
		if(k<=20){
			if(Ismeaningful(it->first)){
				printf("%04d : %s ", it->second, (it->first).c_str());
   				for(int j = 20 - (it->first).size();j > 0;j--) putchar(' ');
				if(k % print_num == 0) puts("");
				k++;
			}
		}
		fprintf(fp,"%05d : %s ", it->second, (it->first).c_str());
   		for(int j = 20 - (it->first).size();j > 0;j--) fputs(" ",fp);
		if(i % print_num == 0) fputs("\n",fp);
	}
	if(i % print_num != 0) fputs("\n",fp);
	printf("具体所有单词统计结果请查看本目录下'res1.txt'文件\n");
}
