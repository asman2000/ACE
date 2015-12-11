#include "performance.h"
#ifdef GAME_DEBUG

#include "managers/timer.h"
#include "managers/memory.h"
#include "managers/log.h"


/**
 *
 */
tAvg* logAvgCreate(char *szName, UWORD uwCount) {
	tAvg *pAvg = memAllocFast(sizeof(tAvg));
	pAvg->szName = szName;
	pAvg->uwCount = uwCount;
	pAvg->uwCurrDelta = 0;
	pAvg->pDeltas = memAllocFast(uwCount*sizeof(tAvg));
	pAvg->ulMin = 0xFFFFFFFF;
	pAvg->ulMax = 0;
	return pAvg;
}

/**
 *
 */
void logAvgDestroy(tAvg *pAvg) {
	logAvgWrite(pAvg);
	memFree(pAvg->pDeltas, pAvg->uwCount*sizeof(tAvg));
	memFree(pAvg, sizeof(tAvg));
}

/**
 *
 */
void logAvgBegin(tAvg *pAvg) {
	pAvg->ulStartTime = timerGetPrec();
}

/**
 *
 */
void logAvgEnd(tAvg *pAvg) {
	/* Calculate timestamp */
	pAvg->pDeltas[pAvg->uwCurrDelta] = timerGetDelta(pAvg->ulStartTime, timerGetPrec());
	/* Update min/max */
	if(pAvg->pDeltas[pAvg->uwCurrDelta] > pAvg->ulMax)
		pAvg->ulMax = pAvg->pDeltas[pAvg->uwCurrDelta];
	if(pAvg->pDeltas[pAvg->uwCurrDelta] < pAvg->ulMin)
		pAvg->ulMin = pAvg->pDeltas[pAvg->uwCurrDelta];
	++pAvg->uwCurrDelta;
	/* Roll */
	if(pAvg->uwCurrDelta == pAvg->uwCount)
		pAvg->uwCurrDelta = 0;
}

/**
 *
 */
void logAvgWrite(tAvg *pAvg) {
	UWORD i;
	ULONG ulAvg;
	char szAvg[15];
	char szMin[15];
	char szMax[15];
	
	/* Calculate average time */
	for(i = pAvg->uwCount; i--;)
		ulAvg += pAvg->pDeltas[i];
	ulAvg /= pAvg->uwCount;
	
	/* Display info */
	timerFormatPrec(szAvg, ulAvg);
	timerFormatPrec(szMin, pAvg->ulMin);
	timerFormatPrec(szMax, pAvg->ulMax);
	logWrite("Avg %s: %s, min: %s, max: %s\n", pAvg->szName, szAvg, szMin, szMax);
}
#endif /* end of GAME_DEBUG */
