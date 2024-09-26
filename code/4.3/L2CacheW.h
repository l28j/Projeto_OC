#ifndef SIMPLECACHE_H
#define SIMPLECACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

void resetTime();

uint32_t getTime();

/************************ Utils ************************/
unsigned int getBlockOffset(uint32_t);
unsigned int getLineIndex(uint32_t);
unsigned int getTag(uint32_t);

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);
void initDRAM();

/*********************** Cache Line *************************/
typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint8_t Block[BLOCK_SIZE];
  uint32_t Time; 
} CacheLine;

/*********************** L1Cache *************************/


typedef struct L1Cache {
  CacheLine lines[L1_LINES];
} L1Cache;

void initL1();

void accessL1(uint32_t, uint8_t *, uint32_t);

/*********************** L2Cache *************************/

// SETS_L2 = Number of Blocks / WAYS = 256
#define WAYS 2
#define SETS_L2 256

typedef struct Sets {
  CacheLine lines[WAYS];
} Sets;

typedef struct L2Cache {
  Sets sets[SETS_L2];
} L2Cache;

void accessL2(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif