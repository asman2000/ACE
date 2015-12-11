#include "log.h"
#include "timer.h"
#ifdef GAME_DEBUG

#ifndef LOG_FILE_NAME
#define LOG_FILE_NAME "game.log"
#endif

#include <stdio.h> // fopen etc
#include <string.h> // strlen etc
#include <stdarg.h> // va_list etc
#include <clib/exec_protos.h> // Amiga typedefs
#include <clib/graphics_protos.h> // Amiga typedefs


typedef struct {
	FILE *pFile;
	UBYTE ubIndent;
	UBYTE ubIsLastWasInline;
	ULONG pTimeStack[256];
	char szTimeBfr[255];
	UBYTE ubBlockEmpty;
	UBYTE ubShutUp;
} tLogManager;

/*extern tLogManager g_sLogManager;*/



/* Globals */
tLogManager g_sLogManager;

/* Functions */

/**
 * Base debug functions
 */

void _logOpen() {
	g_sLogManager.pFile = fopen(LOG_FILE_NAME, "w");
	g_sLogManager.ubIndent = 0;
	g_sLogManager.ubIsLastWasInline = 0;
	g_sLogManager.ubBlockEmpty = 1;
	g_sLogManager.ubShutUp = 0;
}

void _logPushIndent() {
	++g_sLogManager.ubIndent;
}

void _logPopIndent() {
	--g_sLogManager.ubIndent;
}

void _logWrite(char *szFormat, ...) {
	if(g_sLogManager.ubShutUp)
		return;
	if (g_sLogManager.pFile) {
		g_sLogManager.ubBlockEmpty = 0;
		if (!g_sLogManager.ubIsLastWasInline) {
			UBYTE bLogIndent = g_sLogManager.ubIndent;
			while (bLogIndent--)
				fprintf(g_sLogManager.pFile, "\t");
		}

		g_sLogManager.ubIsLastWasInline = szFormat[strlen(szFormat) - 1] != '\n';
		
		va_list vaArgs;
		va_start(vaArgs, szFormat);
		vfprintf(g_sLogManager.pFile, szFormat, vaArgs);
		va_end(vaArgs);

		fflush(g_sLogManager.pFile);
	}
}

void _logClose() {
	if (g_sLogManager.pFile)
		fclose(g_sLogManager.pFile);
	g_sLogManager.pFile = 0;
}

/**
 * Extended debug functions
 */

// Log blocks
void _logBlockBegin(char *szBlockName, ...) {
	if(g_sLogManager.ubShutUp)
		return;
	char szFmtBfr[512];
	char szStrBfr[1024];
	// make format string
	strcpy(szFmtBfr, "Block begin: ");
	strcat(szFmtBfr, szBlockName);
	strcat(szFmtBfr, "\n");
	
	va_list vaArgs;
	va_start(vaArgs, szBlockName);
	vsprintf(szStrBfr,szFmtBfr,vaArgs);
	va_end(vaArgs);
	
	logWrite(szStrBfr);
	g_sLogManager.pTimeStack[g_sLogManager.ubIndent] = timerGetPrec();
	logPushIndent();
	g_sLogManager.ubBlockEmpty = 1;
}

void _logBlockEnd(char *szBlockName) {
	if(g_sLogManager.ubShutUp)
		return;
	logPopIndent();
	timerFormatPrec(
		g_sLogManager.szTimeBfr, 
		timerGetDelta(
			g_sLogManager.pTimeStack[g_sLogManager.ubIndent],
			timerGetPrec()
		)
	);
	if(g_sLogManager.ubBlockEmpty) {
		// empty block - collapse to single line
		g_sLogManager.ubIsLastWasInline = 1;
		fseek(g_sLogManager.pFile, -1, SEEK_CUR);
		logWrite("...OK, time: %s\n", g_sLogManager.szTimeBfr);
	}
	else
		logWrite("Block end: %s, time: %s\n", szBlockName, g_sLogManager.szTimeBfr);
	g_sLogManager.ubBlockEmpty = 0;
}




#endif /* end of GAME_DEBUG */
