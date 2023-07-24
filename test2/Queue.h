#pragma once
const int maxSize = 50;  //顺序队列的最大元素个数
 
template <class T>
class Queue
{
public:
	Queue() {};
	~Queue() {};
	virtual bool Push(const T& x) = 0; //新元素入队
	virtual bool Pop(T& x) = 0;       //队头元素出队
	virtual bool GetFront(T& x) = 0;      //获取队头元素
	virtual bool IsEmpty() const = 0;          //判断队列是否为空
	virtual bool IsFull() const = 0;            //判断队列是否满。顺序队列需要，链式队列不需要，但仍要重写
	virtual int Length() const = 0;            //求队列元素的个数
};
 
//用struct定义节点，链式队才用的到
template <class T>
struct LinkNode {
	T data; //数据域
	LinkNode<T> * link; //指针域，指向下一个节点
	LinkNode() //仅初始化指针的构造函数
	{
		LinkNode<T> *ptr = nullptr;
		link = ptr;
	}
	LinkNode(const T& item, LinkNode<T> *ptr = nullptr) //初始化数据和指针成员的构造函数
	{
		data = item;
		link = ptr;
	}
};