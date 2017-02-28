// Copyright 2009-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2016, Sandia Corporation
// All rights reserved.
// 
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef MESICOHERENCECONTROLLER_H
#define MESICOHERENCECONTROLLER_H

#include <iostream>
#include "coherenceController.h"


namespace SST { namespace MemHierarchy {

class MESIController : public CoherenceController {
public:
    /** Constructor for MESIController. Note that MESIController handles both MESI & MSI protocols */
    MESIController(SST::Component* comp, Params& params) : CoherenceController(comp, params) {
        debug->debug(_INFO_,"--------------------------- Initializing [MESI Controller] ... \n\n");
        
        protocol_ = params.find<bool>("protocol", 1);
        inclusive_ = params.find<bool>("inclusive", true);
      
        /* Statistics */
        stat_evict_S =      registerStatistic<uint64_t>("evict_S");
        stat_evict_SM =     registerStatistic<uint64_t>("evict_SM");
        stat_evict_SInv =   registerStatistic<uint64_t>("evict_SInv");
        stat_evict_MInv =   registerStatistic<uint64_t>("evict_MInv");
        stat_evict_SMInv =  registerStatistic<uint64_t>("evict_SMInv");
        stat_evict_MInvX =  registerStatistic<uint64_t>("evict_MInvX");
        stat_evict_SI =     registerStatistic<uint64_t>("evict_SI");
        stat_stateEvent_GetS_I =    registerStatistic<uint64_t>("stateEvent_GetS_I");
        stat_stateEvent_GetS_S =    registerStatistic<uint64_t>("stateEvent_GetS_S");
        stat_stateEvent_GetS_M =    registerStatistic<uint64_t>("stateEvent_GetS_M");
        stat_stateEvent_GetX_I =    registerStatistic<uint64_t>("stateEvent_GetX_I");
        stat_stateEvent_GetX_S =    registerStatistic<uint64_t>("stateEvent_GetX_S");
        stat_stateEvent_GetX_M =    registerStatistic<uint64_t>("stateEvent_GetX_M");
        stat_stateEvent_GetSEx_I =  registerStatistic<uint64_t>("stateEvent_GetSEx_I");
        stat_stateEvent_GetSEx_S =  registerStatistic<uint64_t>("stateEvent_GetSEx_S");
        stat_stateEvent_GetSEx_M =  registerStatistic<uint64_t>("stateEvent_GetSEx_M");
        stat_stateEvent_GetSResp_IS =       registerStatistic<uint64_t>("stateEvent_GetSResp_IS");
        stat_stateEvent_GetXResp_IM =       registerStatistic<uint64_t>("stateEvent_GetXResp_IM");
        stat_stateEvent_GetXResp_SM =       registerStatistic<uint64_t>("stateEvent_GetXResp_SM");
        stat_stateEvent_GetXResp_SMInv =    registerStatistic<uint64_t>("stateEvent_GetXResp_SMInv");
        stat_stateEvent_PutS_I =        registerStatistic<uint64_t>("stateEvent_PutS_I");
        stat_stateEvent_PutS_S =        registerStatistic<uint64_t>("stateEvent_PutS_S");
        stat_stateEvent_PutS_M =        registerStatistic<uint64_t>("stateEvent_PutS_M");
        stat_stateEvent_PutS_MInv =     registerStatistic<uint64_t>("stateEvent_PutS_MInv");
        stat_stateEvent_PutS_SInv =     registerStatistic<uint64_t>("stateEvent_PutS_SInv");
        stat_stateEvent_PutS_SMInv =    registerStatistic<uint64_t>("stateEvent_PutS_SMInv");
        stat_stateEvent_PutS_MI =       registerStatistic<uint64_t>("stateEvent_PutS_MI");
        stat_stateEvent_PutS_SI =       registerStatistic<uint64_t>("stateEvent_PutS_SI");
        stat_stateEvent_PutS_IB =       registerStatistic<uint64_t>("stateEvent_PutS_IB");
        stat_stateEvent_PutS_SB =       registerStatistic<uint64_t>("stateEvent_PutS_SB");
        stat_stateEvent_PutS_SBInv =    registerStatistic<uint64_t>("stateEvent_PutS_SBInv");
        stat_stateEvent_PutM_I =        registerStatistic<uint64_t>("stateEvent_PutM_I");
        stat_stateEvent_PutM_M =        registerStatistic<uint64_t>("stateEvent_PutM_M");
        stat_stateEvent_PutM_MInv =     registerStatistic<uint64_t>("stateEvent_PutM_MInv");
        stat_stateEvent_PutM_MInvX =    registerStatistic<uint64_t>("stateEvent_PutM_MInvX");
        stat_stateEvent_PutM_MI =       registerStatistic<uint64_t>("stateEvent_PutM_MI");
        stat_stateEvent_Inv_I =         registerStatistic<uint64_t>("stateEvent_Inv_I");
        stat_stateEvent_Inv_S =         registerStatistic<uint64_t>("stateEvent_Inv_S");
        stat_stateEvent_Inv_IS =        registerStatistic<uint64_t>("stateEvent_Inv_IS");
        stat_stateEvent_Inv_IM =        registerStatistic<uint64_t>("stateEvent_Inv_IM");
        stat_stateEvent_Inv_SM =        registerStatistic<uint64_t>("stateEvent_Inv_SM");
        stat_stateEvent_Inv_SInv =      registerStatistic<uint64_t>("stateEvent_Inv_SInv");
        stat_stateEvent_Inv_SMInv =     registerStatistic<uint64_t>("stateEvent_Inv_SMInv");
        stat_stateEvent_Inv_SI =        registerStatistic<uint64_t>("stateEvent_Inv_SI");
        stat_stateEvent_Inv_SB =        registerStatistic<uint64_t>("stateEvent_Inv_SB");
        stat_stateEvent_Inv_IB =        registerStatistic<uint64_t>("stateEvent_Inv_IB");
        stat_stateEvent_FetchInvX_I =       registerStatistic<uint64_t>("stateEvent_FetchInvX_I");
        stat_stateEvent_FetchInvX_M =       registerStatistic<uint64_t>("stateEvent_FetchInvX_M");
        stat_stateEvent_FetchInvX_IS =      registerStatistic<uint64_t>("stateEvent_FetchInvX_IS");
        stat_stateEvent_FetchInvX_IM =      registerStatistic<uint64_t>("stateEvent_FetchInvX_IM");
        stat_stateEvent_FetchInvX_MInv =    registerStatistic<uint64_t>("stateEvent_FetchInvX_MInv");
        stat_stateEvent_FetchInvX_MInvX =   registerStatistic<uint64_t>("stateEvent_FetchInvX_MInvX");
        stat_stateEvent_FetchInvX_MI =      registerStatistic<uint64_t>("stateEvent_FetchInvX_MI");
        stat_stateEvent_FetchInvX_IB =      registerStatistic<uint64_t>("stateEvent_FetchInvX_IB");
        stat_stateEvent_FetchInvX_SB =      registerStatistic<uint64_t>("stateEvent_FetchInvX_SB");
        stat_stateEvent_Fetch_I =           registerStatistic<uint64_t>("stateEvent_Fetch_I");
        stat_stateEvent_Fetch_S =           registerStatistic<uint64_t>("stateEvent_Fetch_S");
        stat_stateEvent_Fetch_IS =          registerStatistic<uint64_t>("stateEvent_Fetch_IS");
        stat_stateEvent_Fetch_IM =          registerStatistic<uint64_t>("stateEvent_Fetch_IM");
        stat_stateEvent_Fetch_SM =          registerStatistic<uint64_t>("stateEvent_Fetch_SM");
        stat_stateEvent_Fetch_SInv =        registerStatistic<uint64_t>("stateEvent_Fetch_SInv");
        stat_stateEvent_Fetch_SI =          registerStatistic<uint64_t>("stateEvent_Fetch_SI");
        stat_stateEvent_Fetch_IB =          registerStatistic<uint64_t>("stateEvent_Fetch_IB");
        stat_stateEvent_Fetch_SB =          registerStatistic<uint64_t>("stateEvent_Fetch_SB");
        stat_stateEvent_FetchInv_I =        registerStatistic<uint64_t>("stateEvent_FetchInv_I");
        stat_stateEvent_FetchInv_S =        registerStatistic<uint64_t>("stateEvent_FetchInv_S");
        stat_stateEvent_FetchInv_M =        registerStatistic<uint64_t>("stateEvent_FetchInv_M");
        stat_stateEvent_FetchInv_IS =       registerStatistic<uint64_t>("stateEvent_FetchInv_IS");
        stat_stateEvent_FetchInv_IM =       registerStatistic<uint64_t>("stateEvent_FetchInv_IM");
        stat_stateEvent_FetchInv_SM =       registerStatistic<uint64_t>("stateEvent_FetchInv_SM");
        stat_stateEvent_FetchInv_MInv =     registerStatistic<uint64_t>("stateEvent_FetchInv_MInv");
        stat_stateEvent_FetchInv_MInvX =    registerStatistic<uint64_t>("stateEvent_FetchInv_MInvX");
        stat_stateEvent_FetchInv_SInv =     registerStatistic<uint64_t>("stateEvent_FetchInv_SInv");
        stat_stateEvent_FetchInv_MI =       registerStatistic<uint64_t>("stateEvent_FetchInv_MI");
        stat_stateEvent_FetchInv_IB =       registerStatistic<uint64_t>("stateEvent_FetchInv_IB");
        stat_stateEvent_FetchInv_SB =       registerStatistic<uint64_t>("stateEvent_FetchInv_SB");
        stat_stateEvent_FetchResp_I =       registerStatistic<uint64_t>("stateEvent_FetchResp_I");
        stat_stateEvent_FetchResp_MInv =    registerStatistic<uint64_t>("stateEvent_FetchResp_MInv");
        stat_stateEvent_FetchResp_MInvX =   registerStatistic<uint64_t>("stateEvent_FetchResp_MInvX");
        stat_stateEvent_FetchResp_MI =      registerStatistic<uint64_t>("stateEvent_FetchResp_MI");
        stat_stateEvent_FetchXResp_I =      registerStatistic<uint64_t>("stateEvent_FetchXResp_I");
        stat_stateEvent_FetchXResp_MInv =   registerStatistic<uint64_t>("stateEvent_FetchXResp_MInv");
        stat_stateEvent_FetchXResp_MInvX =  registerStatistic<uint64_t>("stateEvent_FetchXResp_MInvX");
        stat_stateEvent_FetchXResp_MI =     registerStatistic<uint64_t>("stateEvent_FetchXResp_MI");
        stat_stateEvent_AckInv_I =          registerStatistic<uint64_t>("stateEvent_AckInv_I");
        stat_stateEvent_AckInv_MInv =       registerStatistic<uint64_t>("stateEvent_AckInv_MInv");
        stat_stateEvent_AckInv_MInvX =      registerStatistic<uint64_t>("stateEvent_AckInv_MInvX");
        stat_stateEvent_AckInv_SInv =       registerStatistic<uint64_t>("stateEvent_AckInv_SInv");
        stat_stateEvent_AckInv_SMInv =      registerStatistic<uint64_t>("stateEvent_AckInv_SMInv");
        stat_stateEvent_AckInv_MI =         registerStatistic<uint64_t>("stateEvent_AckInv_MI");
        stat_stateEvent_AckInv_SI =         registerStatistic<uint64_t>("stateEvent_AckInv_SI");
        stat_stateEvent_AckInv_SBInv =      registerStatistic<uint64_t>("stateEvent_AckInv_SBInv");
        stat_stateEvent_FlushLine_I =       registerStatistic<uint64_t>("stateEvent_FlushLine_I");
        stat_stateEvent_FlushLine_S =       registerStatistic<uint64_t>("stateEvent_FlushLine_S");
        stat_stateEvent_FlushLine_M =       registerStatistic<uint64_t>("stateEvent_FlushLine_M");
        stat_stateEvent_FlushLine_IS =      registerStatistic<uint64_t>("stateEvent_FlushLine_IS");
        stat_stateEvent_FlushLine_IM =      registerStatistic<uint64_t>("stateEvent_FlushLine_IM");
        stat_stateEvent_FlushLine_SM =      registerStatistic<uint64_t>("stateEvent_FlushLine_SM");
        stat_stateEvent_FlushLine_MInv =    registerStatistic<uint64_t>("stateEvent_FlushLine_MInv");
        stat_stateEvent_FlushLine_MInvX =   registerStatistic<uint64_t>("stateEvent_FlushLine_MInvX");
        stat_stateEvent_FlushLine_SInv =    registerStatistic<uint64_t>("stateEvent_FlushLine_SInv");
        stat_stateEvent_FlushLine_SMInv =   registerStatistic<uint64_t>("stateEvent_FlushLine_SMInv");
        stat_stateEvent_FlushLine_MI =      registerStatistic<uint64_t>("stateEvent_FlushLine_MI");
        stat_stateEvent_FlushLine_SI =      registerStatistic<uint64_t>("stateEvent_FlushLine_SI");
        stat_stateEvent_FlushLine_IB =      registerStatistic<uint64_t>("stateEvent_FlushLine_IB");
        stat_stateEvent_FlushLine_SB =      registerStatistic<uint64_t>("stateEvent_FlushLine_SB");
        stat_stateEvent_FlushLineInv_I =        registerStatistic<uint64_t>("stateEvent_FlushLineInv_I");
        stat_stateEvent_FlushLineInv_S =        registerStatistic<uint64_t>("stateEvent_FlushLineInv_S");
        stat_stateEvent_FlushLineInv_M =        registerStatistic<uint64_t>("stateEvent_FlushLineInv_M");
        stat_stateEvent_FlushLineInv_IS =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_IS");
        stat_stateEvent_FlushLineInv_IM =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_IM");
        stat_stateEvent_FlushLineInv_SM =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_SM");
        stat_stateEvent_FlushLineInv_MInv =     registerStatistic<uint64_t>("stateEvent_FlushLineInv_MInv");
        stat_stateEvent_FlushLineInv_MInvX =    registerStatistic<uint64_t>("stateEvent_FlushLineInv_MInvX");
        stat_stateEvent_FlushLineInv_SInv =     registerStatistic<uint64_t>("stateEvent_FlushLineInv_SInv");
        stat_stateEvent_FlushLineInv_SMInv =    registerStatistic<uint64_t>("stateEvent_FlushLineInv_SMInv");
        stat_stateEvent_FlushLineInv_MI =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_MI");
        stat_stateEvent_FlushLineInv_SI =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_SI");
        stat_stateEvent_FlushLineInv_IB =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_IB");
        stat_stateEvent_FlushLineInv_SB =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_SB");
        stat_stateEvent_FlushLineResp_I =       registerStatistic<uint64_t>("stateEvent_FlushLineResp_I");
        stat_stateEvent_FlushLineResp_IB =      registerStatistic<uint64_t>("stateEvent_FlushLineResp_IB");
        stat_stateEvent_FlushLineResp_SB =      registerStatistic<uint64_t>("stateEvent_FlushLineResp_SB");
        stat_eventSent_GetS             = registerStatistic<uint64_t>("eventSent_GetS");
        stat_eventSent_GetX             = registerStatistic<uint64_t>("eventSent_GetX");
        stat_eventSent_GetSEx           = registerStatistic<uint64_t>("eventSent_GetSEx");
        stat_eventSent_PutS             = registerStatistic<uint64_t>("eventSent_PutS");
        stat_eventSent_PutM             = registerStatistic<uint64_t>("eventSent_PutM");
        stat_eventSent_FlushLine        = registerStatistic<uint64_t>("eventSent_FlushLine");
        stat_eventSent_FlushLineInv     = registerStatistic<uint64_t>("eventSent_FlushLineInv");
        stat_eventSent_FetchResp        = registerStatistic<uint64_t>("eventSent_FetchResp");
        stat_eventSent_FetchXResp       = registerStatistic<uint64_t>("eventSent_FetchXResp");
        stat_eventSent_AckInv           = registerStatistic<uint64_t>("eventSent_AckInv");
        stat_eventSent_NACK_down        = registerStatistic<uint64_t>("eventSent_NACK_down");
        stat_eventSent_GetSResp         = registerStatistic<uint64_t>("eventSent_GetSResp");
        stat_eventSent_GetXResp         = registerStatistic<uint64_t>("eventSent_GetXResp");
        stat_eventSent_FlushLineResp    = registerStatistic<uint64_t>("eventSent_FlushLineResp");
        stat_eventSent_Fetch            = registerStatistic<uint64_t>("eventSent_Fetch");
        stat_eventSent_FetchInv         = registerStatistic<uint64_t>("eventSent_FetchInv");
        stat_eventSent_FetchInvX        = registerStatistic<uint64_t>("eventSent_FetchInvX");
        stat_eventSent_Inv              = registerStatistic<uint64_t>("eventSent_Inv");
        stat_eventSent_NACK_up          = registerStatistic<uint64_t>("eventSent_NACK_up");
        
        /* Only for non-inclusive caches */
        if (!inclusive_) {
            stat_eventSent_AckPut           = registerStatistic<uint64_t>("eventSent_AckPut");
        }
        
        /* Only for caches that expect writeback acks but we don't know yet so always enabled for now (can't register statistics later) */
        stat_stateEvent_AckPut_I =          registerStatistic<uint64_t>("stateEvent_AckPut_I");
        
        /* MESI-specific statistics (as opposed to MSI) */
        if (protocol_) {
            stat_evict_EInv =   registerStatistic<uint64_t>("evict_EInv");
            stat_evict_EInvX =  registerStatistic<uint64_t>("evict_EInvX");
            stat_stateEvent_GetS_E =    registerStatistic<uint64_t>("stateEvent_GetS_E");
            stat_stateEvent_GetX_E =    registerStatistic<uint64_t>("stateEvent_GetX_E");
            stat_stateEvent_GetSEx_E =  registerStatistic<uint64_t>("stateEvent_GetSEx_E");
            stat_stateEvent_PutS_E =        registerStatistic<uint64_t>("stateEvent_PutS_E");
            stat_stateEvent_PutS_EInv =     registerStatistic<uint64_t>("stateEvent_PutS_EInv");
            stat_stateEvent_PutS_EI =       registerStatistic<uint64_t>("stateEvent_PutS_EI");
            stat_stateEvent_PutE_I =        registerStatistic<uint64_t>("stateEvent_PutE_I");
            stat_stateEvent_PutE_E =        registerStatistic<uint64_t>("stateEvent_PutE_E");
            stat_stateEvent_PutE_M =        registerStatistic<uint64_t>("stateEvent_PutE_M");
            stat_stateEvent_PutE_MInv =     registerStatistic<uint64_t>("stateEvent_PutE_MInv");
            stat_stateEvent_PutE_MInvX =    registerStatistic<uint64_t>("stateEvent_PutE_MInvX");
            stat_stateEvent_PutE_MI =       registerStatistic<uint64_t>("stateEvent_PutE_MI");
            stat_stateEvent_PutE_EInv =     registerStatistic<uint64_t>("stateEvent_PutE_EInv");
            stat_stateEvent_PutE_EInvX =    registerStatistic<uint64_t>("stateEvent_PutE_EInvX");
            stat_stateEvent_PutE_EI =       registerStatistic<uint64_t>("stateEvent_PutE_EI");
            stat_stateEvent_PutM_E =        registerStatistic<uint64_t>("stateEvent_PutM_E");
            stat_stateEvent_PutM_EInv =     registerStatistic<uint64_t>("stateEvent_PutM_EInv");
            stat_stateEvent_PutM_EInvX =    registerStatistic<uint64_t>("stateEvent_PutM_EInvX");
            stat_stateEvent_PutM_EI =       registerStatistic<uint64_t>("stateEvent_PutM_EI");
            stat_stateEvent_FetchInvX_E =       registerStatistic<uint64_t>("stateEvent_FetchInvX_E");
            stat_stateEvent_FetchInvX_EInv =    registerStatistic<uint64_t>("stateEvent_FetchInvX_EInv");
            stat_stateEvent_FetchInvX_EInvX =   registerStatistic<uint64_t>("stateEvent_FetchInvX_EInvX");
            stat_stateEvent_FetchInvX_EI =      registerStatistic<uint64_t>("stateEvent_FetchInvX_EI");
            stat_stateEvent_FetchInv_E =        registerStatistic<uint64_t>("stateEvent_FetchInv_E");
            stat_stateEvent_FetchInv_EInv =     registerStatistic<uint64_t>("stateEvent_FetchInv_EInv");
            stat_stateEvent_FetchInv_EInvX =    registerStatistic<uint64_t>("stateEvent_FetchInv_EInvX");
            stat_stateEvent_FetchInv_EI =       registerStatistic<uint64_t>("stateEvent_FetchInv_EI");
            stat_stateEvent_FetchResp_EInv =    registerStatistic<uint64_t>("stateEvent_FetchResp_EInv");
            stat_stateEvent_FetchResp_EInvX =   registerStatistic<uint64_t>("stateEvent_FetchResp_EInvX");
            stat_stateEvent_FetchResp_EI =      registerStatistic<uint64_t>("stateEvent_FetchResp_EI");
            stat_stateEvent_FetchXResp_EInv =   registerStatistic<uint64_t>("stateEvent_FetchXResp_EInv");
            stat_stateEvent_FetchXResp_EInvX =  registerStatistic<uint64_t>("stateEvent_FetchXResp_EInvX");
            stat_stateEvent_FetchXResp_EI =     registerStatistic<uint64_t>("stateEvent_FetchXResp_EI");
            stat_stateEvent_AckInv_EInv =       registerStatistic<uint64_t>("stateEvent_AckInv_EInv");
            stat_stateEvent_AckInv_EInvX =      registerStatistic<uint64_t>("stateEvent_AckInv_EInvX");
            stat_stateEvent_AckInv_EI =         registerStatistic<uint64_t>("stateEvent_AckInv_EI");
            stat_stateEvent_FlushLine_E =       registerStatistic<uint64_t>("stateEvent_FlushLine_E");
            stat_stateEvent_FlushLine_EInv =    registerStatistic<uint64_t>("stateEvent_FlushLine_EInv");
            stat_stateEvent_FlushLine_EInvX =   registerStatistic<uint64_t>("stateEvent_FlushLine_EInvX");
            stat_stateEvent_FlushLine_EI =      registerStatistic<uint64_t>("stateEvent_FlushLine_EI");
            stat_stateEvent_FlushLineInv_E =        registerStatistic<uint64_t>("stateEvent_FlushLineInv_E");
            stat_stateEvent_FlushLineInv_EInv =     registerStatistic<uint64_t>("stateEvent_FlushLineInv_EInv");
            stat_stateEvent_FlushLineInv_EInvX =    registerStatistic<uint64_t>("stateEvent_FlushLineInv_EInvX");
            stat_stateEvent_FlushLineInv_EI =       registerStatistic<uint64_t>("stateEvent_FlushLineInv_EI");
            stat_eventSent_PutE             = registerStatistic<uint64_t>("eventSent_PutE");
        }
    }
    ~MESIController() {}
    
/*----------------------------------------------------------------------------------------------------------------------
 *  Public functions form external interface to the coherence controller
 *---------------------------------------------------------------------------------------------------------------------*/  

/* Event handlers */
    /** Send cacheline data to the lower level caches */
    CacheAction handleEviction(CacheLine* wbCacheLine, string origRqstr, bool ignoredParam=false);

