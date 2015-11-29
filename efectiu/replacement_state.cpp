#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <map>
#include <iostream>
#include <vector>

using namespace std;

#include "replacement_state.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

// determine whether the incoming block is being passby or not

bool CACHE_REPLACEMENT_STATE::IsPassby(Addr_t PC)
{
    if (dead == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;
    out<<"=========== Sampler Replacement Policy Statistics ========="<<endl;

    // CONTESTANTS:  Insert your statistics printing here
    
    return out;

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// LLC's extra bit information for replacement???
void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways

    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    replSampler = new LINE_REPLACEMENT_STATE* [SAMPLER_SIZE];

    // ensure that we were able to create replacement state

    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex = 0; setIndex < numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way = 0; way < assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

for(UINT32 setSamplerIndex = 0; setSamplerIndex < SAMPLER_SIZE; setSamplerIndex++) 
    {
        replSampler[ setSamplerIndex ]  = new LINE_REPLACEMENT_STATE[ SAMPLER_ASSOC ];

        for(UINT32 way = 0; way < SAMPLER_ASSOC; way++) 
        {
            // initialize stack position (for true LRU)
            replSampler[ setSamplerIndex ][ way ].LRUstackposition = way;
        }
    }


    if (replPolicy != CRC_REPL_CONTESTANT) return;

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE

    // My predictor function, use to predict the dead block information, one extra bit
}



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// argument is the set index. The return value is the physical way            //
// index for the line being replaced.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType ) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE

	return Get_My_Victim (setIndex);
    }

    // We should never get here  

    assert(0);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
	//fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
	//fflush (stderr);
     // updateWayId is the updated way-block, shows which block is going to be replaced???
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
        // should add sampler, if it was in sampler, then do update???
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
	// Get pointer to replacement state of current set

	LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
	INT32   lruWay   = 0;

	// Search for victim whose stack position is assoc-1

	for(UINT32 way=0; way<assoc; way++) {
		if (replSet[way].LRUstackposition == (assoc-1)) {
			lruWay = way;
			break;
		}
	}

	// return lru way

	return lruWay;
}

// Get dead-block prediction victim in LLC

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim( UINT32 setIndex ) 
{
	// return first way always

    LINE_REPLACEMENT_STATE *replSet = repl[setIndex];
    INT32 myWay = -1;
    for (UINT32 way=0; way<assoc; way++) {
        if (replSet[way].dead == true) {
            myWay = way;
            break;
        }
    }
	// if here does not have dead block, we need to call Get_LRU_Victim?????
    return myWay;

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
	// Determine current LRU stack position
	UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

	// Update the stack position of all lines before the current line
	// Update implies incremeting their stack positions by one

	for(UINT32 way=0; way<assoc; way++) {
		if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
			repl[setIndex][way].LRUstackposition++;
		}
	}

	// Set the LRU stack position of new line to be zero
	repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}



void CACHE_REPLACEMENT_STATE::UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID ) {
	// do nothing
}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// SamplerCache functions                                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



// Initialize a samplercache

void InitSamplerCache(samplerCache *s, UINT32 samplerNumsets, UINT32 samplerAssoc, UINT32 replacement_policy)
{
    UINT32 i, j;
    s->samplerSets = new samplerSet[SAMPLER_SIZE];
    s->samplerNumsets = SAMPLER_SIZE;
    s->samplerAssoc = SAMPLER_ASSOC;
    s->replSampler = new CACHE_REPLACEMENT_STATE (samplerNumsets, samplerAssoc, replacement_policy);

    for (i=0; i<samplerNumsets; i++)
    {
        for (j=0; j<samplerAssoc; j++)
        {
            samplerBlock *sb = &s->samplerSets[i].samplerBlocks[j];
            sb->tag = 0;
            sb->PC = 0;
            sb->valid = false;
            sb->dead = false;
            sb->LruStackPosition = j;
        }
        s->samplerSets[i].valid = 0;
    }
}



// decide whether this set is being sampled or not

INT32 samplerCache::GetSamplerSetIndex(UINT32 setIndex)
{
    if ( (setIndex+1) % 64 == 0 )
    {
        samplerSetIndex = setIndex;
    }
    else
    {
        samplerSetIndex = -1;
    }
    return samplerSetIndex;
}

bool samplerCache::IsSamplerHit(UINT32 setIndex, UINT32 samplerSetIndex)
{
    bool isSampled = false;
    if (samplerSetIndex >= 0)
    {
        isSampled = true;
    }
    else
    {
        return false;
    }
    return isSampled;
}

// Get Sampler LRU victim

INT32 samplerCache::GetLruVictim(UINT32 samplerSetIndex)
{
    LINE_REPLACEMENT_STATE *replSamplerSet = replSampler[samplerSetIndex];
    // samplerBlock *samplerBlocks = samplerSets[samplerSetIndex];
    INT32 lruSamplerWay = 0;
    // Search for victim whose stack position is samplerAssoc-1

    for (UINT32 i=0; i<SAMPLER_ASSOC; i++)
    {
        if (replSamplerSet[i].LRUstackposition == (SAMPLER_ASSOC-1))
        {
            lruSamplerWay = i;
            break;
        }
    }
    return lruSamplerWay;
}

// Get Sampler Random victim

INT32 samplerCache::GetRandomVictim( UINT32 samplerSetIndex )
{
    INT32 way = (rand() % SAMPLER_ASSOC);
    
    return way;
}

