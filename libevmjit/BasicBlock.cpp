
#include "BasicBlock.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

#include "Type.h"

namespace dev
{
namespace eth
{
namespace jit
{

const char* BasicBlock::NamePrefix = "Instr.";

BasicBlock::BasicBlock(ProgramCounter _beginInstIdx, ProgramCounter _endInstIdx, llvm::Function* _mainFunc) :
	m_beginInstIdx(_beginInstIdx),
	m_endInstIdx(_endInstIdx),
	m_llvmBB(llvm::BasicBlock::Create(_mainFunc->getContext(), {NamePrefix, std::to_string(_beginInstIdx)}, _mainFunc)),
	m_stack(m_llvmBB)
{}

BasicBlock::BasicBlock(std::string _name, llvm::Function* _mainFunc) :
	m_beginInstIdx(0),
	m_endInstIdx(0),
	m_llvmBB(llvm::BasicBlock::Create(_mainFunc->getContext(), _name, _mainFunc)),
	m_stack(m_llvmBB)
{}


void BasicBlock::LocalStack::push(llvm::Value* _value)
{
	assert(_value->getType() == Type::i256);
	m_backend.push_back(_value);
}

llvm::Value* BasicBlock::LocalStack::pop()
{
	auto top = get(0);
	m_backend.pop_back();
	return top;
}

llvm::Value* BasicBlock::LocalStack::get(size_t _index)
{	
	if (_index >= m_backend.size())
	{
		// Create PHI node for missing values
		auto nMissingVals = _index - m_backend.size() + 1;
		m_backend.insert(m_backend.begin(), nMissingVals, nullptr);
		for (decltype(nMissingVals) i = 0; i < nMissingVals; ++i)
		{
			m_backend[nMissingVals - 1 - i] = m_llvmBB->empty() ?
				llvm::PHINode::Create(Type::i256, 0, {}, m_llvmBB) :
				llvm::PHINode::Create(Type::i256, 0, {}, m_llvmBB->getFirstNonPHI());
			m_numRequiredStackItems += 1;
		}
	}

	return *(m_backend.rbegin() + _index);
}

void BasicBlock::LocalStack::dup(size_t _index)
{
	m_backend.push_back(get(_index));
}

void BasicBlock::LocalStack::swap(size_t _index)
{
	assert(_index != 0);
	get(_index); // Create PHI nodes
	std::swap(*m_backend.rbegin(), *(m_backend.rbegin() + _index));
}

}
}
}

