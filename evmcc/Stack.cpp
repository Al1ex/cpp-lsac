
#include "Stack.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cassert>

#include <llvm/IR/Function.h>

#include "Utils.h"

#ifdef _MSC_VER
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT
#endif

namespace evmcc
{

using StackImpl = std::vector<i256>;


Stack::Stack(llvm::IRBuilder<>& _builder, llvm::Module* _module)
	: m_builder(_builder)
{
	// TODO: Clean up LLVM types
	auto stackPtrTy = m_builder.getInt8PtrTy();
	auto i256Ty = m_builder.getIntNTy(256);
	auto i256PtrTy = i256Ty->getPointerTo();
	auto voidTy = m_builder.getVoidTy();

	auto stackCreate = llvm::Function::Create(llvm::FunctionType::get(stackPtrTy, false),
		llvm::GlobalValue::LinkageTypes::ExternalLinkage, "evmccrt_stack_create", _module);

	llvm::Type* argsTypes[] = {stackPtrTy, i256PtrTy};
	auto funcType = llvm::FunctionType::get(voidTy, argsTypes, false);
	m_stackPush = llvm::Function::Create(funcType,
		llvm::GlobalValue::LinkageTypes::ExternalLinkage, "evmccrt_stack_push", _module);

	m_stackPop = llvm::Function::Create(funcType,
		llvm::GlobalValue::LinkageTypes::ExternalLinkage, "evmccrt_stack_pop", _module);

	llvm::Type* getArgsTypes[] = {stackPtrTy, m_builder.getInt32Ty(), i256PtrTy};
	auto getFuncType = llvm::FunctionType::get(voidTy, getArgsTypes, false);
	m_stackGet = llvm::Function::Create(getFuncType,
		llvm::GlobalValue::LinkageTypes::ExternalLinkage, "evmccrt_stack_get", _module);

	m_stackSet = llvm::Function::Create(getFuncType,
		llvm::GlobalValue::LinkageTypes::ExternalLinkage, "evmccrt_stack_set", _module);

	m_args[0] = m_builder.CreateCall(stackCreate, "stack.ptr");
	m_args[1] = m_builder.CreateAlloca(i256Ty, nullptr, "stack.val");
}


void Stack::push(llvm::Value* _value)
{
	m_builder.CreateStore(_value, m_args[1]);	  // copy value to memory
	m_builder.CreateCall(m_stackPush, m_args);
}


llvm::Value* Stack::pop()
{
	m_builder.CreateCall(m_stackPop, m_args);
	return m_builder.CreateLoad(m_args[1]);
}


llvm::Value* Stack::get(uint32_t _index)
{
	llvm::Value* args[] = {m_args[0], m_builder.getInt32(_index), m_args[1]};
	m_builder.CreateCall(m_stackGet, args);
	return m_builder.CreateLoad(m_args[1]);
}


void Stack::set(uint32_t _index, llvm::Value* _value)
{
	m_builder.CreateStore(_value, m_args[1]);	  // copy value to memory
	llvm::Value* args[] = {m_args[0], m_builder.getInt32(_index), m_args[1]};
	m_builder.CreateCall(m_stackSet, args);
}


llvm::Value* Stack::top()
{
	return get(0);
}


void debugStack(const char* _op, const i256& _word, uint32_t _index = 0)
{
	std::cerr << "STACK " << std::setw(4) << std::setfill(' ') << _op
			  << " [" << std::setw(2) << std::setfill('0') << _index << "] "
			  << std::dec << _word.a
			  << " HEX: " << std::hex;
	if (_word.b || _word.c || _word.d)
	{
		std::cerr << std::setw(16) << _word.d << " "
				  << std::setw(16) << _word.c << " "
				  << std::setw(16) << _word.b << " ";
	}
	std::cerr << std::setw(16) << _word.a << "\n";
}

}

extern "C"
{
	using namespace evmcc;

EXPORT void* evmccrt_stack_create()
{
	auto stack = new StackImpl;
	std::cerr << "STACK create\n";
	return stack;
}

EXPORT void evmccrt_stack_push(StackImpl* _stack, i256* _word)
{
	debugStack("push", *_word);
	_stack->push_back(*_word);
}

EXPORT void evmccrt_stack_pop(StackImpl* _stack, i256* _outWord)
{
	assert(!_stack->empty());
	auto word = &_stack->back();
	debugStack("pop", *word);
	_stack->pop_back();
	*_outWord = *word;
}

EXPORT void evmccrt_stack_get(StackImpl* _stack, uint32_t _index, i256* _outWord)
{
	assert(_index < _stack->size());
	auto word = _stack->rbegin() + _index;
	debugStack("get", *word, _index);
	*_outWord = *word;
}

EXPORT void evmccrt_stack_set(StackImpl* _stack, uint32_t _index, i256* _word)
{
	assert(_index < _stack->size());
	*(_stack->rbegin() + _index) = *_word;
	debugStack("set", *_word, _index);
}

}	// extern "C"
