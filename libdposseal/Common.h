#pragma once
/*
    dpos ������ݽṹ
*/
#include <iostream>
#include <libdevcore/concurrent_queue.h>
#include <libdevcore/RLP.h>
#include <libdevcrypto/Common.h>
#include <libethcore/Exceptions.h>
#include <libp2p/Common.h>
#include <libdevcore/FixedHash.h>

namespace dev
{
	namespace bacd
    {
		using NodeID = p2p::NodeID;
        extern const unsigned c_protocolVersion;

		const unsigned int epochInterval = 3600;        // һ��������ѯ���� s
		const unsigned int blockInterval = 3;           // һ������̳���ʱ�� 
		const unsigned int valitorNum = 5;              // ɸѡ��֤�˵��������ֵ
		const unsigned int maxValitorNum = 21;          // �����֤������

		enum DposPacketType :byte
		{
			DposStatuspacket = 0x23,
            DposDataPacket,
			DposPacketCount
		};
		enum EDposDataType
		{
            e_loginCandidate =0,         // ��Ϊ��ѡ��
            e_logoutCandidate,
            e_delegate,            // �ƾ���֤��
            e_unDelegate,

            e_max,
		};

		using Canlidate = std::set<Address>;                    //��ѡ�˼���
		using Voters = std::set<Address>;
		using Varlitor_Voter = std::map<Address, Voters>;  //��֤�˶�Ӧ��ͶƱ��
		using Voter_Varlitor = std::map<Address, Address>; //ͶƱ�˶�Ӧ��֤��
		using CurrValirots = std::vector<Address>;         //������ȷ������֤�ˣ�˳��̶�
		struct SVarlitor_Voters
		{
			Varlitor_Voter m_varlitor_voter;

			SVarlitor_Voters(){ m_varlitor_voter.clear(); }
			inline void clear(){ m_varlitor_voter.clear(); }
			unsigned int voterCount(const Address& _addr_varlitor) const
			{
				auto ret = m_varlitor_voter.find(_addr_varlitor);
				if(ret == m_varlitor_voter.end())
					return 0;
				return ret->second.size();
			}
			//�Ḳ��֮ǰ���ڵ�ͷƬ��Ϣ
			inline void insert(const Address& _addr_varlitor, const Address& _addr_voter)
			{
				auto ret = m_varlitor_voter.find(_addr_varlitor);
                if(ret == m_varlitor_voter.end())
				{
					Voters voters;
					voters.insert(_addr_voter);
					m_varlitor_voter[_addr_varlitor] = voters;
				}
				else
				{
					ret->second.insert(_addr_voter);
				}
			}

		};

        // �������ݰ� ��װҵ������
		struct DposMsgPacket  
		{
			h512        node_id;     // peer id
			unsigned    packet_id;   //msg id
			bytes       data;        //rlp data

			DposMsgPacket() :node_id(h512(0)), packet_id(0) {}
			DposMsgPacket(h512 _id, unsigned _pid, bytesConstRef _data)
				:node_id(_id), packet_id(_pid), data(_data.toBytes()) {}
		};
		using DposMsgQueue = dev::concurrent_queue<DposMsgPacket>;

        struct DPosStatusMsg
		{
			uint64_t        m_lastTime;
		};

        struct DposDataMsg
		{
		    
		};
    }
}