    /** Process cache request:  GetX, GetS, GetSEx */
    CacheAction handleRequest(MemEvent* event, CacheLine* cacheLine, bool replay);
    
    /** Process replacement request - PutS, PutE, PutM */
    CacheAction handleReplacement(MemEvent* event, CacheLine* cacheLine, MemEvent * reqEvent, bool replay);
    
    /** Process invalidation requests - Inv, FetchInv, FetchInvX */
    CacheAction handleInvalidationRequest(MemEvent *event, CacheLine* cacheLine, MemEvent * collisionEvent, bool replay);

    /** Process responses - GetSResp, GetXResp, FetchResp */
    CacheAction handleResponse(MemEvent* responseEvent, CacheLine* cacheLine, MemEvent* origRequest);

    /* Stuff */
    CacheAction handleHTMEvent(MemEvent* event, CacheLine* line, Command cmd);
    

/* Message send */
    /** Forward a message up, used for non-inclusive caches */
    void forwardMessageUp(MemEvent * event);
    
    /** Call through to coherenceController with statistic recording */
    void addToOutgoingQueue(Response& resp);
    void addToOutgoingQueueUp(Response& resp);

/* Miscellaneous */
    /** Determine in advance if a request will miss (and what kind of miss). Used for stats */
    int isCoherenceMiss(MemEvent* event, CacheLine* cacheLine);
    
