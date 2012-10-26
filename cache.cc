/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include "bus.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
   invalidToShared = invalidToModified = sharedToModified = modifiedToShared = flushes = 0;
   sharedToInvalid = modifiedToInvalid = 0;
   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w') writeMisses++;
		else readMisses++;

		cacheLine *newline = fillLine(addr);
   		if(op == 'w') newline->setFlags(DIRTY);    
		
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w') line->setFlags(DIRTY);
	}
}

void Cache::AccessMSI(ulong addr, uchar op, Bus bus) {
    currentCycle++; /*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
    if (op == 'w') writes++;
    else reads++;

    cacheLine * line = findLine(addr);
    if (line == NULL)/*miss*/ {
        if (op == 'w') writeMisses++;
        else readMisses++;

        cacheLine *newline = fillLine(addr);
        //reading send a busRd
		if(op =='r')
		{
        	bus.busRd(id, addr);
			newline->setFlags(VALID);
			//? invalidToShared++;
		}
        if (op == 'w') {
            newline->setFlags(DIRTY);
            //send a busrdx
            bus.busRdX(id, addr);
			//?invalidToModified++;
        }
    } else {
        /**since it's a hit, update LRU and update dirty flag**/
	  updateLRU(line);                  
	  //if modified do nothing
      if (!line->isValid()) { //2. get from memory and change state
            //line is present just update
            //updateLRU(line);
            if (op == 'r') {
                invalidToShared++;
                bus.busRd(id, addr);
                line->makeShared();
            } else {
                invalidToModified++;
                bus.busRdX(id, addr);
                line->makeModified();
            }
        } else { //3. State is shared
            //updateLRU(line);
            if (op == 'w') {
                sharedToModified++;
                bus.busRdX(id, addr);
                line->makeModified();
            }
        }
    }
}

void Cache::processMSIBusRd(ulong addr){
    cacheLine * line = findLine(addr);
    if (line != NULL){
        if(line->isModified()){
            modifiedToShared++;
            line->makeShared();
            flushes++;
        }
    }
}
void Cache::processMSIBusRdX(ulong addr){
    cacheLine * line = findLine(addr);
    if(line != NULL){
        if(line->isShared()){
            line->makeInvalid();
            sharedToInvalid++;	
        } else if (line->isModified()) {
            modifiedToInvalid++;
            line->makeInvalid();
            flushes++;
        }
    }
}


/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->getFlags() == DIRTY) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats()
{ 
	printf("===== Simulation results      =====\n");
	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
	printf("===== Simulation results (Cache_0)      =====\n");
	printf("01. number of reads:				%ld\n", reads);
	printf("02. number of read misses:			%ld\n", readMisses);
	printf("03. number of writes:				%ld\n", writes);
	printf("04. number of write misses:			%ld\n", writeMisses);
	printf("05. number of write backs:			%ld\n", writeBacks);
}
