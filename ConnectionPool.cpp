#include"ConnectionPool.h"

ConnectionPool* ConnectionPool::getConnectionPool()
{
	//连接池对象
	static ConnectionPool pool;
	return &pool;
}

bool ConnectionPool::loadConfigFile()
{
	FILE* fp = fopen("MConfigFile.ini", "r");
	if (fp == nullptr)
	{//配置文件打开失败
		LOG("Config File is not exist!");
		return false;
	}
	while (!feof(fp))
	{//读取配置文件信息
		char str[1024] = { 0 };
		fgets(str, 1024, fp);
		string line = str;
		size_t idx = line.find('=', 0);
		if (idx == -1)
		{
			continue;
		}
		size_t end = line.find('\n', idx);
		string key = line.substr(0, idx);
		string value = line.substr(idx + 1, end - idx - 1);
		if (key == "_ip")
		{
			_ip = value;
		}
		else if (key == "_port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "_username")
		{
			_username = value;
		}
		else if (key == "_password")
		{
			_password = value;
		}
		else if (key == "_dbname")
		{
			_dbname = value;
		}
		else if (key == "_initSize")
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "_maxSize")
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "_maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "_connectionTimeOut")
		{
			_connectionTimeOut = atoi(value.c_str());
		}
	}
	fclose(fp);
	return true;
}

ConnectionPool::ConnectionPool()
{
	if (!loadConfigFile())
	{//配置文件加载失败
		return;
	}
	for (int i = 0;i < _initSize;++i)
	{//初始化连接池
		//unique_lock<mutex> lock(_queueMutex);
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime();
		_connectionQue.push(p);
		_connectionCount++;
	}
	//生产连接线程
	thread producer(bind(&ConnectionPool::producerConnectionTask, this));
	producer.detach();
	//扫描闲时超时连接线程
	thread scanner(bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();
}

void ConnectionPool::producerConnectionTask()
{
	for (;;)
	{//循环监视生产连接
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			_cv.wait(lock);
		}
		if (_connectionCount < _maxSize)
		{
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();
			_connectionQue.push(p);
			_connectionCount++;
		}
		_cv.notify_all();
	}
}

void ConnectionPool::scannerConnectionTask()
{
	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCount > _initSize)
		{
			Connection* p = _connectionQue.front();
			if (p->getConnectionAliveTime() >= _maxIdleTime * 1000)
			{
				_connectionQue.pop();
				_connectionCount--;
				delete p;
			}
			else
			{
				break;
			}
		}
	}
}

shared_ptr<Connection> ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		if (cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeOut)))
		{//获取连接超时
			if (_connectionQue.empty())
			{
				LOG("Connection time out...");
				return nullptr;
			}
		}
	}
	shared_ptr<Connection> sp(_connectionQue.front(),
		[this](Connection* pcon) {
			//连接使用后回退到连接池中
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime();
			_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	_cv.notify_all();
	return sp;
}
