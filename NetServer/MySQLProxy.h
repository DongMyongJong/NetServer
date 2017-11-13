// MySQLProxy.h: interface for the MySQLProxy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYSQLPROXY_H__1B9869AC_B186_43F4_AA9F_B11DF3DDF2D5__INCLUDED_)
#define AFX_MYSQLPROXY_H__1B9869AC_B186_43F4_AA9F_B11DF3DDF2D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _DEBUG
#pragma comment( lib, "../libs/mysql/debug/libmysql.lib" )
#else
#pragma comment( lib, "../libs/mysql/opt/libmysql.lib" )
#endif

#include "NetServer.h"
#include "mysql.h"
#include <map>

using namespace std;

class MySQLProxy  
{
public:
	MySQLProxy();
	virtual ~MySQLProxy();
	void writeSensorData(PPACKET packet);
	void writeDeviceStatus(int device, bool status);
	bool init(const char *schema, const char *ip, int port, const char *user, const char *password);
private:
	MYSQL *_mysql;
	map<int, string> _mapTransTable;
};

#endif // !defined(AFX_MYSQLPROXY_H__1B9869AC_B186_43F4_AA9F_B11DF3DDF2D5__INCLUDED_)
