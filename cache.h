/*******************************************************
                          cache.h
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
 ********************************************************/

#ifndef CACHE_H
#define CACHE_H

#include <cmath>
#include <iostream>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef unsigned int uint;

/****add new states, based on the protocol****/
enum {
    INVALID = 0, //Invalid
    VALID, //Shared
    DIRTY, //Modified
};

class Cache;
class Bus {
    Cache **caches;
    int num;
public:
    Bus(){
        
    }
    
    ~Bus();

    void setCaches(Cache **caches) {
        this->caches = caches;
    }

    void setNum(int num) {
        this->num = num;
    }

    void busRd(int, unsigned long);
    void busRdx(int, unsigned long);

    void flush(int id, unsigned long address) {
        //DoNothing
    }

private:

};

class cacheLine {
protected:
    ulong tag;
    ulong Flags; // 0:invalid, 1:valid, 2:dirty 
    ulong seq;

public:

    cacheLine() {
        tag = 0;
        Flags = 0;
    }

    ulong getTag() {
        return tag;
    }

    ulong getFlags() {
        return Flags;
    }

    ulong getSeq() {
        return seq;
    }

    void setSeq(ulong Seq) {
        seq = Seq;
    }

    void setFlags(ulong flags) {
        Flags = flags;
    }

    void setTag(ulong a) {
        tag = a;
    }

    void invalidate() {
        tag = 0;
        Flags = INVALID;
    }//useful function

    void makeInvalid() {
        Flags = INVALID;
    }

    void makeModified() {
        Flags = DIRTY;
    }

    void makeShared() {
        Flags = VALID;
    }

    bool isInvalid() {
        return ((Flags) == INVALID);
    }

    bool isModified() {
        return ((Flags) == DIRTY);
    }

    bool isShared() {
        return ((Flags) == VALID);
    }

    bool isValid() {
        return ((Flags) != INVALID);
    }
};

class Cache {
protected:
    ulong size, lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
    ulong reads, readMisses, writes, writeMisses, writeBacks;
    ulong invalidToShared, invalidToModified, sharedToModified, modifiedToShared, flushes;
    ulong sharedToInvalid, modifiedToInvalid, invalidations;
    int id;

    //******///
    //add coherence counters here///
    //******///

    cacheLine **cache;

    ulong calcTag(ulong addr) {
        return (addr >> (log2Blk));
    }

    ulong calcIndex(ulong addr) {
        return ((addr >> log2Blk) & tagMask);
    }

    ulong calcAddr4Tag(ulong tag) {
        return (tag << (log2Blk));
    }

public:
    ulong currentCycle;

    Cache(int, int, int, int);

    ~Cache() {
        delete cache;
    }

    cacheLine *findLineToReplace(ulong addr);
    cacheLine *fillLine(ulong addr);
    cacheLine *findLine(ulong addr);
    cacheLine *getLRU(ulong);

    ulong getRM() {
        return readMisses;
    }

    ulong getWM() {
        return writeMisses;
    }

    ulong getReads() {
        return reads;
    }

    ulong getWrites() {
        return writes;
    }

    ulong getWB() {
        return writeBacks;
    }
    
    int getId() {
        return id;
    }

    void writeBack(ulong) {
        writeBacks++;
    }
    void Access(ulong, uchar);
    void AccessMSI(ulong, uchar, Bus *bus);
    void printStats();
    void updateLRU(cacheLine *);

    //******///
    //add other functions to handle bus transactions///
    //******///
    
    void processMSIBusRd(ulong);
    void processMSIBusRdx(ulong);

};
#endif
