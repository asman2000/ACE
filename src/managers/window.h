#ifndef GUARD_ACE_MANAGER_WINODW_H
#define GUARD_ACE_MANAGER_WINODW_H

#include <stdlib.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <graphics/gfxbase.h>

#include "config.h"

#include "utils/extview.h"
#include "managers/log.h"
#include "managers/memory.h"

#define CPR_SEEK_CUR 0
#define CPR_SEEK_SET 1
#define CPR_SEEK_END 2

/* Types */
typedef struct {
	struct Screen *pScreen;
	struct Window *pWindow;
	struct View *pSysView;  /* System view before running app and swapping views */
	UWORD pPalette[32];
} tWindowManager;

/* Globals */
extern tWindowManager g_sWindowManager;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/* Functions */
void windowCreate(void);

void windowDestroy(void);

void windowKill(
	IN char *szError
);

#endif
