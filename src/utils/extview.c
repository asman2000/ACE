#include "extview.h"

/**
 * Creates blank tView
 */
tView *viewCreate(UWORD uwFlags) {
	
	tView *pView = memAllocFast(sizeof(tView));
	
	logBlockBegin("viewCreate(uwFlags: %u)", uwFlags);
	
	logWrite("addr: %p\n", pView);
	
	/* Fill fields */
	pView->ubVpCount = 0;
	pView->uwFlags = uwFlags;
	pView->pFirstVPort = 0;
	pView->pCopList = copListCreate();
	
	logBlockEnd("viewCreate()");

	return pView;
}

/**
 * Destroys given tView along with attached viewports
 */
void viewDestroy(tView *pView) {
	logBlockBegin("viewDestroy(pView: %p)", pView);
	
	if(g_sCopManager.pCopList == pView->pCopList)
		viewLoad(0);
	
	/* Free all attached viewports */
	while(pView->pFirstVPort)
		vPortDestroy(pView->pFirstVPort);
	
	/* Free view */
	copListDestroy(pView->pCopList);
	memFree(pView, sizeof(tView));
	logBlockEnd("viewDestroy()");
}

/* Wywo�uje process wszystkich mened�er�w podpi�tych do ka�dego viewporta w ob�bie viewa */

void viewProcessManagers(tView *pView) {
	tVPort *pVPort;
	tVpManager *pManager;
	pVPort = pView->pFirstVPort;
	while(pVPort) {
		pManager = pVPort->pFirstManager;
		while(pManager) {
			pManager->process(pManager);
			pManager = pManager->pNext;
		}
		pVPort = pVPort->pNext;
	}
}

void viewUpdateCLUT(tView *pView) {
	if(pView->uwFlags & V_GLOBAL_CLUT)
		CopyMem(pView->pFirstVPort->pPalette, custom.color, sizeof(UWORD)<<pView->pFirstVPort->ubBPP);
	else {
		/* na p�tli: vPortUpdateCLUT(); */
	}
}

void viewLoad(tView *pView) {
	logBlockBegin("viewLoad(pView: %p)", pView);
	WaitTOF();
	{
	UBYTE i;
	ULONG uwDMA;
	if(!pView) {
		g_sCopManager.pCopList = g_sCopManager.pBlankList;
		uwDMA = DMAF_RASTER;
		custom.bplcon0 = 0;
		
		for(i = 0; i != 5; ++i)
			custom.bplpt[1] = 0;
		custom.bpl1mod = 0;
		custom.bpl2mod = 0;
	}
	else {
		g_sCopManager.pCopList = pView->pCopList;
		custom.bplcon0 = pView->pFirstVPort->ubBPP << 12;
		custom.diwstrt = 0x2C81; /* VSTART: 0x2C, HSTART: 0x81 */
		custom.diwstop = 0x2CC1; /* VSTOP: 0x2C, HSTOP: 0xC1 */
		viewUpdateCLUT(pView);
		uwDMA = DMAF_SETCLR | DMAF_RASTER;
	}
	copProcess();
	custom.copjmp1 = 1;
	custom.dmacon = uwDMA;
	WaitTOF();
	logBlockEnd("viewLoad()");
	}
}


/**
 * Creates new tVPort inside given view with supplied dimensions and BPP
 * Current implementation requires 2-line space between viewports,
 * as system viewport and ACE viewport manager copperlists collide
 */
tVPort *vPortCreate(tView *pView, UWORD uwWidth, UWORD uwHeight, UBYTE ubBPP, UWORD uwFlags) {
	tVPort *pVPort;
	logBlockBegin("vPortCreate(pView: %p, uwWidth: %u, uwHeight: %u, ubBPP: %hu, uwFlags: %u)", pView, uwWidth, uwHeight, ubBPP, uwFlags);
	
	pVPort = memAllocFastClear(sizeof(tVPort));
	logWrite("Addr: %p\n", pVPort);
	
	/* Initial field fill */
	pVPort->pView = pView;
	pVPort->pNext = 0;
	pVPort->uwOffsX = 0; /* TODO: implement non-zero */
	pVPort->uwWidth = uwWidth;
	pVPort->uwHeight = uwWidth;
	pVPort->ubBPP = ubBPP;
	pVPort->pFirstManager = 0;
	
	/* Alloc palette */
	pVPort->pPalette = memAllocFastClear(sizeof(UWORD) << ubBPP);
	
	/* Calculate Y offset - beneath previous ViewPort */
	pVPort->uwOffsY = 0;
	{
	tVPort *pPrevVPort = pView->pFirstVPort;
	while(pPrevVPort) {
		pVPort->uwOffsY += pPrevVPort->uwHeight;
		pPrevVPort = pPrevVPort->pNext;
	}
	if(pVPort->uwOffsY)
		pVPort->uwOffsY += 2; /* TODO: not always required */
	logWrite("Offsets: %ux%u\n", pVPort->uwOffsX, pVPort->uwOffsY);
	
		
	/* Update view - add to vPort list */
	++pView->ubVpCount;
	if(!pView->pFirstVPort) {
		pView->pFirstVPort = pVPort;
		logWrite("No prev VPorts - added to head\n");
	}
	else {
		pPrevVPort = pView->pFirstVPort;
		while(pPrevVPort->pNext)
			pPrevVPort = pPrevVPort->pNext;
		pPrevVPort->pNext = pVPort;
		logWrite("VPort added after %p\n", pPrevVPort);
	}
	}
	logBlockEnd("vPortCreate()");
	return pVPort;
}

