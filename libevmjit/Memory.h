#pragma once

#include <libdevcore/Common.h>

#include "CompilerHelper.h"

namespace dev
{
namespace eth
{
namespace jit
{

class Memory : public CompilerHelper
{
public:
	Memory(llvm::IRBuilder<>& _builder, class GasMeter& _gasMeter);

	llvm::Value* loadWord(llvm::Value* _addr);
	void storeWord(llvm::Value* _addr, llvm::Value* _word);
	void storeByte(llvm::Value* _addr, llvm::Value* _byte);
	llvm::Value* getData();
	llvm::Value* getSize();
	void copyBytes(llvm::Value* _srcPtr, llvm::Value* _srcSize, llvm::Value* _srcIndex,
	               llvm::Value* _destMemIdx, llvm::Value* _byteCount);

	/// Requires this amount of memory. And counts gas fee for that memory.
	void require(llvm::Value* _size);

	/// Requires the amount of memory to for data defined by offset and size. And counts gas fee for that memory.
	void require(llvm::Value* _offset, llvm::Value* _size);

	void registerReturnData(llvm::Value* _index, llvm::Value* _size);
	static bytesConstRef getReturnData();

	void dump(uint64_t _begin, uint64_t _end = 0);

private:
	llvm::Function* createFunc(bool _isStore, llvm::Type* _type, GasMeter& _gasMeter);
	llvm::Function* createRequireFunc(GasMeter& _gasMeter);

private:
	llvm::GlobalVariable* m_data;
	llvm::GlobalVariable* m_size;

	/// @TODO: m_data and m_size could be used
	llvm::GlobalVariable* m_returnDataOffset;
	llvm::GlobalVariable* m_returnDataSize;

	llvm::Function* m_resize;
	llvm::Function* m_require;
	llvm::Function* m_loadWord;
	llvm::Function* m_storeWord;
	llvm::Function* m_storeByte;

	llvm::Function* m_memDump;
};

}
}
}

