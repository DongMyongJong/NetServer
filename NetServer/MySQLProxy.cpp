// MySQLProxy.cpp: implementation of the MySQLProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "MySQLProxy.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MySQLProxy::MySQLProxy()
{

}

MySQLProxy::~MySQLProxy()
{
	mysql_close(_mysql);
}

bool MySQLProxy::init(const char *schema, const char *ip, int port, const char *user, const char *password)
{
	_mysql = mysql_init((MYSQL*)NULL);
	if (_mysql == NULL)
	{
		fprintf(s_logFile, "----- Failed init MySQL!");
		return false;
	}
	if (!mysql_real_connect(_mysql, ip, user, password, schema, port, NULL, 0)) {
		fprintf(s_logFile, "----- Failed connect MySQL Server!\n");
		return false;
	}
	MYSQL_RES* result = mysql_store_result(_mysql);
	MYSQL_ROW row;
	while ((row = mysql_fetch_row(result))) {
		int sensor = atoi(row[0]);
		string location = row[1];
		_mapTransTable.insert(make_pair(sensor, location));
	}
	return true;
}

void MySQLProxy::writeSensorData(PPACKET packet)
{
	char sql[1024];
	int device = packet->device;
	int channel = packet->channel;
	int sensor = packet->sensor;
	sprintf(sql, "select * from fa_sensor where device=%d and channel=%d and sensor=%d;", device, channel, sensor);
	if (mysql_query(_mysql, sql) != 0) {
		printf("Failed excute SQL statement(select)!\n");
		return;
	}
	MYSQL_RES* result = mysql_store_result(_mysql);
	if(result->row_count == 0) {
		sprintf(sql, "insert into fa_sensor(device, channel, section, sensor, status) values (%d, %d, %d, %d, %d);", packet->device, packet->channel, packet->section, packet->sensor, packet->status);
	} else {
		sprintf(sql, "update fa_sensor set section=%d, status=%d where device=%d and channel=%d and sensor=%d;", packet->section, packet->status, packet->device, packet->channel, packet->sensor);
	}
	if (mysql_query(_mysql, sql) != 0) {
		printf("Failed excute SQL statement(insert/update)!\n");
		return;
	}
	mysql_free_result(result);
	sprintf(sql, "update fa_location set %s=%d", _mapTransTable[sensor].c_str(), (packet->status == 2)?1:0);
	if (mysql_query(_mysql, sql) != 0) {
		printf("Failed excute SQL statement(fa_location)!\n");
		return;
	}
	printf("Writing sensor data successfully!...\n");
}

void MySQLProxy::writeDeviceStatus(int device, bool status)
{
	char sql[1024];
	sprintf(sql, "select * from fa_device where device=%d;", device);
	if (mysql_query(_mysql, sql) != 0) {
		printf("Failed excute SQL statement(select)!\n");
		return;
	}
	MYSQL_RES* result = mysql_store_result(_mysql);
	if(result->row_count == 0) {
		sprintf(sql, "insert into fa_device(device, status) values (%d, %d);", device, (status)?1:0);
	} else {
		sprintf(sql, "update fa_device set status=%d where device=%d;", (status)?1:0, device);
	}
	if (mysql_query(_mysql, sql) != 0) {
		printf("Failed excute SQL statement(insert/update)!\n");
		return;
	}
	printf("Writing device status successfully!...\n");
}