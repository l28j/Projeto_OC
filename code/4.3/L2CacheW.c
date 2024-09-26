#include "L2CacheW.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache cacheL1;
L2Cache cacheL2;


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

/*********************** Initializations *************************/
void initCacheL1(){
    for (int i = 0; i < L1_LINES; i++) {
        cacheL1.lines[i].Valid = 0;
        cacheL1.lines[i].Dirty = 0;
        cacheL1.lines[i].Tag = 0;
        memset(cacheL1.lines[i].Block, 0, BLOCK_SIZE);
    }
}

void initCacheL2()
{
    for (int i = 0; i < SETS_L2; i++) {
        for (int j = 0; j < WAYS; j++) {
            cacheL2.sets[i].lines[j].Valid = 0;
            cacheL2.sets[i].lines[j].Dirty = 0;
            cacheL2.sets[i].lines[j].Tag = 0;
            memset(cacheL2.sets[i].lines[j].Block, 0, BLOCK_SIZE);
        }
    }
}

void initDram(){ memset(DRAM, 0, DRAM_SIZE); }

void initCache(){
    initCacheL1();
    initCacheL2();
    initDram();
}

/*********************** L1 cache *************************/
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

    uint32_t index, Tag, MemAddress, blockOffset;
    blockOffset = getOffset(address);
    index = getLineIndex(address);
    Tag = getTag(address);  
    CacheLine *Line = &cacheL1.lines[index];

    // Check if block is in cache L1
    if (Line->Valid && Line->Tag == Tag){
        if (mode == MODE_WRITE) {
            Line->Dirty = 1;
            memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
            time += L1_WRITE_TIME;
        } 
        else {
            memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
            time += L1_READ_TIME;
        }
    }

    // If block is not in cache L1
    else {
        // If block is dirty, save it in cache L2
        if (Line->Dirty){
            MemAddress = address - blockOffset;
            accessL2(MemAddress, Line->Block, MODE_WRITE);
        }

        // Ask cache L2 to get the block
        MemAddress = address - blockOffset;
        accessL2(MemAddress, Line->Block, MODE_READ);

        Line->Valid = 1;
        Line->Tag = Tag;
        // Write or Read on the block requested
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
}

/*********************** L2 cache *************************/
void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {
    
    uint32_t index, Tag, MemAddress, blockOffset;
    blockOffset = getOffset(address);
    index = getLineIndex(address);
    Tag = getTag(address);
    MemAddress = address - blockOffset;


    // Check if block is in cache
    for (int i = 0; i < WAYS; i++) {
        CacheLine *Line = &cacheL2.sets[index].lines[i];

        if(Line->Valid && Line->Tag == Tag) {
            // Write
            if (mode == MODE_WRITE) {
                memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
                Line->Dirty = 1;
                time += L2_WRITE_TIME;
            }
            // Read            
            else{
                memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
                time += L2_READ_TIME;
            }
            Line->Time = getTime();
            return;
        } 
    }

    // Block is not in cache

    // Search for least recently used (LRU)
    uint32_t oldest = cacheL2.sets[index].lines[0].Time;
    uint32_t lru = 0;
    for(int i = 0; i < WAYS; i++) {
        uint32_t block_time = cacheL2.sets[index].lines[i].Time;
        if(block_time > oldest) {
            oldest = cacheL2.sets[index].lines[i].Time;
            lru = i;
        }
    }

    // Evicting LRU block
    CacheLine *Line = &cacheL2.sets[index].lines[lru];
    // Save if LRU was dirty
    if(Line->Dirty) {
        accessDRAM(MemAddress, Line->Block, MODE_WRITE);
    }
    accessDRAM(MemAddress, Line->Block, MODE_READ);

    // Insert the new block
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Time = getTime();
    if(mode == MODE_READ) {
        memcpy(data, &Line->Block[blockOffset], WORD_SIZE);
        Line->Dirty = 0;
        time += L2_READ_TIME;
    }
    else{
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