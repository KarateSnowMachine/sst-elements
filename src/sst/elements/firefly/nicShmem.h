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


class Shmem {


    class Op {
      public:
        typedef std::function<void()> Callback;
        enum Type { Wait } m_type;  
        Op( Type type, NicShmemOpCmdEvent* cmd, Callback callback ) : m_type(type), m_cmd(cmd), m_callback(callback) {}
        virtual ~Op() { 
            m_callback();
            delete m_cmd;
        }
        virtual bool checkOp( ) = 0;
        bool inRange( Hermes::Vaddr addr, size_t length ) {
            //printf("%s() addr=%lu length=%lu\n",__func__,addr, length);
            return ( m_cmd->addr >= addr && m_cmd->addr + m_cmd->value.getLength() <= addr + length );
        }

      protected:
        NicShmemOpCmdEvent* m_cmd;
        Callback            m_callback;
        Hermes::Value       m_value;   
    };

    class WaitOp : public Op {
      public:
        WaitOp( NicShmemOpCmdEvent* cmd, void* backing, Callback callback ) : 
            Op( Wait, cmd, callback ), 
            m_value( cmd->value.getType(), backing ) 
        {} 

        bool checkOp() {
#if 0
            std::stringstream tmp;
            tmp << m_value << " " << m_cmd->op << " " << m_cmd->value;
            printf("%s %s\n",__func__,tmp.str().c_str());
#endif
            switch ( m_cmd->op ) {
              case Hermes::Shmem::NE:
                return m_value != m_cmd->value; 
                break;
              case Hermes::Shmem::GTE:
                return m_value >= m_cmd->value; 
                break;
              case Hermes::Shmem::GT:
                return m_value > m_cmd->value; 
                break;
              case Hermes::Shmem::EQ:
                return m_value == m_cmd->value; 
                break;
              case Hermes::Shmem::LT:
                return m_value < m_cmd->value;
                break;
              case Hermes::Shmem::LTE:
                return m_value <= m_cmd->value;
                break;
              default:
                assert(0);
            } 
            return false;
        } 
      private:
        Hermes::Value m_value;
    }; 

  public:
    Shmem( Nic& nic, int numVnics, Output& output, SimTime_t nic2HostDelay_ns, SimTime_t host2NicDelay_ns ) : 
		m_nic( nic ), m_dbg(output), m_one( (long) 1 ), m_freeCmdSlots( 32 ),
    	m_nic2HostDelay_ns(nic2HostDelay_ns), m_host2NicDelay_ns(host2NicDelay_ns)
    {
		m_regMem.resize( numVnics ); 
		m_pendingOps.resize( numVnics );
		m_pendingRemoteOps.resize( numVnics );
	}
    ~Shmem() {
        m_regMem.clear();
    }
	void handleEvent( NicShmemCmdEvent* event, int id );
	void handleEvent2( NicShmemCmdEvent* event, int id );
	void decPending( int core ) {
		m_pendingRemoteOps[core].second -= m_one;
		checkWaitOps( core, m_pendingRemoteOps[core].first, m_pendingRemoteOps[core].second.getLength() );
	}	

    std::pair<Hermes::MemAddr, size_t>& findRegion( int core, uint64_t addr ) { 
//		printf("%s() core=%d %#" PRIx64 "\n",__func__,core,addr);
        for ( int i = 0; i < m_regMem[core].size(); i++ ) {
            if ( addr >= m_regMem[core][i].first.getSimVAddr() &&
                addr < m_regMem[core][i].first.getSimVAddr() + m_regMem[core][i].second ) {
                return m_regMem[core][i]; 
            } 
        } 
        assert(0);
    }
    void checkWaitOps( int core, Hermes::Vaddr addr, size_t length );

private:
	SimTime_t getNic2HostDelay_ns() { return m_nic2HostDelay_ns; }
	SimTime_t getHost2NicDelay_ns() { return m_host2NicDelay_ns; }
    void init( NicShmemInitCmdEvent*, int id );
    void regMem( NicShmemRegMemCmdEvent*, int id );
    void wait( NicShmemOpCmdEvent*, int id );
    void put( NicShmemPutCmdEvent*, int id );
    void putv( NicShmemPutvCmdEvent*, int id );
    void get( NicShmemGetCmdEvent*, int id );
    void getv( NicShmemGetvCmdEvent*, int id );
    void add( NicShmemAddCmdEvent*, int id );
    void fadd( NicShmemFaddCmdEvent*, int id );
    void cswap( NicShmemCswapCmdEvent*, int id );
    void swap( NicShmemSwapCmdEvent*, int id );

    void* getBacking( int core, Hermes::Vaddr addr, size_t length ) {
        return  m_nic.findShmem( core, addr, length ).getBacking();
    }
	void doReduction( Hermes::Shmem::ReduOp op, unsigned char* dest, unsigned char* src, size_t length, Hermes::Value::Type );

	void incFreeCmdSlots( ) {
		++m_freeCmdSlots;
		if ( ! m_pendingCmds.empty() ) {
			handleEvent( m_pendingCmds.front().first, m_pendingCmds.front().second );
			m_pendingCmds.pop_front();
		}
	}

	std::deque<std::pair<NicShmemCmdEvent*, int> > m_pendingCmds;
	int m_freeCmdSlots;

	Hermes::Value m_one;
	std::vector< std::pair< Hermes::Vaddr, Hermes::Value > > m_pendingRemoteOps;
    Nic& m_nic;
    Output& m_dbg;
    std::vector< std::list<Op*> > m_pendingOps;
    std::vector<std::vector< std::pair<Hermes::MemAddr, size_t> > > m_regMem;
	SimTime_t m_nic2HostDelay_ns;
	SimTime_t m_host2NicDelay_ns;
};
