#ifndef CLOG_H_
#define CLOG_H_

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <string>
#include "define.h"

// 日志类型
enum LogType
{
	P = 1,
	M = 2,
	W = 3,
	E = 4
};

class CLog
{
// 接口函数
public:
	CLog();
	virtual ~CLog();
	int InitLog(char * strPath);
	int InitLog(std::string strPath);
	int PrintLog(LogType nLogType, const char * pLog, ...);
	int PrintLog(LogType nLogType, std::string strLog, ...);
	void Close();
	
// 内部成员函数
private:
	// 组织日志语句，传递变参。[ZuoW,2010/3/12]
	void FormatLog(LogType nLogType, const char * pLog, va_list args);
	int GetCurrentTime(std::string &strTime);
	int GetCurrentTime(char* &cTime);	
	int IsNull();	// 判断日志文件是否存在 [ZuoW,2010/3/12]	
	
private:
	FILE* m_pLog;
	
};

#endif /*CLOG_H_*/