    /** Determine whether a NACKed event should be retried */
    bool isRetryNeeded(MemEvent * event, CacheLine * cacheLine);
   
private:
/* Private data members */
    bool                protocol_;  // True for MESI, false for MSI
    bool                inclusive_;
    
/* Statistics */
    Statistic<uint64_t>* stat_evict_S;
    Statistic<uint64_t>* stat_evict_SM;
    Statistic<uint64_t>* stat_evict_SInv;
    Statistic<uint64_t>* stat_evict_EInv;
    Statistic<uint64_t>* stat_evict_MInv;
    Statistic<uint64_t>* stat_evict_SMInv;
    Statistic<uint64_t>* stat_evict_EInvX;
    Statistic<uint64_t>* stat_evict_MInvX;
    Statistic<uint64_t>* stat_evict_SI;
    Statistic<uint64_t>* stat_stateEvent_GetS_I;
    Statistic<uint64_t>* stat_stateEvent_GetS_S;
    Statistic<uint64_t>* stat_stateEvent_GetS_E;
    Statistic<uint64_t>* stat_stateEvent_GetS_M;
    Statistic<uint64_t>* stat_stateEvent_GetX_I;
    Statistic<uint64_t>* stat_stateEvent_GetX_S;
    Statistic<uint64_t>* stat_stateEvent_GetX_E;
    Statistic<uint64_t>* stat_stateEvent_GetX_M;
    Statistic<uint64_t>* stat_stateEvent_GetSEx_I;
    Statistic<uint64_t>* stat_stateEvent_GetSEx_S;
    Statistic<uint64_t>* stat_stateEvent_GetSEx_E;
    Statistic<uint64_t>* stat_stateEvent_GetSEx_M;
    Statistic<uint64_t>* stat_stateEvent_GetSResp_IS;
    Statistic<uint64_t>* stat_stateEvent_GetXResp_IM;
    Statistic<uint64_t>* stat_stateEvent_GetXResp_SM;
    Statistic<uint64_t>* stat_stateEvent_GetXResp_SMInv;
    Statistic<uint64_t>* stat_stateEvent_PutS_I;
    Statistic<uint64_t>* stat_stateEvent_PutS_S;
    Statistic<uint64_t>* stat_stateEvent_PutS_E;
    Statistic<uint64_t>* stat_stateEvent_PutS_M;
    Statistic<uint64_t>* stat_stateEvent_PutS_MInv;
    Statistic<uint64_t>* stat_stateEvent_PutS_EInv;
    Statistic<uint64_t>* stat_stateEvent_PutS_SInv;
    Statistic<uint64_t>* stat_stateEvent_PutS_SMInv;
    Statistic<uint64_t>* stat_stateEvent_PutS_MI;
    Statistic<uint64_t>* stat_stateEvent_PutS_EI;
    Statistic<uint64_t>* stat_stateEvent_PutS_SI;
    Statistic<uint64_t>* stat_stateEvent_PutS_IB;
    Statistic<uint64_t>* stat_stateEvent_PutS_SB;
    Statistic<uint64_t>* stat_stateEvent_PutS_SBInv;
    Statistic<uint64_t>* stat_stateEvent_PutE_I;
    Statistic<uint64_t>* stat_stateEvent_PutE_E;
    Statistic<uint64_t>* stat_stateEvent_PutE_M;
    Statistic<uint64_t>* stat_stateEvent_PutE_MInv;
    Statistic<uint64_t>* stat_stateEvent_PutE_MInvX;
    Statistic<uint64_t>* stat_stateEvent_PutE_EInv;
    Statistic<uint64_t>* stat_stateEvent_PutE_EInvX;
    Statistic<uint64_t>* stat_stateEvent_PutE_MI;
    Statistic<uint64_t>* stat_stateEvent_PutE_EI;
    Statistic<uint64_t>* stat_stateEvent_PutM_I;
    Statistic<uint64_t>* stat_stateEvent_PutM_E;
    Statistic<uint64_t>* stat_stateEvent_PutM_M;
    Statistic<uint64_t>* stat_stateEvent_PutM_MInv;
    Statistic<uint64_t>* stat_stateEvent_PutM_MInvX;
    Statistic<uint64_t>* stat_stateEvent_PutM_EInv;
    Statistic<uint64_t>* stat_stateEvent_PutM_EInvX;
    Statistic<uint64_t>* stat_stateEvent_PutM_MI;
    Statistic<uint64_t>* stat_stateEvent_PutM_EI;
    Statistic<uint64_t>* stat_stateEvent_Inv_I;
    Statistic<uint64_t>* stat_stateEvent_Inv_S;
    Statistic<uint64_t>* stat_stateEvent_Inv_IS;
    Statistic<uint64_t>* stat_stateEvent_Inv_IM;
    Statistic<uint64_t>* stat_stateEvent_Inv_SM;
    Statistic<uint64_t>* stat_stateEvent_Inv_SInv;
    Statistic<uint64_t>* stat_stateEvent_Inv_SMInv;
    Statistic<uint64_t>* stat_stateEvent_Inv_SI;
    Statistic<uint64_t>* stat_stateEvent_Inv_SB;
    Statistic<uint64_t>* stat_stateEvent_Inv_IB;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_I;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_E;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_M;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_IS;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_IM;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_MInv;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_MInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_EInv;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_EInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_MI;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_EI;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_IB;
    Statistic<uint64_t>* stat_stateEvent_FetchInvX_SB;
    Statistic<uint64_t>* stat_stateEvent_Fetch_I;
    Statistic<uint64_t>* stat_stateEvent_Fetch_S;
    Statistic<uint64_t>* stat_stateEvent_Fetch_IS;
    Statistic<uint64_t>* stat_stateEvent_Fetch_IM;
    Statistic<uint64_t>* stat_stateEvent_Fetch_SM;
    Statistic<uint64_t>* stat_stateEvent_Fetch_SInv;
    Statistic<uint64_t>* stat_stateEvent_Fetch_SI;
    Statistic<uint64_t>* stat_stateEvent_Fetch_IB;
    Statistic<uint64_t>* stat_stateEvent_Fetch_SB;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_I;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_S;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_E;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_M;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_IS;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_IM;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_SM;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_MInv;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_MInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_EInv;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_EInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_SInv;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_MI;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_EI;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_IB;
    Statistic<uint64_t>* stat_stateEvent_FetchInv_SB;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_I;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_MInv;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_MInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_EInv;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_EInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_MI;
    Statistic<uint64_t>* stat_stateEvent_FetchResp_EI;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_I;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_MInv;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_MInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_EInv;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_EInvX;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_MI;
    Statistic<uint64_t>* stat_stateEvent_FetchXResp_EI;
    Statistic<uint64_t>* stat_stateEvent_AckInv_I;
    Statistic<uint64_t>* stat_stateEvent_AckInv_MInv;
    Statistic<uint64_t>* stat_stateEvent_AckInv_MInvX;
    Statistic<uint64_t>* stat_stateEvent_AckInv_EInv;
    Statistic<uint64_t>* stat_stateEvent_AckInv_EInvX;
    Statistic<uint64_t>* stat_stateEvent_AckInv_SInv;
    Statistic<uint64_t>* stat_stateEvent_AckInv_SMInv;
    Statistic<uint64_t>* stat_stateEvent_AckInv_MI;
    Statistic<uint64_t>* stat_stateEvent_AckInv_EI;
    Statistic<uint64_t>* stat_stateEvent_AckInv_SI;
    Statistic<uint64_t>* stat_stateEvent_AckInv_SBInv;
    Statistic<uint64_t>* stat_stateEvent_AckPut_I;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_I;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_S;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_E;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_M;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_IS;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_IM;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_SM;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_MInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_MInvX;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_EInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_EInvX;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_SInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_SMInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_MI;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_EI;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_SI;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_IB;
    Statistic<uint64_t>* stat_stateEvent_FlushLine_SB;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_I;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_S;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_E;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_M;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_IS;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_IM;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_SM;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_MInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_MInvX;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_EInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_EInvX;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_SInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_SMInv;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_MI;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_EI;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_SI;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_IB;
    Statistic<uint64_t>* stat_stateEvent_FlushLineInv_SB;
    Statistic<uint64_t>* stat_stateEvent_FlushLineResp_I;
    Statistic<uint64_t>* stat_stateEvent_FlushLineResp_IB;
    Statistic<uint64_t>* stat_stateEvent_FlushLineResp_SB;
    Statistic<uint64_t>* stat_eventSent_GetS;
    Statistic<uint64_t>* stat_eventSent_GetX;
    Statistic<uint64_t>* stat_eventSent_GetSEx;
    Statistic<uint64_t>* stat_eventSent_PutS;
    Statistic<uint64_t>* stat_eventSent_PutE;
    Statistic<uint64_t>* stat_eventSent_PutM;
    Statistic<uint64_t>* stat_eventSent_FlushLine;
    Statistic<uint64_t>* stat_eventSent_FlushLineInv;
    Statistic<uint64_t>* stat_eventSent_FetchResp;
    Statistic<uint64_t>* stat_eventSent_FetchXResp;
    Statistic<uint64_t>* stat_eventSent_AckInv;
    Statistic<uint64_t>* stat_eventSent_NACK_down;
    Statistic<uint64_t>* stat_eventSent_GetSResp;
    Statistic<uint64_t>* stat_eventSent_GetXResp;
    Statistic<uint64_t>* stat_eventSent_FlushLineResp;
    Statistic<uint64_t>* stat_eventSent_AckPut;
    Statistic<uint64_t>* stat_eventSent_Fetch;
    Statistic<uint64_t>* stat_eventSent_FetchInv;
    Statistic<uint64_t>* stat_eventSent_FetchInvX;
    Statistic<uint64_t>* stat_eventSent_Inv;
    Statistic<uint64_t>* stat_eventSent_NACK_up;

/* Private event handlers */
    /** Handle GetX request. Request upgrade if needed */
    CacheAction handleGetXRequest(MemEvent* event, CacheLine* cacheLine, bool replay);

