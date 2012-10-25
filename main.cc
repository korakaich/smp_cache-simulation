/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"

int main(int argc, char *argv[])
{
	
	ifstream fin;
	FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	//int cache_size = atoi(argv[1]);
	//int cache_assoc= atoi(argv[2]);
	//int blk_size   = atoi(argv[3]);
	//int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	//int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:MOESI*/
	char *fname =  (char *)malloc(20);
 	fname = argv[1];
	Cache *cacheArray[4];
	for(int i =0;i<4;i++)
	{
		cacheArray[i]=new Cache(512 , 4, 2);
	}
	//****************************************************//
	//**printf("===== Simulator configuration =====\n");**//
	//*******print out simulator configuration here*******//
	//****************************************************//

 
	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	

	pFile = fopen (fname,"r");
	if(pFile == 0)
	{   
		printf("Trace file problem\n");
		exit(0);
	}
	///******************************************************************//
	//**read trace file,line by line,each(processor#,operation,address)**//
	//*****propagate each request down through memory hierarchy**********//
	//*****by calling cachesArray[processor#]->Access(...)***************//
	char sLine [100];
        uint procNum;
        uchar op;
        ulong addr;
        int i,j;
        char tempAddr[10];
	int total_reads=0, total_writes=0;
        while (fgets(sLine,sizeof sLine,pFile)!=NULL)
        {
                i=0;
                j=0;
                while(sLine[i]!='\0')
                {
                        if(i==0)
                        {
                                procNum=sLine[i]-'0';
                                i+=2;
                                op=sLine[i];
                                i+=2;
                        }
                        else
                        {
                                tempAddr[j++]=sLine[i++];
                        }
                }
                tempAddr[j]='\0';
                //?? convert tempAddr to ulong ??//
                addr=strtoul ( tempAddr, NULL,16 );
                cacheArray[procNum]->Access(addr, op);
		if(op=='r')
			total_reads++;
		else
			total_writes++;
        }
	cout<<"reads: "<<total_reads<<" writes: "<<total_writes;
	cout<<endl;
	for(i=0;i<4;i++)
		cacheArray[i]->printStats();
	///******************************************************************//
	fclose(pFile);

	//********************************//
	//print out all caches' statistics //
	//********************************//
	
}