/**
 * Destroys given tVPort along with attached managers
 */
void vPortDestroy(tVPort *pVPort) {
	
	tView *pView;
	tVPort *pPrevVPort, *pCurrVPort;
	logBlockBegin("vPortDestroy(pVPort: %p)", pVPort);
	
	pView = pVPort->pView;
	logWrite("Parent extView: %p\n", pView);
	pPrevVPort = 0;
	pCurrVPort = pView->pFirstVPort;
	while(pCurrVPort) {
		logWrite("found VP: %p...", pCurrVPort);
		if(pCurrVPort == pVPort) {
			logWrite(" gotcha!\n");
			
			/* Remove from list */
			if(pPrevVPort)
				pPrevVPort->pNext = pCurrVPort->pNext;
			else
				pView->pFirstVPort = pCurrVPort->pNext;
			--pView->ubVpCount;
			
			/* Destroy managers */
			logBlockBegin("Destroying managers");
			while(pCurrVPort->pFirstManager)
				vPortRmManager(pCurrVPort, pCurrVPort->pFirstManager);
			logBlockEnd("Destroying managers");
			
			/* Free stuff */
			memFree(pVPort->pPalette, sizeof(UWORD) << pVPort->ubBPP);
			memFree(pVPort, sizeof(tVPort));
			break;
		}
		else
			logWrite("\n");
		pPrevVPort = pCurrVPort;
		pCurrVPort = pCurrVPort->pNext;
	}
	logBlockEnd("vPortDestroy()");
}

void vPortUpdateCLUT(tVPort *pVPort) {
	/* TODO: blok palety kolor�w, priorytety na copperli�cie */
}

/* Doczepia mened�er do viewporta */
void vPortAddManager(tVPort *pVPort, tVpManager *pVpManager) {
	/* podpi�cie */
	if(!pVPort->pFirstManager) {
		pVPort->pFirstManager = pVpManager;
		logWrite("Manager %p attached to head of VP %p\n", pVpManager, pVPort);
		return;
	}
	{
	tVpManager *pVpCurr = pVPort->pFirstManager;
	/* przewi� przed mened�er o wy�szym numerze ni� wstawiany */
	while(pVpCurr->pNext && pVpCurr->pNext->ubId <= pVpManager->ubId) {
		if(pVpCurr->ubId <= pVpManager->ubId) {
			pVpCurr = pVpCurr->pNext;
		}
	}
	pVpManager->pNext = pVpCurr->pNext;
	pVpCurr->pNext = pVpManager;
	logWrite("Manager %p attached after manager %p of VP %p\n", pVpManager, pVpCurr, pVPort);
	}
}

/* Usuwa mened�er z viewporta, wywo�uje jego destroy */
void vPortRmManager(tVPort *pVPort, tVpManager *pVpManager) {
	if(!pVPort->pFirstManager) {
		logWrite("ERR: vPort %p has no managers!\n", pVPort);
		return;
	}
	if(pVPort->pFirstManager == pVpManager) {
		logWrite("Destroying manager %u @addr: %p\n", pVpManager->ubId, pVpManager);
		pVPort->pFirstManager = pVpManager->pNext;
		pVpManager->destroy(pVpManager);
		return;
	}
	{
	tVpManager *pParent = pVPort->pFirstManager;
	while(pParent->pNext) {
		if(pParent->pNext == pVpManager) {
			logWrite("Destroying manager %u @addr: %p\n", pVpManager->ubId, pVpManager);
			pParent->pNext = pVpManager->pNext;
			pVpManager->destroy(pVpManager);
			return;
		}
	}
	}
	logWrite("ERR: vPort %p manager %p not found!\n", pVPort, pVpManager);
}