    /** Handle GetS request. Request block if needed */
    CacheAction handleGetSRequest(MemEvent* event, CacheLine* cacheLine, bool replay);
    
    /** Handle FlushLine request. Forward if needed */
    CacheAction handleFlushLineRequest(MemEvent * event, CacheLine * cacheLine, MemEvent * reqEvent, bool replay);

    /** Handle FlushLineInv request. Forward if needed */
    CacheAction handleFlushLineInvRequest(MemEvent * event, CacheLine * cacheLine, MemEvent * reqEvent, bool replay);

    /** Handle PutM request. Write data to cache line.  Update E->M state if necessary */
    CacheAction handlePutMRequest(MemEvent* event, CacheLine* cacheLine, MemEvent * reqEvent);
    
    /** Handle PutS requesst. Update sharer state */ 
    CacheAction handlePutSRequest(MemEvent* event, CacheLine* cacheLine, MemEvent * reqEvent);
    
    /** Handle Inv */
    CacheAction handleInv(MemEvent * event, CacheLine * cacheLine, bool replay);
    
    /** Handle Fetch */
    CacheAction handleFetch(MemEvent * event, CacheLine * cacheLine, bool replay);
    
    /** Handle FetchInv */
    CacheAction handleFetchInv(MemEvent * event, CacheLine * cacheLine, MemEvent * collisionEvent, bool replay);
    
