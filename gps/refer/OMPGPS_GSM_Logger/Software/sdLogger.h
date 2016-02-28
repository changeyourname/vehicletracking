#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <stdarg.h>
#include "SDFileSystem.h"

class logger : public Stream 
{
};

class SDLogger
{
private:
	
protected:
	char logCat [20];
	char logFile[20];	
public:
	SDLogger(const char* logCatName, const char* logFileName);

	void log(char* logmessage);
	int log_printf(char *fmt, ...);

};

extern SDLogger sdLogger;

#endif