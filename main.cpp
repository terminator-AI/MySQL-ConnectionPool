#include<iostream>
#include"ConnectionPool.h"
int main()
{
	ConnectionPool* sp = ConnectionPool::getConnectionPool();
	for (int i = 0;i < 100;++i)
	{
		shared_ptr<Connection>p = sp->getConnection();
		p->update("insert into user(name,age,sex) values('Tom',21,'male'");
	}
	return 0;
}