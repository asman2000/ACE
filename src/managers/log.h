#ifndef ACE_MANAGER_LOG_H
#define ACE_MANAGER_LOG_H

#include "config.h"

#include <stdio.h> // fopen etc
#include <string.h> // strlen etc
#include <stdarg.h> // va_list etc
#include <clib/exec_protos.h> // Amiga typedefs
#include <clib/graphics_protos.h> // Amiga typedefs


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

void logBlockBegin(char *szBlockName,...);
void logBlockEnd(char *szBlockName);


/* Functions - struct dump */

#else

# define logOpen()
# define logClose()
# define logPushIndent()
# define logPopIndent()
# define logWrite(char *szFormat,...)

# define logBlockBegin(char *szFormat,...)
# define logBlockEnd(szBlockName)

#endif

#endif /* end of ACE_MANAGER_LOG_H */
