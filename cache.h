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
enum{
	INVALID = 0,
	VALID,
	DIRTY
};




class Bus;//forward declaration
class cacheLine 
{
protected:
   ulong tag;
   ulong Flags;   // 0:invalid, 1:valid, 2:dirty 
   ulong seq; 
 
public:
   cacheLine()            { tag = 0; Flags = 0; }
   ulong getTag()         { return tag; }
   ulong getFlags()			{ return Flags;}
   ulong getSeq()         { return seq; }
   void setSeq(ulong Seq)			{ seq = Seq;}
   void setFlags(ulong flags)			{  Flags = flags;}
   void setTag(ulong a)   { tag = a; }
   void invalidate()      { tag = 0; Flags = INVALID; }//useful function
   bool isValid()         { return ((Flags) != INVALID); }
   void makeInvalid() 	  { Flags = INVALID;  }
   void makeModified()   { Flags = DIRTY; }
   void makeShared() 	  { Flags = VALID; }
   bool isModified() 	  { return ((Flags) == DIRTY); }
   bool isShared() 	  { return ((Flags) == VALID); }
   bool isInvalid()	  { return ((Flags) == INVALID); }
};

class Cache
{
	
protected:
   ulong  size,lineSize, assoc, sets, log2Sets, log2Blk, tagMask, numLines;
   ulong reads,readMisses,writes,writeMisses,writeBacks;
   
   //******///
   //add coherence counters here///
   ulong invalidToShared, invalidToModified, sharedToModified, modifiedToShared, exclusiveToModified, flushes, invalidations;
   //ulong invalidToExclusive;
   ulong sharedToInvalid, modifiedToInvalid, exclusiveToShared, ownedToModified, modifiedToOwned, cacheToCache, interventions;
   //******///

   cacheLine **cache;
   ulong calcTag(ulong addr)     { return (addr >> (log2Blk) );}
   ulong calcIndex(ulong addr)  { return ((addr >> log2Blk) & tagMask);}
   ulong calcAddr4Tag(ulong tag)   { return (tag << (log2Blk));}
   
public:
    uint id;//id of the cache
    ulong currentCycle;  
    Cache(int,int,int);
   ~Cache() { delete cache;}
   
   cacheLine *findLineToReplace(ulong addr);
   cacheLine *fillLine(ulong addr);
   cacheLine * findLine(ulong addr);
   cacheLine * getLRU(ulong);
   
   ulong getRM(){return readMisses;} 
   ulong getWM(){return writeMisses;} 
   ulong getReads(){return reads;}
   ulong getWrites(){return writes;}
   ulong getWB(){return writeBacks;}
   uint getId(){return id;}
   void setId(uint id){this->id=id;}
   
   void writeBack(ulong)   {writeBacks++;}
   void Access(ulong,uchar);
   void printStats();
   void updateLRU(cacheLine *);
   void AccessMSI(ulong, uchar, Bus);
   void processMSIBusRd(ulong);
   void processMSIBusRdX(ulong);
   //******///
   //add other functions to handle bus transactions///
   //******///

};

class Bus {
private:
    Cache **caches;
    int numOfCaches;
public:
    Bus();
    ~Bus();
    
    void setCaches(Cache**, int num);
    
    void busRd(int, ulong);
    
    void busRdX(int, ulong);
	
    void busUpgr(int , ulong);
    
    void flush(int , ulong);

    void printBusCaches();
};
#endif
