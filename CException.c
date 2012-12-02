/*
 * Copyright(c)2010, michael
 * All rights reserved
 *
 *  File  name : CException.c
 *  Discription:
 *  Created  on: Dec 30, 2010
 *       Author: michael, zuow11@gmail.com
 */

//#include <containers.h>
#include <stdio.h>
#include <string.h>
#include "CException.h"

void GetCurrentTime(char* timestring)
{
	time_t sec =  time(NULL);
	struct tm t=*localtime(&sec);
	sprintf(timestring, "%d-%02d-%02d %02d:%02d:%02d", t.tm_year+1900,
			t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}

void FormatLog(enum LogType type, const char * msg, va_list args)
{
//	StringCollection buffer; 暂不考虑
	char buffer[1024];
	memset(buffer, 0, 1024);

	vsprintf(buffer, msg, args);

	char time[32];
	memset(time, 0, 32);
	GetCurrentTime(time);

	switch(type)
	{
		case P:  //prompt
		{
			fprintf(stderr, "[%s Prompt]%s\n", time, buffer);
			break;
		}
		case M: //message
		{
			fprintf(stderr, "[%s Message]%s\n", time, buffer);
			break;
		}
		case W: //warning
		{
			fprintf(stderr, "[%s Warning]%s\n", time, buffer);
			break;
		}
		case E: //error
		{
			fprintf(stderr, "[%s Error]%s\n", time, buffer);
			break;
		}
		default:
			break;
	}
}

void CException(enum LogType type, const char* msg, ...)
{

	va_list args;	// 变参链表

	va_start(args, msg);	// 将可变参数转换位变参链表

	FormatLog(type, msg, args);		// 组织日志语句并输出到终端

	va_end(args);

}
