#include "cache.h"

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
