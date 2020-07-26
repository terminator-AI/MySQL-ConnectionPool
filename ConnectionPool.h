#pragma once
#include<iostream>
#include<queue>
#include<mutex>
#include<atomic>
#include<thread>
#include<functional>
#include<condition_variable>
#include"Connection.h"
using std::mutex;
using std::atomic_int;
using std::queue;
using std::thread;
using std::bind;
using std::unique_lock;
using std::shared_ptr;
using std::cv_status;
using std::condition_variable;
//连接池类
class ConnectionPool
{
public:
	//获取连接池对象
	static ConnectionPool* getConnectionPool();
	//获取队列中的连接
	shared_ptr<Connection> getConnection();
private:
	ConnectionPool();
	//~ConnectionPool();
	//加载配置信息
	bool loadConfigFile();
	//生产连接线程函数
	void producerConnectionTask();
	//扫描队列闲余（闲余时间超时）连接
	void scannerConnectionTask();
	//连接数据库信息
	string _ip;
	unsigned short _port;
	string _username;
	string _password;
	string _dbname;
	//连接池基本属性
	int _initSize;
	int _maxSize;
	int _maxIdleTime;
	int _connectionTimeOut;
	//连接池队列
	queue<Connection*> _connectionQue;
	//连接池队列线程安全互斥锁
	mutex _queueMutex;
	//连接池连接数
	atomic_int _connectionCount;
	//线程间同步
	condition_variable _cv;
};