#ifndef ACE_PERFORMANCE_LOG_H
#define ACE_PERFORMANCE_LOG_H

#include <exec/types.h>

typedef struct {
	UWORD uwCount;
	UWORD uwCurrDelta;
	ULONG ulMin;
	ULONG ulMax;
	ULONG ulStartTime;
	ULONG *pDeltas;
	char *szName;
} tAvg;

/* Functions - average block time */

tAvg* logAvgCreate(char* szName, UWORD uwCount);
void logAvgDestroy(tAvg* pAvg);
void logAvgBegin(tAvg* pAvg);
void logAvgEnd(tAvg* pAvg);
void logAvgWrite(tAvg* pAvg);

/* performance */



#else

#define logAvgCreate(szName, wCount)
#define logAvgDestroy(pAvg)
#define logAvgBegin(pAvg)
#define logAvgEnd(pAvg)
#define logAvgWrite(pAvg)



#endif /* end of ACE_PERFORMANCE_LOG_H */
