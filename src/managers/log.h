#ifndef ACE_MANAGER_LOG_H
#define ACE_MANAGER_LOG_H

#include <stdio.h> // fopen etc
#include <string.h> // strlen etc
#include <stdarg.h> // va_list etc
#include <clib/exec_protos.h> // Amiga typedefs
#include <clib/graphics_protos.h> // Amiga typedefs

#include "managers/timer.h"




#ifdef GAME_DEBUG



/* Functions - general */

void logOpen(void);
void logClose(void);

void logPushIndent(void);
void logPopIndent(void);

void logWrite(
	IN char *szFormat,
	IN ...
);

/* Functions - block logging */

void logBlockBegin(
	IN char *szBlockName,
	IN ...
);
void logBlockEnd(
	IN char *szBlockName
);


/* Functions - struct dump */

void _logUCopList(
	IN struct UCopList *pUCopList
);
void _logBitMap(
	IN struct BitMap *pBitMap
);

/*# define logOpen() _logOpen()
# define logClose() _logClose()
# define logPushIndent() _logPushIndent()
# define logPopIndent() _logPopIndent()
# define logWrite(...) _logWrite(__VA_ARGS__)

# define logBlockBegin(...) _logBlockBegin(__VA_ARGS__)
# define logBlockEnd(szBlockName) _logBlockEnd(szBlockName)
*/

# define logUCopList(pUCopList) _logUCopList(pUCopList)
# define logBitMap(pBitMap) _logBitMap(pBitMap)

#else

# define logOpen()
# define logClose()
# define logPushIndent()
# define logPopIndent()
# define logWrite(char *szFormat,...)

# define logBlockBegin(char *szFormat...)
# define logBlockEnd(szBlockName)


# define logUCopList(pUCopList)
# define logBitMap(pBitMap)
#endif

#endif /* end of ACE_MANAGER_LOG_H */