tVpManager *vPortGetManager(tVPort *pVPort, UBYTE ubId) {
	tVpManager *pManager;
	
	pManager = pVPort->pFirstManager;
	while(pManager) {
		if(pManager->ubId == ubId)
			return pManager;
		pManager = pManager->pNext;
	}
	return 0;
}

/*
void extViewFadeOut(tExtView *pExtView) {
	tExtVPort *pVPort;
	BYTE bFadeStep;
	UBYTE ubColorIdx;
	UWORD pTmpPalette[32]; // TODO: view bpp aware
	UBYTE ubR, ubG, ubB;
	
	for (bFadeStep = 15; bFadeStep >= 0; --bFadeStep) {
		pVPort = (tExtVPort*)pExtView->sView.ViewPort;
		while(pVPort) {
			for (ubColorIdx = 0; ubColorIdx != 32; ++ubColorIdx) {
				// Wy�uskanie sk�adowych
				ubR = (pVPort->pPalette[ubColorIdx] >> 8) & 0xF;
				ubG = (pVPort->pPalette[ubColorIdx] >> 4) & 0xF;
				ubB = (pVPort->pPalette[ubColorIdx] >> 0) & 0xF;
				// Przemnozenie i sciecie skladowych
				ubR = ((ubR * bFadeStep) >> 4) & 0xF;
				ubG = ((ubG * bFadeStep) >> 4) & 0xF;
				ubB = ((ubB * bFadeStep) >> 4) & 0xF;
				// Zlozenie w kolor
				pTmpPalette[ubColorIdx] = (ubR << 8) + (ubG << 4) + (ubB << 0);
			}
			LoadRGB4(&pVPort->sVPort, pTmpPalette, pVPort->sVPort.ColorMap->Count);
			pVPort = (tExtVPort*)pVPort->sVPort.Next;
		}
		WaitTOF();
	}

	// Zapisz trwale palet� aktualnego bufora
	// MakeVPort(pExtView->pView, pExtView->pMainViewPort);
	// MrgCop(pExtView->pView);

	// Zaktualizuj palet� drugiego bufora
	// swapScreenBuffers(pExtView);
	// LoadRGB4(pExtView->pMainViewPort, pTmpPalette, 1 << WINDOW_SCREEN_BPP);
	// MakeVPort(pExtView->pView, pExtView->pMainViewPort);
	// MrgCop(pExtView->pView);

	// Wr�� do bufora wyj�ciowego
	// swapScreenBuffers(pExtView);
}

void extViewFadeIn(tExtView *pExtView) {
	tExtVPort *pVPort;
	BYTE bFadeStep;
	UBYTE ubColorIdx;
	UWORD pTmpPalette[32]; // TODO: view bpp aware
	UBYTE ubR, ubG, ubB;
	
	for (bFadeStep = 0; bFadeStep <= 15; ++bFadeStep) {
		pVPort = (tExtVPort*)pExtView->sView.ViewPort;
		while(pVPort) {
			for(ubColorIdx = 0; ubColorIdx != 32; ++ubColorIdx) {
				// Wyluskanie skladowych
				ubR = (pVPort->pPalette[ubColorIdx] >> 8) & 0xF;
				ubG = (pVPort->pPalette[ubColorIdx] >> 4) & 0xF;
				ubB = (pVPort->pPalette[ubColorIdx] >> 0) & 0xF;
				// Przemnozenie i sciecie skladowych
				ubR = ((ubR * bFadeStep) >> 4) & 0xF;
				ubG = ((ubG * bFadeStep) >> 4) & 0xF;
				ubB = ((ubB * bFadeStep) >> 4) & 0xF;
				// Zlozenie w kolor
				pTmpPalette[ubColorIdx] = (ubR << 8) + (ubG << 4) + (ubB << 0);
			}
			LoadRGB4(&pVPort->sVPort, pTmpPalette, pVPort->sVPort.ColorMap->Count);
			pVPort = (tExtVPort*)pVPort->sVPort.Next;
		}
		WaitTOF();
	}

	WaitTOF();
	pVPort = (tExtVPort*)pExtView->sView.ViewPort;
	while(pVPort) {
		LoadRGB4(&pVPort->sVPort, pVPort->pPalette, pVPort->sVPort.ColorMap->Count);
		pVPort = (tExtVPort*)pVPort->sVPort.Next;
	}
}*/
