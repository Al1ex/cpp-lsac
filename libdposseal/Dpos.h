#pragma once

#include "Common.h"
#include "DposHostCapability.h"
#include <libethcore/SealEngine.h>
#include <libethcore/Common.h>
#include <libp2p/Common.h>
#include <libdevcore/Worker.h>

namespace dev
{
    namespace bacd
	{
	using namespace dev ::eth;
	    class Dpos: public SealEngineBase, Worker
		{
		public:
			Dpos();
			~Dpos() { stopWorking(); }
			static std::string  name(){ return "Dpos"; }
			unsigned            revision() const override { return 1; }
			unsigned            sealFields() const override { return 2; }
			strings             sealers() const override { return { "cpu" }; };
			void                generateSeal(BlockHeader const& _bi) override;
			static void         init();
			void                initEnv(std::weak_ptr<DposHostcapality> _host);
			inline void         startGeneration()   { setName("Poa"); startWorking(); }   //loop ����
			inline void         SetCurrValitor(Address const& _addr) { m_curr_valitor = Address(_addr); }
		protected:
			void                workLoop() override;
		public:
			void                onPoaMsg(NodeID _nodeid, unsigned _id, RLP const& _r);
			void                requestStatus(NodeID const& _nodeID, u256 const& _peerCapabilityVersion);
		private:
			// �㲥��Ϣ
			void                brocastMsg(DposPacketType _type, RLPStream& _msg_s);
			//ָ������
			void                sealAndSend(NodeID const& _nodeid, DposPacketType _type, RLPStream const& _msg_s);

		public:
			void                updateDposData(EDposDataType _type, Address const& _addr_source, Address const& _addr_target);

			bool                isBolckSeal(uint64_t _last_time ,uint64_t _now);
			bool                checkDeadline(uint64_t _last_block_time,uint64_t _now);           //��֤����ʱ������
		private:
			bool                CheckValidator(uint64_t _now);                  //��֤�Ƿ�õ�ǰ�ڵ����
			bool                tryElect(uint64_t _last_time, uint64_t _now);   //�ж��Ƿ�����˱��ֳ��飬ѡ����һ����֤��
			void                kickoutValidator(unsigned int _prveslot);       //�޳����ϸ���֤�ˣ����ܲ�����һ�־�ѡ
			void                countVotes(){}
            void                disorganizeVotes(){}
		private:
			mutable  Mutex                  m_mutex;
			std::weak_ptr<DposHostcapality> m_host;
			DposMsgQueue                    m_msg_queue;

			SVarlitor_Voters                m_varlitor_vote;         //��֤�˶�ӦͶƱ�� 1��n
			Voter_Varlitor                  m_vote_varlitor;         //ͶƱ�˶�Ӧ��֤�� 1��1
			Canlidate                       m_canditate;             //��ѡ��

			CurrValirots                    m_curr_valitors;         //������֤�˼���
			Address                         m_curr_valitor;          //���ڵ���֤��

            //���ִ�������
			unsigned int                    m_prveslot;                //��һ������
			unsigned int                    m_currslot;                //��������

		};

        
	}
}