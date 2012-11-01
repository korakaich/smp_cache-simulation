
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

using namespace std;

Cache::Cache(int s,int a,int b )
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   
   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   writeMisses = writeBacks = currentCycle = 0;
   invalidToShared = invalidToModified = sharedToModified = modifiedToShared = flushes = interventions = invalidations =0;
   sharedToInvalid = modifiedToInvalid = modifiedToOwned= ownedToModified= exclusiveToModified= exclusiveToShared=0;
   //invalidToExclusive=0;
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
		invalidToShared++;
	}
        if (op == 'w') {
            newline->setFlags(DIRTY);
            //send a busrdx
            bus.busRdX(id, addr);
	    invalidToModified++;
        }
    } 
    else 
    {
        /**since it's a hit, update LRU and update dirty flag**/
	updateLRU(line);                  
	//1.if modified do nothing
	if(line->isModified()){}
        else if (line->isInvalid()) { //2. shared--get from memory and change state
            //line is present just update
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

void Cache::AccessMESI(ulong addr, uchar op, Bus bus) {
    currentCycle++; /*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
    if (op == 'w') writes++;
    else reads++;

    cacheLine * line = findLine(addr);
    if (line == NULL)/*miss*/ {
        if (op == 'w') writeMisses++;
        else readMisses++;
        cacheLine *newline = fillLine(addr);
        if (bus.isCached(id, addr))
            cacheToCache++; //SARKAR CHANGE
        //reading send a busRd
        if (op == 'r') {
            //if no other cache has the line, then change to E
            if (!bus.isCached(id, addr)) {
                bus.busRd(id, addr);
                newline->setFlags(EXCLUSIVE);
                invalidToExclusive++;
            } else {
                bus.busRd(id, addr);
                newline->setFlags(VALID);
                invalidToShared++;
                //cacheToCache++; //SARKAR CHANGE
            }
        }
        if (op == 'w') {
            newline->setFlags(DIRTY);
            bus.busRdX(id, addr); //send a busrdx
            invalidToModified++;
        }
    } else {
        /**since it's a hit, update LRU and update dirty flag**/
        updateLRU(line);
        //1.if modified do nothing
        if (line->isModified()) {
        } else if (line->isInvalid()) { //2. invalid--get from memory and change state
            if (bus.isCached(id, addr))
                cacheToCache++; //SARKAR CHANGE

            //line is present just update
            if (op == 'r') {
                //if line in no other cache has the line, then change to E
                if (!bus.isCached(id, addr)) {
                    bus.busRd(id, addr);
                    line->setFlags(EXCLUSIVE);
                    invalidToExclusive++;
                } else {
                    bus.busRd(id, addr);
                    line->setFlags(VALID);
                    invalidToShared++;
                    //cacheToCache++; //SARKAR CHANGE
                }
            } else if (op == 'w') {
                line->setFlags(DIRTY);
                bus.busRdX(id, addr); //send a busrdx
                invalidToModified++;
                //if (bus.isCached(id, addr))
                //cacheToCache++; //SARKAR CHANGE
            }
        } else if (line->isShared()) { //3. State is shared
            if (op == 'w') {
                sharedToModified++;
                bus.busUpgr(id, addr);
                line->makeModified();
            }
        } else if (line->isExclusive()) {
            if (op == 'w') {
                exclusiveToModified++;
                line->makeModified();
            }
        }
    }
}



void Cache::AccessMOESI(ulong addr, uchar op, Bus bus) {
    currentCycle++; /*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
    if (op == 'w') writes++;
    else reads++;

    cacheLine * line = findLine(addr);
    if (line == NULL)/*miss*/ {
        if (op == 'w') writeMisses++;
        else readMisses++;
        cacheLine *newline = fillLineMOESI(addr);
        if (bus.isCachedDirty(id, addr))
            cacheToCache++; //SARKAR CHANGE
        //reading send a busRd
        if (op == 'r') {
            //if no other cache has the line, then change to E
            if (!bus.isCachedOwner(id, addr)) {
                bus.busRd(id, addr);
                newline->setFlags(EXCLUSIVE);
                invalidToExclusive++;
            } else {
                bus.busRd(id, addr);
                newline->setFlags(VALID);
                invalidToShared++;
                //cacheToCache++; //SARKAR CHANGE
            }
        }
        if (op == 'w') {
            newline->setFlags(DIRTY);
            bus.busRdX(id, addr); //send a busrdx
            invalidToModified++;
        }
    } else {
        /**since it's a hit, update LRU and update dirty flag**/
        updateLRU(line);
        //1.if modified do nothing
        if (line->isModified()) {
        } else if (line->isInvalid()) { //2. invalid--get from memory and change state
            if (bus.isCachedOwner(id, addr))
                cacheToCache++; //SARKAR CHANGE
            //line is present just update
            if (op == 'r') {
                //if line in no other cache has the line, then change to E
                if (!bus.isCachedOwner(id, addr)) {
                    bus.busRd(id, addr);
                    line->setFlags(EXCLUSIVE);
                    invalidToExclusive++;
                } else {
                    bus.busRd(id, addr);
                    line->setFlags(VALID);
                    invalidToShared++;
                    //cacheToCache++; //SARKAR CHANGE
                }
            } else if (op == 'w') {
                line->setFlags(DIRTY);
                bus.busRdX(id, addr); //send a busrdx
                invalidToModified++;
                //if (bus.isCached(id, addr))
                //cacheToCache++; //SARKAR CHANGE
            }
        } else if (line->isShared()) { //3. State is shared
            if (op == 'w') {
                sharedToModified++;
                bus.busUpgr(id, addr);
                line->makeModified();
            }
        } else if (line->isExclusive()) {
            if (op == 'w') {
                exclusiveToModified++;
                line->makeModified();
            }
        } else if (line->isOwner()) {
            if (op == 'w') {
                ownedToModified++;
                bus.busUpgr(id, addr);
                line->makeModified();
            }
        }
    }
}


/****************************************** MESI **************************************/
void Cache::processMESIBusRd(ulong addr){
    cacheLine * line = findLine(addr);
    if (line != NULL){		
        if(line->isModified()){
            modifiedToShared++;
            line->makeShared();
            flushes++;
	    interventions++;//Korak
        }
	else if(line->isExclusive())
	{
            exclusiveToShared++;
       	    line->makeShared();
       	    //cacheToCache++;	
	    //interventions++;//Korak		
	}
	else if(line->isShared())
	{
		//cacheToCache++;
	}
   }
}

void Cache::processMESIBusRdX(ulong addr){
    cacheLine * line = findLine(addr);
    if(line != NULL){
        if(line->isShared()){
            line->makeInvalid();
            sharedToInvalid++;	
	    invalidations++;
	    //cacheToCache++;
        } 
	else if (line->isModified()) 
	{
            modifiedToInvalid++;
	    invalidations++;
            line->makeInvalid();
            flushes++;
        }
	else if (line->isExclusive()) 
	{
           //exclusiveToInvalid++;
	   invalidations++;
           line->makeInvalid();
           flushes++;
        }
    }
}



void Cache::processMESIBusUpgr(ulong addr)
{
    cacheLine * line = findLine(addr);
    if(line != NULL)
	{
		if(line->isShared())
		{
			line->makeInvalid();
			invalidations++;
		}	
	}
}

/****************************************** MSI **************************************/


void Cache::processMSIBusRd(ulong addr)
{
    cacheLine * line = findLine(addr);
    if (line != NULL){
        if(line->isModified()){
            modifiedToShared++;
            line->makeShared();
            flushes++;
	    interventions++;
        }
    }
}
void Cache::processMSIBusRdX(ulong addr){
    cacheLine * line = findLine(addr);
    if(line != NULL){
        if(line->isShared()){
            line->makeInvalid();
            sharedToInvalid++;	
	    invalidations++;
        } else if (line->isModified()) {
            modifiedToInvalid++;
	    invalidations++;
            line->makeInvalid();
            flushes++;
        }
    }
}



/****************************************** MOESI **************************************/


void Cache::processMOESIBusRd(ulong addr) {
    cacheLine * line = findLine(addr);
    if (line != NULL) {
        if (line->isModified()) {
            modifiedToOwned++;
            line->makeOwner();
            flushes++;
            interventions++;
        } else if (line->isExclusive()) {
            exclusiveToShared++;
            line->makeShared();
	    //interventions++;
            //cacheToCache++; 
        } else if (line->isShared()) {
            //cacheToCache++;
        } else if (line->isOwner()) {
            //do Nothing
	   flushes++;
        }
    }
}

void Cache::processMOESIBusRdX(ulong addr) {
    cacheLine * line = findLine(addr);
    if (line != NULL) {
        if (line->isShared()) {
            line->makeInvalid();
            sharedToInvalid++;
            invalidations++;
            //cacheToCache++;  SARKAR CHANGE
        } else if (line->isModified()) {
            modifiedToInvalid++;
            invalidations++;
            line->makeInvalid();
            flushes++;
        } else if (line->isExclusive()) {
            invalidations++;
            line->makeInvalid();
            //flushes++;
        } else if (line->isOwner()) {
            invalidations++;
            line->makeInvalid();
            flushes++;
        }
    }
}

void Cache::processMOESIBusUpgr(ulong addr) {
    cacheLine * line = findLine(addr);
    if (line != NULL) {
        if (line->isShared()) {
            line->makeInvalid();
            invalidations++;
        } else if(line->isOwner()){
            line->makeInvalid();
            invalidations++;
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

/*allocate a new line only for MOESI*/
cacheLine *Cache::fillLineMOESI(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if(victim->isModified()|| victim->isOwner() ) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(VALID);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

void Cache::printStats()
{ 

	/****print out the rest of statistics here.****/
	/****follow the ouput file format**************/
	printf("===== Simulation results (Cache_%d)      =====\n", id);
	printf("01. number of reads:				%ld\n", reads);
	printf("02. number of read misses:			%ld\n", readMisses);
	printf("03. number of writes:				%ld\n", writes);
	printf("04. number of write misses:			%ld\n", writeMisses);
	printf("05. number of write backs:			%ld\n", writeBacks);
	printf("06. number of invalid to exclusive (INV->EXC):	%d\n", invalidToExclusive);
	printf("07. number of invalid to shared (INV->SHD):	%d\n", invalidToShared);
	printf("08. number of modified to shared (MOD->SHD):	%d\n", modifiedToShared);
	printf("09. number of exclusive to shared (EXC->SHD):	%d\n", exclusiveToShared);
	printf("10. number of shared to modified (SHD->MOD):	%d\n", sharedToModified);
	printf("11. number of invalid to modified (INV->MOD):	%d\n", invalidToModified);
	printf("12. number of exclusive to modified (EXC->MOD):	%d\n", exclusiveToModified);
	printf("13. number of owned to modified (OWN->MOD):	%d\n", ownedToModified);
	printf("14. number of modified to owned (MOD->OWN):	%d\n", modifiedToOwned);
	printf("15. number of cache to cache transfers:		%d\n", cacheToCache);
	printf("16. number of interventions:			%d\n", interventions);
	printf("17. number of invalidations:			%d\n", invalidations);
	printf("18. number of flushes:				%d\n", flushes);
}
