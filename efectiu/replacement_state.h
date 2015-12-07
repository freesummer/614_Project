#ifndef REPL_STATE_H
#define REPL_STATE_H

//  Predictor defination
#define PREDICTOR_SIZE 4096   // number of entries
#define COUNTER_BIT 2        // number of bits of counter
#define THRESHOLD 8          // the boundary for dead
#define PREDICTOR_NUM 3

//  Sampler defination
#define SAMPLER_SIZE 32
#define TAG_LENGTH 15
#define PC_LENGTH 15
#define LRU_BIT 4
#define SAMPLER_ASSOC 12

 
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

#include <cstdlib>
#include <cassert>
#include "utils.h"
#include "crc_cache_defs.h"
#include <iostream>
#include <vector>

using namespace std;

// Replacement Policies Supported

typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line

typedef struct
{
    UINT32  LRUstackposition;
    bool dead; // an extra bit per cache line

    // CONTESTANTS: Add extra state per cache line here

} LINE_REPLACEMENT_STATE;

// Sampler block state

struct samplerBlock
{
    UINT32 LruStackPosition;
    bool valid;
    bool dead;
    Addr_t PC;
    Addr_t tag;

    samplerBlock (void)
        {
            valid = false;
            dead = false;
            tag = 0;
            PC = 0;
        }
};

// SamplerSet state

struct samplerSet
{
    // Sampler set has multiple samplerBlock and 1-bit extra metadata--set valid bit

    samplerBlock samplerBlocks[SAMPLER_ASSOC];
    unsigned char valid;  // means entire samplerSet is valid

    samplerSet (void)
        {
            valid = false;
            for (int i=0; i<SAMPLER_ASSOC; i++)
            {
                samplerBlocks[i].LruStackPosition = i;
            }
        }
};



// The implementation for the cache replacement policy

class CACHE_REPLACEMENT_STATE
{
public:
    LINE_REPLACEMENT_STATE   **repl;


private:

    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    UINT32 samplerReplPolicy;


    COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here
    bool dead;
    bool flag;
    bool isSampled;
    bool isPassby;

public:
    ostream & PrintStats(ostream &out);

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32 GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; }

    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    bool IsPassby(Addr_t PC);

  

private:
    
    void   InitReplacementState(); 
    INT32  Get_Random_Victim( UINT32 setIndex ); 
    INT32  Get_LRU_Victim( UINT32 setIndex );  
    INT32  Get_My_Victim( UINT32 setIndex); 
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID ); 
    void   UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID );

public:

    samplerSet *samplerSets;  

    LINE_REPLACEMENT_STATE **replSampler;

    //  const samplerBlock *vicSamplerSet;

    const LINE_STATE *vicSamplerSet;
    INT32 victimBlock;
    UINT32 samplerNumsets;
    UINT32 samplerSetIndex;
    UINT32 samplerAssoc;
  
    unsigned long long misses, accesses;

    // Predictor table

    Addr_t PC;
    int predictorEntry;
    int counterSum;

    int **table;

    // Predictor function
  
    void InitMyPredictor();

    INT32 predictorResult(Addr_t PC);

    bool IsDead(Addr_t PC);
    void UpdatePredictorDecrease(Addr_t PC);
    void CounterDecrease(Addr_t PC);
    void CounterIncrease(Addr_t PC);

    // sampler function

    INT32 GetSamplerSetIndex(UINT32 setIndex);

    INT32 GetVictimInSamplerSet(UINT32 samplerSetIndex, const LINE_STATE *vicSamplerSet,  UINT32 samplerAssoc, Addr_t PC, Addr_t paddr, UINT32 accessType, bool dead);

    void SetSamplerReplacementPolicy(UINT32 _pol) {replPolicy = _pol;}

    void UpdateSamplerReplacementState(UINT32 samplerSetIndex, INT32 updateWayID, UINT32 setIndex, const LINE_STATE *currBlock, Addr_t PC, UINT32 accessType, bool cacheHit, bool dead);
 
    bool IsSamplerHit(UINT32 setIndex, UINT32 samplerSetIndex);

    void InitSamplerSets();

    INT32 GetSamplerRandomVictim(UINT32 samplerSetIndex);
    INT32 GetSamplerLruVictim(UINT32 samplerSetIndex);
    INT32 GetSamplerMyVictim(UINT32 samplerSetIndex);

    void UpdateSamplerLRU(UINT32 samplerSetIndex, INT32 updateWayID);

    void UpdateSamplerMyPolicy(UINT32 setIndex, UINT32 samplerSetIndex, INT32 updateWayID);
 
    ~CACHE_REPLACEMENT_STATE(void);
};





#endif
