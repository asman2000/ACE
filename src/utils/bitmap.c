#include "bitmap.h"

/* Globals */

/* Functions */

// AllocBitMap nie dzia�a na kick 1.3
tBitMap *bitmapCreate(UWORD uwWidth, UWORD uwHeight, UBYTE ubDepth, UBYTE ubFlags) {
	tBitMap *pBitMap;
	UBYTE i;
	
	logBlockBegin("bitmapCreate(uwWidth: %u, uwHeight: %u, ubDepth: %hu, ubFlags: %hu)", uwWidth, uwHeight, ubDepth, ubFlags);
	pBitMap = (tBitMap*) memAllocFastClear(sizeof(tBitMap));
	logWrite("addr: %p\n", pBitMap);

	InitBitMap(pBitMap, ubDepth, uwWidth, uwHeight);
	
	if(ubFlags & BMF_INTERLEAVED) {
		UWORD uwRealWidth;
		uwRealWidth = pBitMap->BytesPerRow;
		pBitMap->BytesPerRow *= ubDepth;
		
		pBitMap->Planes[0] = (PLANEPTR) memAllocChip(pBitMap->BytesPerRow*uwHeight);
		for(i = 1; i != ubDepth; ++i)
			pBitMap->Planes[i] = pBitMap->Planes[i-1] + uwRealWidth;
		
		if (ubFlags & BMF_CLEAR)
			BltClear(pBitMap->Planes[0], (pBitMap->Rows << 16) | pBitMap->BytesPerRow, 3L);
	}
	else
		for(i = ubDepth; i--;) {
			pBitMap->Planes[i] = (PLANEPTR) memAllocChip(pBitMap->BytesPerRow*uwHeight);
			if (ubFlags & BMF_CLEAR)
				BltClear(pBitMap->Planes[i], (pBitMap->Rows << 16) | pBitMap->BytesPerRow, 3L);
		}

	if (ubFlags & BMF_CLEAR)
		WaitBlit();

	logBlockEnd("bitmapCreate()");
	return pBitMap;
}

tBitMap *bitmapCreateFromFile(char *szFileName) {
	tBitMap *pBitMap = 0;
	FILE *pFile;
	UWORD uwWidth, uwHeight;            // wymiary obrazka
	UWORD uwCopperLength, uwCopperSize; // copperlista - raczej niepotrzebne, usun�� p�niej ze specyfikacji pliku
	UBYTE ubPlaneCount;                 // liczba bitplanes�w
	UBYTE i;
	
	logBlockBegin("bitmapCreateFromFile(szFileName: %s)", szFileName);
	pFile = fopen(szFileName, "r");
	if(!pFile) {
		logWrite("File does not exist: %s\n", szFileName);
		logBlockEnd("bitmapCreateFromFile()");
		return 0;
	}
	logWrite("Addr: %p\n",pBitMap);
	
	// Nag��wek
	fread(&uwWidth, 2, 1, pFile);
	fread(&uwHeight, 2, 1, pFile);
	fread(&ubPlaneCount, 1, 1, pFile);
	fread(&uwCopperLength, 2, 1, pFile);
	fread(&uwCopperSize, 2, 1, pFile);
	
	// Init bitmapy
	pBitMap = bitmapCreate(uwWidth, uwHeight, ubPlaneCount, 0);
	for (i = 0; i != ubPlaneCount; ++i)
		fread(pBitMap->Planes[i], 1, (uwWidth >> 3) * uwHeight, pFile);
	fclose(pFile);
	
	logWrite("Dimensions: %ux%u@%uBPP, unused: %u %u\n",uwWidth,uwHeight,ubPlaneCount,uwCopperLength,uwCopperSize);
	logBlockEnd("bitmapCreateFromFile()");
	return pBitMap;
}

void bitmapDestroy(tBitMap *pBitMap) {
	UBYTE i;
	logBlockBegin("bitmapDestroy(pBitMap: %p)", pBitMap);
	if (pBitMap) {
		WaitBlit();
		if(bitmapIsInterleaved(pBitMap))
			pBitMap->Depth = 1;
		for (i = pBitMap->Depth; i--;)
			memFree(pBitMap->Planes[i], pBitMap->BytesPerRow*pBitMap->Rows);
		memFree(pBitMap, sizeof(tBitMap));
	}
	logBlockEnd("bitmapDestroy()");
}

inline BYTE bitmapIsInterleaved(tBitMap *pBitMap) {
	return (pBitMap->Depth > 1 && ((ULONG)pBitMap->Planes[1] - (ULONG)pBitMap->Planes[0])*pBitMap->Depth == pBitMap->BytesPerRow);
}

#ifdef GAME_DEBUG

void logBitMap(struct BitMap *pBitMap) {
	UBYTE i;
	logBlockBegin("logBitMap(pBitMap: %p)", pBitMap);
	logWrite("BytesPerRow: %u\n", pBitMap->BytesPerRow);
	logWrite("Rows: %u\n", pBitMap->Rows);
	logWrite("Flags: %hu\n", pBitMap->Flags);
	logWrite("Depth: %hu\n", pBitMap->Depth);
	logWrite("pad: %u\n", pBitMap->pad);
	/* since Planes is always 8-long, dump all its entries */
	for(i = 0; i != 8; ++i)
		logWrite("Planes[%hu]: %p\n", i, pBitMap->Planes[i]);
	logBlockEnd("logBitMap");
}

#endif /* GAME_DEBUG */