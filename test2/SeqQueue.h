#pragma once
//顺序队列，存储形式是数组。为了有效利用内存，设计成循环队列
# include <iostream>
# include <assert.h>
# include "Queue.h"
using namespace std;
 
template <class T>
class SeqQueue :public Queue<T>   //继承时注意Queue也是类模板，后面加<T>区别普通的类
{
public:                                     //如果不加public,权限为private
	SeqQueue(int sz=10);                   //构造函数
	~SeqQueue() { delete[] elements; }  //[]紧跟delete，表明释放的elements地址是一个数组，而不是单个变量
	bool IsFull() const{ return ((rear + 1) % maxSize == 0) ? true : false; }//是否队满
	bool IsEmpty() const { return (rear = front) ? true : false; }  //是否队空
	bool Push(const T& x);           //新元素入队
	bool Pop(T& x);                 //队头元素出队
	bool GetFront(T& x);                //获取队头元素
	void makeEmpty() { rear = front = 0; }//清空队列(其实队列内容未被修改，只是将指针置0）
	int Length() const ;                        //求队列元素的个数
 
	template <class R>
	friend ostream & operator<<(ostream & out, SeqQueue<R> &sq);//重载<<输出队列内容
private:
	int rear, front;   //队尾和队首的指针，同链式栈一样，不是真正的指针，而是下标。front是第一个元素的下标，rear是最后一个元素下一个位置的坐标
	T * elements;      //指向存放数据的数组的指针
	int maxSize;       //队伍最大可容纳元素的个数
};
 
 
//构造函数
template <class T>
SeqQueue<T>::SeqQueue(int sz):front(0),rear(0),maxSize(sz)  //这里的sz不允许使用默认参数
{
	//建立一个最大容量为maxSize的空队列
	//rear = front = 0;
	//maxSize = sz;
	elements = new T[maxSize];
	assert(elements != nullptr);//断言函数，括号里的内容成立，则继续执行代码；否则，终止代码的执行
}
 
 
//新元素入队尾
template <class T>
bool SeqQueue<T>::Push(const T& x)
{
	if (IsFull() == true)
		return false;   //队列满则插入失败
	elements[rear] = x; //队尾位置插入新元素
	rear = (rear+1)%maxSize; //队尾指针更新
	return true;
}
 
 
//队头元素出队
template <class T>
bool SeqQueue<T>::Pop(T& x) 
{
	if (IsEmpty()) return false;
	x = elements[front];
	front = (front + 1) % maxSize; //队头指针加一
	return true;
}
 
 
//获取队头元素
template <class T>
bool SeqQueue<T>::GetFront(T &x)
{
	if (IsEmpty()) return false;
	x = elements[front];
	return true;
}
 
 
//求队列元素的个数
template <class T>
int SeqQueue<T>::Length() const
{
	return (rear - front + maxSize) % maxSize; 
	//加maxSize是为了保证队列只入队且队列入满时rear=0，front=1，此时rear-front=-1，
	//此时元素个数为maxSize-1，由此加上maxSize可以得到正确结果
}
 
 
template <class R>
ostream & operator<<(ostream & out, SeqQueue<R> &sq)
{
	out << "front = " << sq.front << ",rear = " << sq.rear << endl;
	for (int i = sq.front; i != sq.rear; i = (i + 1) % sq.maxSize)
	{
		out << i << " : " << sq.elements[i] << endl;
	}
	return out;
}
 