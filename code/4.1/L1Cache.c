#include "L1Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache SimpleCache;



/**************** Initialization ***************/

void initDRAM() { memset(DRAM, 0, DRAM_SIZE); }

void initL1() { 
    SimpleCache.init = 1;
    for (int i = 0; i < L1_LINES; i++) {
        SimpleCache.lines[i].Valid = 0;
        SimpleCache.lines[i].Dirty = 0;
        SimpleCache.lines[i].Tag = 0;
        memset(SimpleCache.lines[i].Block, 0, BLOCK_SIZE);
    }
}

void initCache() { 
  initDRAM();
  initL1();
}

/**************** Adress Manipulation ***************/

uint32_t getOffset(uint32_t address) { return address & 0x3F; }
uint32_t getLineIndex(uint32_t address) { return (address >> 6) & 0xFF;}
uint32_t getTag(uint32_t address) { return (address >> 14);}

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/


void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {
    uint32_t index, Tag, MemAddress, blockOffset;
    
    blockOffset = getOffset(address);
    index = getLineIndex(address);
    Tag = getTag(address);  
    MemAddress = address - blockOffset;

    CacheLine *Line = &SimpleCache.lines[index];

    /*MISS*/
    if(!Line->Valid || Line->Tag != Tag) {
        
        if (Line->Dirty){
            accessDRAM(MemAddress, Line->Block, MODE_WRITE);
        }

        accessDRAM(MemAddress, Line->Block, MODE_READ);

        Line->Valid = 1;
        Line->Tag = Tag;

        if (mode == MODE_WRITE) {
            Line->Dirty = 1;
            memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
            time += L1_WRITE_TIME;
        } else {
            Line->Dirty = 0;
            memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
            time += L1_READ_TIME;
        }
    }
    /*HIT*/
    else {
        if (mode == MODE_WRITE) {
            Line->Dirty = 1;
            memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
            time += L1_WRITE_TIME;
        } else {
            memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
            time += L1_READ_TIME;
        }
    } 
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
