#include "L2Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1Cache cacheL1;
L2Cache cacheL2;


/**************** Adress Manipulation ***************/
// cache L2
uint32_t getOffsetL2(uint32_t address) { return address & 0x3F;}
uint32_t getLineIndexL2(uint32_t address) { return (address >> 6) & 0x1FF; }
uint32_t getTagL2(uint32_t address) { return (address >> 15); }

// cache L1
uint32_t getOffsetL1(uint32_t address) { return address & 0x3F; }
uint32_t getLineIndexL1(uint32_t address) { return (address >> 6) & 0xFF;}
uint32_t getTagL1(uint32_t address) { return (address >> 14);}

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode)
{

    if (address >= DRAM_SIZE - WORD_SIZE + 1)
        exit(-1);

    if (mode == MODE_READ)
    {
        memcpy(data, &(DRAM[address]), BLOCK_SIZE);
        time += DRAM_READ_TIME;
    }

    if (mode == MODE_WRITE)
    {
        memcpy(&(DRAM[address]), data, BLOCK_SIZE);
        time += DRAM_WRITE_TIME;
    }
}

/*********************** Initializations *************************/
void initCacheL1()
{
    for (int i = 0; i < L1_LINES; i++) {
        cacheL1.lines[i].Valid = 0;
        cacheL1.lines[i].Dirty = 0;
        cacheL1.lines[i].Tag = 0;
        memset(cacheL1.lines[i].Block, 0, BLOCK_SIZE);
    }
}

void initCacheL2()
{
    for (int i = 0; i < L2_LINES; i++) {
        cacheL2.lines[i].Valid = 0;
        cacheL2.lines[i].Dirty = 0;
        cacheL2.lines[i].Tag = 0;
        memset(cacheL2.lines[i].Block, 0, BLOCK_SIZE);
    }
}

void initDram(){ memset(DRAM, 0, DRAM_SIZE); }

void initCache()
{
    initCacheL1();
    initCacheL2();
    initDram();
}

/*********************** L1 cache *************************/
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

    uint32_t index, Tag, MemAddress, blockOffset;
    blockOffset = getOffsetL1(address);
    index = getLineIndexL1(address);
    Tag = getTagL1(address);  
    CacheLine *Line = &cacheL2.lines[index];

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
    blockOffset = getOffsetL2(address);
    index = getLineIndexL2(address);
    Tag = getTagL2(address);  
    MemAddress = address - blockOffset;
    CacheLine *Line = &cacheL2.lines[index];

    // Check if block is in cache L2
    if (Line ->Valid && Line->Tag == Tag){
        if (mode == MODE_WRITE) {
            Line->Dirty = 1;
            memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
            time += L2_WRITE_TIME;
        } else {
            memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
            time += L2_READ_TIME;
        }
    }

    // If block is not in cache L2
    else {
        // If block is dirty, save it in memory
        if (Line->Dirty){
            accessDRAM(MemAddress, Line->Block, MODE_WRITE);
        }
        accessDRAM(MemAddress, Line->Block, MODE_READ);
        Line->Valid = 1;
        Line->Tag = Tag;

        // Write or Read on the block requested
        if (mode == MODE_WRITE) {
            Line->Dirty = 1;
            memcpy(&(Line->Block[blockOffset]), data, WORD_SIZE);
            time += L2_WRITE_TIME;
        } else {
            Line->Dirty = 0;
            memcpy(data, &(Line->Block[blockOffset]), WORD_SIZE);
            time += L2_READ_TIME;
        }
    }
}
    
void read(uint32_t address, uint8_t *data)
{
    accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data)
{
    accessL1(address, data, MODE_WRITE);
}