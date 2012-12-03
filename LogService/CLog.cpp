#include "CLog.h"

CLog::CLog()
{
	m_pLog = NULL;
}

CLog::~CLog()
{
}

int CLog::InitLog(char *strPath)
{
	char strLogFileName[1024];
	try
	{
		sprintf(strLogFileName, "%s%s%s", strPath, SEPARATOR, LOG_FILE_NAME);
		m_pLog=fopen(strLogFileName, "wt");
		if(m_pLog==NULL)
		{
			printf("Invalid log file path:%s\n", strLogFileName);
			return FAIL;
		}
	}
	catch(...)
	{
		return FAIL;
	}

	return SUCCESS;
}

int CLog::InitLog(std::string strPath)
{
	char strLogFileName[1024];
	try
	{
		sprintf(strLogFileName, "%s%s%s", strPath.c_str(), SEPARATOR, LOG_FILE_NAME);
		m_pLog=fopen(strLogFileName, "wt");
		if(m_pLog==NULL)
		{
			printf("Invalid log file path:%s\n", strLogFileName);
			return FAIL;
		}
		PrintLog(P, "<<<<<<<<<<<Process begin!>>>>>>>>>>>");
	}
	catch(...)
	{
		return FAIL;
	}

	return SUCCESS;
}

int CLog::PrintLog(LogType nLogType, const char * pLog, ...)
{
	try
	{
		if(IsNull()!=0)
		{
			fprintf(m_pLog, "Invalid log file path and set as default!");
			fflush(m_pLog);
		}
		va_list args;	// 变参链表 [ZuoW,2010/3/11]

		va_start(args, pLog);	// 将可变参数转换位变参链表
		// 组织日志语句
		FormatLog(nLogType, pLog, args);

		va_end(args);

		// 输出日志到文件
		fflush(m_pLog);

	}
	catch(...)
	{
		return FAIL;
	}

	return SUCCESS;
}

int CLog::PrintLog(LogType nLogType, std::string strLog,...)
{
	try
	{
		if(IsNull()!=0)
		{
			fprintf(m_pLog, "Invalid log file path and set as default!");
			fflush(m_pLog);
		}

		va_list args;	// 可变参数链表 [ZuoW,2010/3/11]
		va_start(args, strLog);		/// va_start 的第二个参数不是最后一个有名参数 [ZuoW,2010/6/8]

		FormatLog(nLogType, strLog.c_str(), args);

		fflush(m_pLog);
	}
	catch(...)
	{
		return FAIL;
	}

	return SUCCESS;
}

void CLog::FormatLog(LogType nLogType, const char * pLog, va_list args)
{
	std::string buffer;
	vsprintf((char*)buffer.c_str(), pLog, args);

	std::string strTime;
	GetCurrentTime(strTime);
	switch(nLogType)
	{
		case P:  //prompt
		{
			fprintf(m_pLog, "[%s Prompt]%s\n", strTime.c_str(), buffer.c_str());
			break;
		}
		case M: //message
		{
			fprintf(m_pLog, "[%s Message]%s\n", strTime.c_str(), buffer.c_str());
			break;
		}
		case W: //warning
		{
			fprintf(m_pLog, "[%s Warning]%s\n", strTime.c_str(), buffer.c_str());
			break;
		}
		case E: //error
		{
			fprintf(m_pLog, "[%s Error]%s\n", strTime.c_str(), buffer.c_str());
			break;
		}
		default:
			break;
	}

	// Print the log
#ifdef _PRINT_LOG
	printf("%s:%s\n", strTime.c_str(), buffer.c_str());
#endif	
}

void CLog::Close()
{
	try
	{
		PrintLog(P, "<<<<<<<<<<<AGM end!>>>>>>>>>>>");
		fclose(m_pLog);
	}
	catch(...)
	{
		return;
	}
}

int CLog::GetCurrentTime(std::string &strTime)
{
	char cTime[256];
	memset(cTime, 0, sizeof(char)*256);

	time_t sec =  time(NULL);
	struct tm t=*localtime(&sec);
	sprintf(cTime, "%d-%02d-%02d %02d:%02d:%02d", t.tm_year+1900,
			t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

//	strcpy(strTime, cTime);
	strTime = cTime;

	return SUCCESS;
}

int CLog::GetCurrentTime(char* &cTime)
{
	time_t sec =  time(NULL);
	struct tm t=*localtime(&sec);
	sprintf(cTime, "%d-%02d-%02d %02d:%02d:%02d", t.tm_year+1900,
			t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

	return SUCCESS;
}

int CLog::IsNull()
{
	if(m_pLog==NULL)
	{
		printf("Have not init the log and set as default!\n");

		char buffer[1024] = {};
		if (access(DEFAULT_LOG_PATH, 0)!=0)
		{
			sprintf(buffer, "mkdir -p %s", DEFAULT_LOG_PATH);
			system(buffer);
		}
		sprintf(buffer, "%s%s%s", DEFAULT_LOG_PATH, SEPARATOR, LOG_FILE_NAME);
		m_pLog=fopen(buffer, "wt");
		if(m_pLog==NULL)
		{
			printf("Invalid log file path:%s\n", buffer);
			return FAIL;
		}
	}

	return SUCCESS;
}
