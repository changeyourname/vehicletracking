# include "sdLogger.h"

#define SD_MOSI PA_7
#define SD_MISO PA_6
#define SD_SCLK PA_5
#define SD_SSEL PA_9
SDFileSystem sdFileSystem(SD_MOSI, SD_MISO, SD_SCLK, SD_SSEL, "sd");


SDLogger::SDLogger(const char* logCatName, const char* logFileName)
{
	sprintf(logCat, "%s",  logCatName);	
	sprintf(logFile, "%s", logFileName);	
};

void SDLogger::log(char* logmessage)
{
	time_t seconds = time(NULL);
	char timeStrBuffer[20];
	strftime(timeStrBuffer, 20, "%F %X\n", localtime(&seconds));

	char message[120];
	char logFullPath[200];

	sprintf(logFullPath, "/sd/%s", logCat);	
	mkdir(logFullPath, 0777);
	sprintf(logFullPath, "%s/%s",logFullPath, logFile);	
	FILE *logfile = fopen(logFullPath, "a+");

	if(logfile != NULL)
	{
		sprintf(message, "%s %s",timeStrBuffer, logmessage);
		//Write the entry to the log file and console
		fprintf(logfile, "%s\n", message);
		fclose(logfile);
	}
	// print to the serial port too:
	//Serial.println(message);
}

int SDLogger::log_printf(char *fmt, ...)
{
    int ret = 0;

	time_t seconds = time(NULL);
	char timeStrBuffer[20];
	strftime(timeStrBuffer, 20, "%F %X\n", localtime(&seconds));

	char logFullPath[200];

	sprintf(logFullPath, "/sd/%s", logCat);	
	mkdir(logFullPath, 0777);
	sprintf(logFullPath, "%s/%s",logFullPath, logFile);	
	FILE *logfile = fopen(logFullPath, "a+");

	if(logfile != NULL)
	{
		fprintf(logfile, "%s - ", timeStrBuffer);

	    /* Declare a va_list type variable */
	    va_list myargs;

	    /* Initialise the va_list variable with the ... after fmt */
	    va_start(myargs, fmt);

	    /* Forward the '...' to vprintf */
	    ret = vfprintf(logfile, fmt, myargs);

	    /* Clean up the va_list */
	    va_end(myargs);

	    fclose(logfile);
	}
    return ret;
}

SDLogger sdLogger("logs", "log");

// void logThis(char* logmessage){
// char message[120];
// DateTime now = rtc.now();
// long epoch = now.unixtime();
// int Year = now.year();
// int Month = now.month();
// int Day = now.day();
// int Hour = now.hour();
// int Minute = now.minute();
// int Second = now.second();
// sprintf(message, "%ld,%d/%d/%d %02d:%02d:%02d,%s",epoch,Year,Month,Day,Hour,Minute,Second,logmessage );
// //Write the entry to the log file and console
// dataFile.println(message);
// dataFile.flush();
// // print to the serial port too:
// Serial.println(message);
// }

