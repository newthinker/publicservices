/*
 * Copyright(c)2010, michael
 * All rights reserved
 *
 *  File  name : CException.h
 *  Discription:
 *  Created  on: Dec 30, 2010
 *       Author: michael, zuow11@gmail.com
 */

#ifndef CEXCEPTION_H_
#define CEXCEPTION_H_

#include <stdarg.h>
#include <time.h>

// 日志类型
enum LogType
{
	P = 1,		// prompt
	M = 2,		// message
	W = 3,		// warning
	E = 4		// error
};

void GetCurrentTime(char* timestring);

void FormatLog(enum LogType type, const char * msg, va_list args);

void CException(enum LogType type, const char* msg, ...);

#endif /* CEXCEPTION_H_ */
