#include "Dpos.h"

dev::bacd::Dpos::Dpos()
{
	m_varlitor_vote.clear();
	m_vote_varlitor.clear();
	m_curr_valitor.clear();
	m_curr_valitors.clear();
	m_canditate.clear();
	m_prveslot = 0;
	m_currslot = 0;
}

void dev::bacd::Dpos::generateSeal(BlockHeader const & _bi)
{
	BlockHeader header(_bi);
	header.setSeal(NonceField, h64{ 0 });
	header.setSeal(MixHashField, h256{ 0 });
	RLPStream ret;
	header.streamRLP(ret);
	if(m_onSealGenerated)
		m_onSealGenerated(ret.out());
}

void dev::bacd::Dpos::workLoop()
{
    
}

void dev::bacd::Dpos::init()
{
	ETH_REGISTER_SEAL_ENGINE(Dpos);
}

void dev::bacd::Dpos::initEnv(std::weak_ptr<DposHostcapality> _host)
{
	m_host = _host;
}

void dev::bacd::Dpos::onPoaMsg(NodeID _nodeid, unsigned _id, RLP const & _r)
{
	if(_id < DposPacketCount && _id >= DposStatuspacket)
	{
		cdebug << "onRaftMsg: id=" << _id << ",from=" << _nodeid;
		m_msg_queue.push(DposMsgPacket(_nodeid, _id, _r[0].data()));

	}
	else
	{
		cwarn << "Recv an illegal msg, id=" << _id << "  and the max_id:" << DposPacketCount - 1;
	}
}

void dev::bacd::Dpos::requestStatus(NodeID const & /*_nodeID*/, u256 const & /*_peerCapabilityVersion*/)
{

}

void dev::bacd::Dpos::brocastMsg(DposPacketType _type, RLPStream & _msg_s)
{
	// �㲥����  �� sealAndSend() �ص�ʹ��
	auto h = m_host.lock();
	h->hostFace()->foreachPeer(h->name(),
		[&](NodeID const& _nodeId){
		sealAndSend(_nodeId, _type, _msg_s);
		cdebug << "brocastMsg ... NodeId:" << _nodeId << "type" << _type;
		return true;
	});
}

void dev::bacd::Dpos::sealAndSend(NodeID const & _nodeid, DposPacketType _type, RLPStream const & _msg_s)
{
	// ��װ����
	RLPStream msg_ts; //��������ݰ� ���Ὣҵ���߼�׷�Ӱ���װ�����
	auto h = m_host.lock(); //weak_ptr ��Ҫ lock��)����Ϊ share_ptr���ܵ���
	h->hostFace()->prep(_nodeid, name(), msg_ts, _type, 1).append(_msg_s.out()); // ��װ��ϢID���ȵ�
	h->hostFace()->sealAndSend(_nodeid, msg_ts);
	cdebug << " Poa Send[" << _type << "] to " << _nodeid;
}

void dev::bacd::Dpos::updateDposData(EDposDataType _type, Address const & _addr_source, Address const & _addr_target)
{
    //dpos ѡ����/��֤�� ����
	Debug << "updateDposData|" << "type:" << _type << " _addr_source:" << _addr_source << " _addr_target" << _addr_target;
	switch(_type)
	{
	case dev::bacd::e_loginCandidate:
		break;
	case dev::bacd::e_logoutCandidate:
		break;
	case dev::bacd::e_delegate:
		break;
	case dev::bacd::e_unDelegate:
		break;
	case dev::bacd::e_max:
		break;
	default:
		break;
	}
}

bool dev::bacd::Dpos::isBolckSeal(uint64_t _last_time, uint64_t _now)
{
	Debug << "isBolckSeal: _last_time:" << _last_time << "||_now:" << _now;
	if(!CheckValidator(_now))
		return false;
	if(_last_time == 0)
		return true;
	if(tryElect(_last_time, _now))
		return false; //������һ��
	return true;
}

bool dev::bacd::Dpos::checkDeadline(uint64_t _last_block_time, uint64_t _now)
{  
	if(m_curr_valitors.empty())
		return false;
    if(_last_block_time <=0)
	{
	    //���Ϊ0 �����Ǵ�������
		return false;
	}
	//�õ�ÿ�γ��������ʱ��̶ȣ��Ƚ��ϴΣ����ں��´�
	// int64((now+blockInterval-1)/blockInterval) * blockInterval
	int next_slot = (_now + blockInterval - 1) / blockInterval * blockInterval;
    //nt64((now-1)/blockInterval) * blockInterval
	int prve_slot = (_now - 1) / blockInterval * blockInterval;
	int last_slot = (_last_block_time - 1) / blockInterval * blockInterval;

    if(last_slot >= next_slot)
	{
		cwarn << "waring :" << "last_slot:"<<last_slot<<" >= next_slot:"<<next_slot;
		return false;
	}
	if(last_slot == prve_slot || (next_slot - _now) <= 1)
		return true;
	return false;
}

bool dev::bacd::Dpos::CheckValidator(uint64_t _now)
{
	if(m_curr_valitors.empty())
		return false;

	int offet = _now % epochInterval;       // ��ǰ�� �����˶�ʱ��
	if(offet % blockInterval != 0)
		return false;   //��ʱ���ڳ������ʱ�� ����

	offet /= blockInterval;
	offet %= m_curr_valitors.size();
	Address const& curr_valitor = m_curr_valitors[offet]; //�õ�����֤��
	if(m_curr_valitor == curr_valitor)
		return true;
	return false;
}

bool dev::bacd::Dpos::tryElect(uint64_t _last_time, uint64_t _now)
{
    //���� ��֤���Ѿ�ͨ�� ����ͳ��ͶƱ������һ�� 
    //ʧ�� ���������
	unsigned int prveslot = _last_time / epochInterval; //��һ���������
	unsigned int currslot = _now / epochInterval;   //��ǰ�������������
	cdebug << "prveslot:" prveslot << " |currslot" << currslot;
    if(prveslot == currslot)
	{
	    //������ͬ���� ����
		return false;
	}
    // ����������ϸ���֤��
	kickoutValidator(prveslot);
    //ͳ��ͶƱ
	countVotes();
    //������֤��˳��
	disorganizeVotes();
    //�㲥ͬ����һ�ֵ���֤��
    //brocastMsg(DposDataPacket, )
	return true;
}

void dev::bacd::Dpos::kickoutValidator(unsigned int _prveslot)
{
    //�޳����ϸ�
	cdebug << "kickoutValidator:" << _prveslot;
    //��һ���ִ��� ����
	if(m_prveslot == 0 && m_canditate.size() <=maxValitorNum)
		return;
	unsigned int kickout_num = m_canditate.size() - maxValitorNum; // �����֤������


}
