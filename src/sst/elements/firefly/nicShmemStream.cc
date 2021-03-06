// Copyright 2009-2017 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2017, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#include "sst_config.h"
#include "nic.h"

using namespace SST;
using namespace SST::Firefly;


Nic::RecvMachine::ShmemStream::ShmemStream( Output& output, FireflyNetworkEvent* ev,
       RecvMachine& rm  ) : 
    StreamBase(output, rm )
{
    m_hdr = *(MsgHdr*) ev->bufPtr();
    m_shmemHdr = *(ShmemMsgHdr*) ev->bufPtr( sizeof(MsgHdr) );

    ev->bufPop(sizeof(MsgHdr) + sizeof(m_shmemHdr) );

    Callback callback;
    switch ( m_shmemHdr.op ) { 
        
      case ShmemMsgHdr::Put: 
        callback = processPut( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      case ShmemMsgHdr::Get: 
        callback = processGet( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      case ShmemMsgHdr::Add: 
        callback = processAdd( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      case ShmemMsgHdr::Fadd: 
        callback = processFadd( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      case ShmemMsgHdr::Cswap: 
        callback = processCswap( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      case ShmemMsgHdr::Swap: 
        callback = processSwap( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      case ShmemMsgHdr::Ack: 
        callback = processAck( m_shmemHdr, ev, m_hdr.dst_vNicId, m_hdr.src_vNicId );
        break;

      default:
        assert(0);
        break;
    }
    m_rm.nic().schedCallback( callback, 0 );
}

Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processAck( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{
    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Ack\n");
	m_rm.m_nic.shmemDecPending( local_vNic );

    return std::bind( &Nic::RecvMachine::state_move_2, &m_rm, ev );
}

Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processPut( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{

    // this is not a get response
    if ( ! hdr.respKey ) { 
        m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d myAddr=%#" PRIx64 " length=%u\n",
            m_shmemHdr.op, m_shmemHdr.vaddr, m_shmemHdr.length);
        
        Hermes::MemAddr addr = m_rm.nic().findShmem( local_vNic, hdr.vaddr, hdr.length ); 

        if ( hdr.op2 == Hermes::Shmem::MOVE ) {
            m_recvEntry = new ShmemRecvEntry( m_rm.m_nic.m_shmem, local_vNic, addr, hdr.length );
        }else{
            m_recvEntry = new ShmemRecvEntry( m_rm.m_nic.m_shmem, local_vNic, addr, hdr.length,
                                (Hermes::Shmem::ReduOp) hdr.op2, 
                                (Hermes::Value::Type) hdr.dataType );
        } 

		m_sendEntry = new ShmemAckSendEntry( local_vNic, ev->src, dest_vNic );

    } else {
        m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d respKey=%#" PRIx64 "\n",
            m_shmemHdr.op, m_shmemHdr.respKey);

        ShmemRespSendEntry* entry = (ShmemRespSendEntry*)hdr.respKey;

        if ( entry->getCmd()->type == NicShmemCmdEvent::Getv || 
               entry->getCmd()->type == NicShmemCmdEvent::Fadd || 
               entry->getCmd()->type == NicShmemCmdEvent::Swap || 
               entry->getCmd()->type == NicShmemCmdEvent::Cswap ) {
            m_recvEntry = new ShmemGetvRespRecvEntry( m_rm.m_nic.m_shmem, hdr.length, static_cast<ShmemGetvSendEntry*>(entry) );
        } else {
            Hermes::Vaddr addr = static_cast<NicShmemGetCmdEvent*>(entry->getCmd())->getMyAddr();

            Hermes::MemAddr memAddr = m_rm.nic().findShmem( local_vNic, addr, hdr.length ); 

            m_recvEntry = new ShmemGetbRespRecvEntry( m_rm.m_nic.m_shmem, local_vNic, hdr.length, static_cast<ShmemGetbSendEntry*>(entry), 
                    memAddr.getBacking() );
        }
    }
	m_matched_len = hdr.length;

    return std::bind( &Nic::RecvMachine::state_move_0, &m_rm, ev, this );
}

Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processGet( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{
    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d myAddr=%#" PRIx64 " length=%u respKey=%#" PRIx64 "\n",
            m_shmemHdr.op, m_shmemHdr.vaddr, m_shmemHdr.length, m_shmemHdr.respKey);

    Hermes::MemAddr addr = m_rm.nic().findShmem( local_vNic, hdr.vaddr, hdr.length ); 

    m_rm.nic().m_sendMachine[0]->run( new ShmemPut2SendEntry( local_vNic, ev->src, dest_vNic, addr.getBacking(), 
                hdr.length, hdr.respKey ) );

    return std::bind( &Nic::RecvMachine::state_move_2, &m_rm, ev );
}

Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processAdd( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{
    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d myAddr=%#" PRIx64 " length=%u respKey=%#" PRIx64 "\n",
            m_shmemHdr.op, m_shmemHdr.vaddr, m_shmemHdr.length, m_shmemHdr.respKey);
    Hermes::MemAddr addr = m_rm.nic().findShmem( local_vNic, hdr.vaddr, hdr.length ); 

    assert( ev->bufSize() == Hermes::Value::getLength((Hermes::Value::Type)hdr.dataType) );

    Hermes::Value local( (Hermes::Value::Type) hdr.dataType, addr.getBacking());
    Hermes::Value got( (Hermes::Value::Type) hdr.dataType, ev->bufPtr() );

    local += got;

    m_rm.m_nic.m_shmem->checkWaitOps( local_vNic, addr.getSimVAddr(), local.getLength() );

	m_matched_len = hdr.length;

	m_sendEntry = new ShmemAckSendEntry( local_vNic, ev->src, dest_vNic );

    return std::bind( &Nic::RecvMachine::state_move_2, &m_rm, ev );
}

Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processFadd( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{
    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d myAddr=%#" PRIx64 " length=%u respKey=%#" PRIx64 "\n",
            m_shmemHdr.op, m_shmemHdr.vaddr, m_shmemHdr.length, m_shmemHdr.respKey);
    Hermes::MemAddr addr = m_rm.nic().findShmem( local_vNic, hdr.vaddr, hdr.length ); 

    assert( ev->bufSize() == Hermes::Value::getLength((Hermes::Value::Type)hdr.dataType) );

    Hermes::Value* save = new Hermes::Value( (Hermes::Value::Type)hdr.dataType );
    Hermes::Value local( (Hermes::Value::Type) hdr.dataType, addr.getBacking());
    Hermes::Value got( (Hermes::Value::Type) hdr.dataType, ev->bufPtr() );

    *save = local;

    local += got;

    m_rm.m_nic.m_shmem->checkWaitOps( local_vNic, addr.getSimVAddr(), local.getLength() );

    m_rm.nic().m_sendMachine[0]->run( new ShmemPut2SendEntry( local_vNic, ev->src, dest_vNic, save, hdr.respKey ) );

    return std::bind( &Nic::RecvMachine::state_move_2, &m_rm, ev );
}

Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processSwap( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{
    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d myAddr=%#" PRIx64 " length=%u respKey=%#" PRIx64 "\n",
            m_shmemHdr.op, m_shmemHdr.vaddr, m_shmemHdr.length, m_shmemHdr.respKey);
    Hermes::MemAddr addr = m_rm.nic().findShmem( local_vNic, hdr.vaddr, hdr.length ); 

    assert( ev->bufSize() == Hermes::Value::getLength((Hermes::Value::Type)hdr.dataType) );

    Hermes::Value local( (Hermes::Value::Type)hdr.dataType, addr.getBacking());
    Hermes::Value* save = new Hermes::Value( (Hermes::Value::Type)hdr.dataType );
    *save = local;
    Hermes::Value swap( (Hermes::Value::Type)hdr.dataType, ev->bufPtr() );

    local = swap;

    m_rm.nic().m_sendMachine[0]->run( new ShmemPut2SendEntry( local_vNic, ev->src, dest_vNic, save, hdr.respKey ) );

    return std::bind( &Nic::RecvMachine::state_move_2, &m_rm, ev );
}
Nic::RecvMachine::ShmemStream::Callback Nic::RecvMachine::ShmemStream::processCswap( ShmemMsgHdr& hdr, FireflyNetworkEvent* ev, int local_vNic, int dest_vNic )
{
    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation %d myAddr=%#" PRIx64 " length=%u respKey=%#" PRIx64 "\n",
            m_shmemHdr.op, m_shmemHdr.vaddr, m_shmemHdr.length, m_shmemHdr.respKey);
    Hermes::MemAddr addr = m_rm.nic().findShmem( local_vNic, hdr.vaddr, hdr.length ); 

    assert( ev->bufSize() == Hermes::Value::getLength((Hermes::Value::Type)hdr.dataType) * 2 );

    Hermes::Value local( (Hermes::Value::Type) hdr.dataType, addr.getBacking());
    Hermes::Value* save = new Hermes::Value( (Hermes::Value::Type) hdr.dataType );
    *save = local;
    Hermes::Value swap( (Hermes::Value::Type) hdr.dataType, ev->bufPtr() );
    ev->bufPop(swap.getLength());
    Hermes::Value cond( (Hermes::Value::Type) hdr.dataType, ev->bufPtr() );

#if 0
    std::stringstream tmp1;
    tmp1 << local;
    std::stringstream tmp2;
    tmp2 << cond;
    std::stringstream tmp3;
    tmp3 << swap;

    m_dbg.verbose(CALL_INFO,1,NIC_DBG_RECV_MACHINE,"SHMEM Operation local=%s cond=%s swap=%s\n",
                        tmp1.str().c_str(), tmp2.str().c_str(), tmp3.str().c_str());
#endif

    if ( local == cond ) {
        local = swap;
    }

    m_rm.nic().m_sendMachine[0]->run( new ShmemPut2SendEntry( local_vNic, ev->src, dest_vNic, save, hdr.respKey ) );

    return std::bind( &Nic::RecvMachine::state_move_2, &m_rm, ev );
}