// Get dead-block prediction victim in Sampler cache

INT32 samplerCache::GetMyVictim( UINT32 samplerSetIndex ) 
{
	// return first way always

    LINE_REPLACEMENT_STATE *replSamplerSet = replSampler[samplerSetIndex];
    INT32 mySamplerWay = -1;
    for (UINT32 way=0; way<SAMPLER_ASSOC; way++) {
        if (replSamplerSet[way].dead == true) {
            mySamplerWay = way;
            break;
        }
    }
	// if here does not have dead block, we need to call GetLruVictim?????
    return mySamplerWay;

}

// Get the victim block in sampler cache


INT32 samplerCache::GetVictimInSamplerSet(UINT32 samplerSetIndex, const samplerBlock *vicSamplerSet,INT32 victimBlock, UINT32 samplerAssoc, Addr_t PC, Addr_t paddr, UINT32 accessType, bool dead)
{

    Addr_t tag;
    INT32 victimBLock = -1;

    for (int i=0; i<SAMPLER_ASSOC; i++)
    {
        if (vicSamplerSet[i].tag == (tag & ((1 << TAG_LENGTH)-1)))
        {
            return 0;
        }
    }
    if (vicSamplerSet->valid == false)
    {
        int way;
        for (way = 0; way<SAMPLER_ASSOC; way++)
        {
            if (vicSamplerSet[way].valid == 0)
            {
                break;
            }
        }
        victimBlock = way;
        return victimBlock;
    }
    else
    {
             // If no invalid lines, then replace based on replacement policy
        if (replPolicy == CRC_REPL_LRU)
        {
            return GetLruVictim(samplerSetIndex);
        }
        else if (replPolicy == CRC_REPL_RANDOM)
        {
            return GetRandomVictim(samplerSetIndex);
        }
        else if (replPolicy == CRC_REPL_CONTESTANT)
        {
            // Add victim selection function...............

            return GetMyVictim(samplerSetIndex);
        }
    }

    assert(0);
    return -1;
}


// Update sampler LRU replacement

void samplerCache::UpdateSamplerLRU( UINT32 samplerSetIndex, INT32 updateWayID )
{
	// Determine current LRU stack position
	UINT32 currLRUstackposition = replSampler[ samplerSetIndex ][ updateWayID ].LRUstackposition;

	// Update the stack position of all lines before the current line
	// Update implies incremeting their stack positions by one

	for(UINT32 way=0; way<SAMPLER_ASSOC; way++) {
		if( replSampler[ samplerSetIndex ][way].LRUstackposition < currLRUstackposition ) {
			replSampler[ samplerSetIndex ][way].LRUstackposition++;
		}
	}

	// Set the LRU stack position of new line to be zero
	replSampler[ samplerSetIndex ][ updateWayID ].LRUstackposition = 0;
}



// Update my sampler dead-block prediction replacement

void samplerCache::UpdateMyPolicy(UINT32 setIndex, UINT32 samplerSetIndex, INT32 updateWayID )
{ 
    Addr_t PC = 0;
    GetSamplerSetIndex(setIndex);
    int way;
    for (way=0; way<SAMPLER_ASSOC; way++)
    {
        if (samplerSets[samplerSetIndex][way].dead == true)
        {
            PC = samplerSets[samplerSetIndex][way].PC;
        }
    }

      	
}



// Update sampler replacement

void samplerCache::UpdateSamplerReplacementState(UINT32 samplerSetIndex, INT32 updateWayID, UINT32 setIndex, const samplerBlock *currBlock, Addr_t PC, UINT32 accessType, bool cacheHit, bool dead)
{

if (replPolicy == CRC_REPL_LRU)
{
    UpdateSamplerLRU(samplerSetIndex, updateWayID);
}
else if (replPolicy == CRC_REPL_RANDOM)
{
    // Do nothing
}
else if (replPolicy == CRC_REPL_CONTESTANT)
{
    UpdateMyPolicy(setIndex, samplerSetIndex, updateWayID);
}


}



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is doing prediction job, get the 1-extra bit dead            //
// information                                                                //
// block.signature = PC, with 64 bit                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void myPredictor::InitMyPredictor()
{
    table = new int* [PREDICTOR_SIZE];

    assert(table);

    for (int predictorEntry=0; predictorEntry<PREDICTOR_SIZE; predictorEntry++)
    {
        table[predictorEntry] = new int[PREDICTOR_NUM];
        for (int i=0; i<PREDICTOR_NUM; i++)
        {
            table[predictorEntry][i] = 0;
        }
    }
}


bool myPredictor::IsDead(Addr_t PC)
{
    bool dead = false;
    predictorResult(PC);
   
    if (counterSum >= THRESHOLD)
    {
        dead = true;
    }
    else
    {
        dead = false; 
    }
    return dead;
}


INT32 myPredictor::predictorResult(Addr_t PC)
{
    int counterSum = 0;
    PC &= ((1<<PC_LENGTH)-1);
    int hash[3];
    hash[0] = 4096;
    hash[1] = 4096;
    hash[2] = 4096;

    for (int i=0; i<PREDICTOR_NUM; i++)
    {
        predictorEntry = PC % hash[i];
        table[predictorEntry][i] ++;
        if (table[predictorEntry][i] > 3)
            table[predictorEntry][i] = 3;
        if (table[predictorEntry][i] < 0)
            table[predictorEntry][i] = 0;
        counterSum = counterSum + table[predictorEntry][i];
    }

    return counterSum;
}

