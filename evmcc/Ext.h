
#pragma once

#include <llvm/IR/IRBuilder.h>

#include <libevm/ExtVMFace.h>

namespace evmcc
{



class Ext
{
public:
	Ext(llvm::IRBuilder<>& _builder, llvm::Module* module);
	static void init(std::unique_ptr<dev::eth::ExtVMFace> _ext);

	llvm::Value* store(llvm::Value* _index);
	void setStore(llvm::Value* _index, llvm::Value* _value);

	llvm::Value* address();
	llvm::Value* caller();
	llvm::Value* origin();
	llvm::Value* callvalue();
	llvm::Value* calldatasize();
	llvm::Value* gasprice();
	llvm::Value* prevhash();
	llvm::Value* coinbase();
	llvm::Value* timestamp();
	llvm::Value* number();
	llvm::Value* difficulty();
	llvm::Value* gaslimit();

	llvm::Value* balance(llvm::Value* _address);
	void suicide(llvm::Value* _address);
	llvm::Value* calldataload(llvm::Value* _index);
	llvm::Value* create(llvm::Value* _endowment, llvm::Value* _initOff, llvm::Value* _initSize);
	llvm::Value* call(llvm::Value* _gas, llvm::Value* _receiveAddress, llvm::Value* _value, llvm::Value* _inOff, llvm::Value* _inSize, llvm::Value* _outOff, llvm::Value* _outSize);

	llvm::Value* sha3(llvm::Value* _inOff, llvm::Value* _inSize);

private:
	llvm::Value* getDataElem(unsigned _index, const llvm::Twine& _name = "");

	llvm::Value* bswap(llvm::Value*);

private:
	llvm::IRBuilder<>& m_builder;

	llvm::Value* m_args[2];
	llvm::Value* m_arg2;
	llvm::Value* m_arg3;
	llvm::Value* m_arg4;
	llvm::Value* m_arg5;
	llvm::Value* m_arg6;
	llvm::Value* m_arg7;
	llvm::Value* m_data;
	llvm::Function* m_init;
	llvm::Function* m_store;
	llvm::Function* m_setStore;
	llvm::Function* m_calldataload;
	llvm::Function* m_balance;
	llvm::Function* m_suicide;
	llvm::Function* m_create;
	llvm::Function* m_call;
	llvm::Function* m_bswap;
	llvm::Function* m_sha3;
};
	

}
