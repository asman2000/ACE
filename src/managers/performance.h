#ifndef ACE_PERFORMANCE_LOG_H
#define ACE_PERFORMANCE_LOG_H

#include "config.h"

#ifndef GAME_DEBUG
	/* release */

#define logAvgCreate(szName, wCount)
#define logAvgDestroy(pAvg)
#define logAvgBegin(pAvg)
#define logAvgEnd(pAvg)
#define logAvgWrite(pAvg)

#else	/* GAME_DEBUG */
	/* debug */

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

#endif /* enf of GAME_DEBUG */

#endif /* end of ACE_PERFORMANCE_LOG_H */