    /** Handle FetchInvX */
    CacheAction handleFetchInvX(MemEvent * event, CacheLine * cacheLine, MemEvent * collisionEvent, bool replay);

    /** Process GetSResp/GetXResp.  Update the cache line */
    CacheAction handleDataResponse(MemEvent* responseEvent, CacheLine * cacheLine, MemEvent * reqEvent);
    
    /** Handle FetchResp */
    CacheAction handleFetchResp(MemEvent * responseEvent, CacheLine* cacheLine, MemEvent * reqEvent);
    
    /** Handle Ack */
    CacheAction handleAckInv(MemEvent * responseEvent, CacheLine* cacheLine, MemEvent * reqEvent);

/* Private methods for sending events */
    /** Send response to lower level cache */
    void sendResponseDown(MemEvent* event, CacheLine* cacheLine, bool dirty, bool replay);
    
    /** Send response to lower level cache, address is not cached */
    void sendResponseDownFromMSHR(MemEvent* response, MemEvent * request, bool dirty);

    /** Send writeback request to lower level caches */
    void sendWriteback(Command cmd, CacheLine* cacheLine, bool dirty, string origRqstr);
    
    /** Send AckPut to upper level cache */
    void sendWritebackAck(MemEvent * event);

    /** Send AckInv to lower level cache */
    void sendAckInv(Addr baseAddr, string origRqstr);

    /** Fetch data from owner and invalidate their copy of the line */
    void sendFetchInv(CacheLine * cacheLine, string rqstr, bool replay);
    
    /** Fetch data from owner and downgrade owner to sharer */
    void sendFetchInvX(CacheLine * cacheLine, string rqstr, bool replay);

    /** Invalidate all sharers of a block. Used for invalidations and evictions */
    void invalidateAllSharers(CacheLine * cacheLine, string rqstr, bool replay);
    
    /** Invalidate all sharers of a block except the requestor (rqstr). Used for upgrade requests. */
    bool invalidateSharersExceptRequestor(CacheLine * cacheLine, string rqstr, string origRqstr, bool replay);
    
    /** Send a flush response */
    void sendFlushResponse(MemEvent * reqEent, bool success);
    
    /** Forward a FlushLine request with or without data */
    void forwardFlushLine(Addr baseAddr, string origRqstr, CacheLine * cacheLine, Command cmd);

/* Helper methods */
   
    void printData(vector<uint8_t> * data, bool set);

/* Statistics */
    void recordStateEventCount(Command cmd, State state);
    void recordEvictionState(State state);
    void recordEventSentUp(Command cmd);
    void recordEventSentDown(Command cmd);
};


}}


#endif

