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


#include <sst_config.h>
#include <sst/core/interfaces/stringEvent.h>

#include "HTM.h"
#include "memEvent.h"
#include "coherenceController.h"


SST::MemHierarchy::HTM::HTM(ComponentId_t id, Params &params) : Component(id)
{
    /* --------------- Output Class --------------- */
    output_ = new Output();
    int debugLevel = params.find<int>("debug_level", 0);

    output_->init("--->  ", debugLevel, 0,(Output::output_location_t)params.find<int>("debug", 0));
    if(debugLevel < 0 || debugLevel > 10)     output_->fatal(CALL_INFO, -1, "Debugging level must be between 0 and 10. \n");
    output_->debug(_INFO_,"--------------------------- Initializing [HTM]: %s... \n", this->Component::getName().c_str());

    /* --------------- Get Parameters --------------- */
    core_count_ = (uint32_t) params.find<uint32_t>("corecount", 1);

    // LRU - default replacement policy
    int associativity           = params.find<int>("associativity", -1);
    string sizeStr              = params.find<std::string>("cache_size", "");         //Bytes
    int lineSize                = params.find<int>("cache_line_size", -1);            //Bytes

    /* Check user specified all required fields */
    if(-1 >= associativity)         output_->fatal(CALL_INFO, -1, "Param not specified: associativity\n");
    if(sizeStr.empty())             output_->fatal(CALL_INFO, -1, "Param not specified: cache_size\n");
    if(-1 == lineSize)              output_->fatal(CALL_INFO, -1, "Param not specified: cache_line_size - number of bytes in a cacheline (block size)\n");

    fixByteUnits(sizeStr);
    UnitAlgebra ua(sizeStr);
    if (!ua.hasUnits("B")) {
        output_->fatal(CALL_INFO, -1, "Invalid param: cache_size - must have units of bytes (e.g., B, KB,etc.)\n");
    }
    uint64_t cacheSize = ua.getRoundedValue();
    uint numLines = cacheSize/lineSize;

    /* ---------------- Initialization ----------------- */
    HashFunction* ht = new PureIdHashFunction;
    ReplacementMgr* replManager = new LRUReplacementMgr(output_, numLines, associativity, true);
    CacheArray* cacheArray = new SetAssociativeArray(output_, numLines, lineSize, associativity, replManager, ht, false);

    /* --------------- Setup links --------------- */
    // Going Down toward Main Memory
    if (isPortConnected("htm_low_link"))
    {
        lowLink_ = configureLink("htm_low_link", "50ps", new Event::Handler<HTM>(this, &HTM::processResponse));
        output_->debug(_INFO_, "Low Network Link ID: %u\n", (uint)lowLink_->getId());
    } else
    {
        output_->fatal(CALL_INFO, -1, "%s, Error: no connected cache port. Please connect a cache to port 'cache'\n", getName().c_str());
    }

    // Going Up Toward PE
    if (isPortConnected("htm_high_link"))
    {
        highLink_ = configureLink("htm_high_link", "50ps", new Event::Handler<HTM>(this, &HTM::processRequest));
        output_->debug(_INFO_, "High Network Link ID: %u\n", (uint)highLink_->getId());
    } else
    {
        output_->fatal(CALL_INFO, -1, "%s, Error: no connected cache port. Please connect a cache to port 'cache'\n", getName().c_str());
    }

    /* Register statistics */
    statReadSetSize    = registerStatistic<uint64_t>("ReadSetSize");
    statWriteSetSize  = registerStatistic<uint64_t>("WriteSetSize");
    statAborts   = registerStatistic<uint64_t>("NumAborts");
    statCommits = registerStatistic<uint64_t>("NumCommits");
}

void SST::MemHierarchy::HTM::init(unsigned int phase)
{
    SST::Event *ev;

    if(!phase)
    {
        highLink_->sendInitData(new MemEvent(this, 0, 0, NULLCMD));
        lowLink_->sendInitData(new MemEvent(this, 10, 10, NULLCMD));
    }

    while ((ev = highLink_->recvInitData()))
    {
        MemEvent* memEvent = dynamic_cast<MemEvent*>(ev);
        if (!memEvent) { /* Do nothing */ }
        else if (memEvent->getCmd() == NULLCMD) {
            if (memEvent->getCmd() == NULLCMD) {    // Save upper level cache names
                upperLevelNames_.push_back(memEvent->getSrc());
            }
        } else {
                lowLink_->sendInitData(new MemEvent(*memEvent));
        }
        delete memEvent;
     }

    while ((ev = lowLink_->recvInitData()))
    {
        MemEvent* memEvent = dynamic_cast<MemEvent*>(ev);
        if (memEvent && memEvent->getCmd() == NULLCMD) {
            lowerLevelNames_.push_back(memEvent->getSrc());
        }
        delete memEvent;
    }

//     std::cout << "High Name:  " << upperLevelNames_[0] << "\n";
//     std::cout << "Low Name:  "  << lowerLevelNames_[0] << "\n";

}

