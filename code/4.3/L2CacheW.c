#include "L2CacheW.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache CashL1;
L2Cache CashL2;


/**************** Utils ***************/

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

void initCacheL1(){
    for (int i = 0; i < L1_LINES; i++) {
        CashL1.line[i].Valid = 0;
        CashL1.line[i].Dirty = 0;
        CashL1.line[i].Tag = 0;
        memset(CashL1.line[i].Block, 0, BLOCK_SIZE);
    }
}

void initCacheL2()
{
    for (int i = 0; i < L2_SETS; i++) {
        for (int j = 0; j < WAYS; j++) {
            CashL2.sets[i].line[j].Valid = 0;
            CashL2.sets[i].line[j].Dirty = 0;
            CashL2.sets[i].line[j].Tag = 0;
            memset(CashL2.sets[i].line[j].Block, 0, BLOCK_SIZE);
        }
    }
}

void initDram(){ memset(DRAM, 0, DRAM_SIZE); }

void initCache(){
    initCacheL1();
    initCacheL2();
    initDram();
}
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

    uint32_t index, Tag, MemAddress, blockOffset;
    
    blockOffset = getOffset(address);
    index = getLineIndex(address);
    Tag = getTag(address);  

    CacheLine *Line = &CashL1.line[index];

    /*MISS*/
    if(!Line->Valid || Line->Tag != Tag) {
        
        if (Line->Dirty){
            MemAddress = address - blockOffset;
            accessL2(MemAddress, Line->Block, MODE_WRITE);
        }

        MemAddress = address - blockOffset;
        accessL2(MemAddress, Line->Block, MODE_READ);

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

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {
    uint32_t index, Tag, MemAddress, blockOffset;

    blockOffset = getOffset(address);
    index = getLineIndex(address);
    Tag = getTag(address);
    MemAddress = address - blockOffset;

    for (int i = 0; i < WAYS; i++) {
        CacheLine *Line = &CashL2.sets[index].line[i];

        /* HIT */
        if(Line->Valid && Line->Tag == Tag) {
            if (mode == MODE_WRITE) {
                memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
            if (mode == MODE_READ) {
                memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
                time += L2_READ_TIME;
            }
            Line->Dirty = 1;
            time += L2_WRITE_TIME;
            }
            Line->Time = getTime();
            return;
        } 
    }

    /* MISS */

    uint32_t oldestTime = CashL2.sets[index].line[0].Time;
    uint32_t oldestIndex = 0;

    for(int i = 0; i < WAYS; i++) {
        uint32_t current_time = CashL2.sets[index].line[i].Time;

        if(current_time > oldestTime) {
        oldestTime = CashL2.sets[index].line[i].Time;
        oldestIndex = i;
        }
    }

    CacheLine *Line = &CashL2.sets[index].line[oldestIndex];

    if(Line->Dirty) {
        accessDRAM(MemAddress, Line->Block, MODE_WRITE);
    }

    accessDRAM(MemAddress, Line->Block, MODE_READ);

    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Time = getTime();

    if(mode == MODE_READ) {
        memcpy(data, &Line->Block[blockOffset], WORD_SIZE);
        Line->Dirty = 0;
        time += L2_READ_TIME;
    }

    if(mode == MODE_WRITE) {
        memcpy(&Line->Block[blockOffset], data, WORD_SIZE);
        Line->Dirty = 1;
        time += L2_WRITE_TIME;
    }

}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}