void SST::MemHierarchy::HTM::processRequest(SST::Event* ev)
{
    MemEvent* event = static_cast<MemEvent*>(ev);

//     processEvent(event, 0);

    Addr baseAddr       = event->getBaseAddr();
    Command cmd         = event->getCmd();
    bool noncacheable   = event->queryFlag(MemEvent::F_NONCACHEABLE);

#ifdef __SST_DEBUG_OUTPUT__
    output_->debug(_L3_,"\n\n-------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    output_->debug(_L3_,"HTM:  REQUEST Name: %s, Cmd: %s, BsAddr: %" PRIx64 ", Addr: %" PRIx64 ", VAddr: %" PRIx64 ", iPtr: %" PRIx64 ", Rqstr: %s, Src: %s, Dst: %s, PreF:%s, Bytes requested = %u, cycles: %" PRIu64 ", %s\n",
                this->getName().c_str(), CommandString[event->getCmd()], baseAddr, event->getAddr(), event->getVirtualAddress(), event->getInstructionPointer(), event->getRqstr().c_str(),
                event->getSrc().c_str(), event->getDst().c_str(), event->isPrefetch() ? "true" : "false", event->getSize(), 0, noncacheable ? "noncacheable" : "cacheable");
#endif

    if(cmd == BeginTx)
    {
        std::cout << "HTM:  Start Transaction\n\n" << std::flush;
        inc_transactionDepth();
    }
    else if(cmd == EndTx)
    {
        std::cout << "HTM:  End Transaction\n\n" << std::flush;
        dec_transactionDepth();
    }
    else
    {
        //Nothing to do, so pass the event on
        lowLink_->send(event);
    }


}

void SST::MemHierarchy::HTM::processResponse(SST::Event* ev)
{
    MemEvent* event = static_cast<MemEvent*>(ev);

//     processEvent(event, 1);

    Addr baseAddr       = event->getBaseAddr();
    Command cmd         = event->getCmd();
    bool noncacheable   = event->queryFlag(MemEvent::F_NONCACHEABLE);

#ifdef __SST_DEBUG_OUTPUT__
    output_->debug(_L3_,"\n\n-------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    output_->debug(_L3_,"HTM:  RESPONSE Name: %s, Cmd: %s, BsAddr: %" PRIx64 ", Addr: %" PRIx64 ", VAddr: %" PRIx64 ", iPtr: %" PRIx64 ", Rqstr: %s, Src: %s, Dst: %s, PreF:%s, Bytes requested = %u, cycles: %" PRIu64 ", %s\n",
                this->getName().c_str(), CommandString[event->getCmd()], baseAddr, event->getAddr(), event->getVirtualAddress(), event->getInstructionPointer(), event->getRqstr().c_str(),
                event->getSrc().c_str(), event->getDst().c_str(), event->isPrefetch() ? "true" : "false", event->getSize(), 0, noncacheable ? "noncacheable" : "cacheable");
#endif

    //Nothing to do, so pass the event on
    highLink_->send(event);

}

void SST::MemHierarchy::HTM::processEvent(MemEvent* event, uint32_t direction)
{
    MemEvent * forwardEvent = new MemEvent(*event);
    forwardEvent->setSrc(event->getSrc());
    forwardEvent->setDst(event->getDst());

    Addr baseAddr       = forwardEvent->getBaseAddr();
    Command cmd         = forwardEvent->getCmd();
    bool noncacheable   = forwardEvent->queryFlag(MemEvent::F_NONCACHEABLE);

#ifdef __SST_DEBUG_OUTPUT__
    output_->debug(_L3_,"\n\n-------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    std::cout << flush;
#endif

    if(cmd == BeginTx)
    {
        std::cout << "HTM Start Transaction\n\n" << std::flush;
    }
    else if(cmd == EndTx)
    {
        std::cout << "HTM End Transaction\n\n" << std::flush;
    }

    if(direction == 0)
    {
#ifdef __SST_DEBUG_OUTPUT__
        output_->debug(_L3_,"HTM-REQ. Name: %s, Cmd: %s, BsAddr: %" PRIx64 ", Addr: %" PRIx64 ", VAddr: %" PRIx64 ", iPtr: %" PRIx64 ", Rqstr: %s, Src: %s, Dst: %s, PreF:%s, Bytes requested = %u, cycles: %" PRIu64 ", %s\n",
                    this->getName().c_str(), CommandString[forwardEvent->getCmd()], baseAddr, forwardEvent->getAddr(), forwardEvent->getVirtualAddress(), forwardEvent->getInstructionPointer(), forwardEvent->getRqstr().c_str(),
                    forwardEvent->getSrc().c_str(), forwardEvent->getDst().c_str(), forwardEvent->isPrefetch() ? "true" : "false", forwardEvent->getSize(), 0, noncacheable ? "noncacheable" : "cacheable");
#endif

        //Nothing to do, so pass the event on
        lowLink_->send(forwardEvent);
    }
    else
    {
#ifdef __SST_DEBUG_OUTPUT__
        output_->debug(_L3_,"HTM-RES. Name: %s, Cmd: %s, BsAddr: %" PRIx64 ", Addr: %" PRIx64 ", VAddr: %" PRIx64 ", iPtr: %" PRIx64 ", Rqstr: %s, Src: %s, Dst: %s, PreF:%s, Bytes requested = %u, cycles: %" PRIu64 ", %s\n",
                    this->getName().c_str(), CommandString[forwardEvent->getCmd()], baseAddr, forwardEvent->getAddr(), forwardEvent->getVirtualAddress(), forwardEvent->getInstructionPointer(), forwardEvent->getRqstr().c_str(),
                    forwardEvent->getSrc().c_str(), forwardEvent->getDst().c_str(), forwardEvent->isPrefetch() ? "true" : "false", forwardEvent->getSize(), 0, noncacheable ? "noncacheable" : "cacheable");
#endif

        //Nothing to do, so pass the event on
        highLink_->send(forwardEvent);
    }


}


