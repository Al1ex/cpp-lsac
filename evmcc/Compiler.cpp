
#include "Compiler.h"

#include <llvm/IR/IRBuilder.h>

#include <libevmface/Instruction.h>

#include "Memory.h"
#include "Stack.h"
#include "Ext.h"

namespace evmcc
{

struct
{
	llvm::Type* word8;
	llvm::Type* word8ptr;
	llvm::Type* word256;
	llvm::Type* word256ptr;
	llvm::Type* word256arr;
	llvm::Type* size;
	llvm::Type* Void;
	llvm::Type* WordLowPrecision;
} Types;

Compiler::Compiler()
{
	auto& context = llvm::getGlobalContext();
	Types.word8 = llvm::Type::getInt8Ty(context);
	Types.word8ptr = llvm::Type::getInt8PtrTy(context);
	Types.word256 = llvm::Type::getIntNTy(context, 256);
	Types.word256ptr = Types.word256->getPointerTo();
	Types.word256arr = llvm::ArrayType::get(Types.word256, 100);
	Types.size = llvm::Type::getInt64Ty(context);
	Types.Void = llvm::Type::getVoidTy(context);

	// TODO: Use 64-bit for now. In 128-bit compiler-rt library functions are required
	Types.WordLowPrecision = llvm::Type::getIntNTy(context, 64);
}

llvm::BasicBlock* Compiler::getOrCreateBasicBlockAtPC(ProgramCounter pc)
{
	llvm::BasicBlock* block = nullptr;
	auto blockIter = basicBlocks.find(pc);
	if (blockIter == basicBlocks.cend())
	{
		// Create a basic block at targetPC.
		std::ostringstream oss;
		oss << "instr." << pc;
		block = llvm::BasicBlock::Create(llvm::getGlobalContext(), oss.str());
		basicBlocks[pc] = block;
	}
	else
	{
		block = blockIter->second;
	}
	return block;
}

void Compiler::createBasicBlocks(const dev::bytes& bytecode)
{
	getOrCreateBasicBlockAtPC(0);

	for (auto curr = bytecode.cbegin(); curr != bytecode.cend(); ++curr)
	{
		using dev::eth::Instruction;

		auto inst = static_cast<Instruction>(*curr);
		switch (inst)
		{
		case Instruction::PUSH1:
		case Instruction::PUSH2:
		case Instruction::PUSH3:
		case Instruction::PUSH4:
		case Instruction::PUSH5:
		case Instruction::PUSH6:
		case Instruction::PUSH7:
		case Instruction::PUSH8:
		case Instruction::PUSH9:
		case Instruction::PUSH10:
		case Instruction::PUSH11:
		case Instruction::PUSH12:
		case Instruction::PUSH13:
		case Instruction::PUSH14:
		case Instruction::PUSH15:
		case Instruction::PUSH16:
		case Instruction::PUSH17:
		case Instruction::PUSH18:
		case Instruction::PUSH19:
		case Instruction::PUSH20:
		case Instruction::PUSH21:
		case Instruction::PUSH22:
		case Instruction::PUSH23:
		case Instruction::PUSH24:
		case Instruction::PUSH25:
		case Instruction::PUSH26:
		case Instruction::PUSH27:
		case Instruction::PUSH28:
		case Instruction::PUSH29:
		case Instruction::PUSH30:
		case Instruction::PUSH31:
		case Instruction::PUSH32:
		{
			auto numBytes = static_cast<size_t>(inst) - static_cast<size_t>(Instruction::PUSH1) + 1;
			auto next = curr + numBytes + 1;
			if (next == bytecode.cend())
				break;

			auto nextInst = static_cast<Instruction>(*next);

			if (nextInst == Instruction::JUMP || nextInst == Instruction::JUMPI)
			{
				// Compute target PC of the jump.
				dev::u256 val = 0;
				for (auto iter = curr + 1; iter < next; ++iter)
				{
					val <<= 8;
					val |= *iter;
				}

				// Create a block for the JUMP target.
				ProgramCounter targetPC = val.convert_to<ProgramCounter>();
				auto targetBlock = getOrCreateBasicBlockAtPC(targetPC);

				ProgramCounter jumpPC = (next - bytecode.cbegin());
				jumpTargets[jumpPC] = targetBlock;

				// Create a block following the JUMP.
				if (next + 1 < bytecode.cend())
				{
					ProgramCounter nextPC = (next + 1 - bytecode.cbegin());
					getOrCreateBasicBlockAtPC(nextPC);
				}

				curr += 1; // skip over JUMP
			}

			curr += numBytes;
			break;
		}

		case Instruction::JUMP:
		case Instruction::JUMPI:
		{
			std::cerr << "JUMP/JUMPI at " << (curr - bytecode.cbegin()) << " not preceded by PUSH\n";
			std::exit(1);
		}

		default:
			break;
		}
	}
}

std::unique_ptr<llvm::Module> Compiler::compile(const dev::bytes& bytecode)
{
	using namespace llvm;

	auto& context = getGlobalContext();
	auto module = std::make_unique<Module>("main", context);
	IRBuilder<> builder(context);

	// Create main function
	const auto i32Ty = builder.getInt32Ty();
	Type* retTypeElems[] = {i32Ty, i32Ty};
	auto retType = StructType::create(retTypeElems, "MemRef", true);
	auto mainFuncType = FunctionType::get(builder.getInt64Ty(), false);
	auto mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage, "main", module.get());

	// Create the basic blocks.
	auto entryBlock = BasicBlock::Create(context, "entry", mainFunc);
	builder.SetInsertPoint(entryBlock);
	createBasicBlocks(bytecode);

	// Init runtime structures.
	auto stack = Stack(builder, module.get());
	auto memory = Memory(builder, module.get());
	auto ext = Ext(builder, module.get());

	auto userRet = false;
	auto finished = false;

	BasicBlock* currentBlock = entryBlock;

	for (auto pc = bytecode.cbegin(); pc != bytecode.cend() && !finished; ++pc)
	{
		using dev::eth::Instruction;

		ProgramCounter currentPC = pc - bytecode.cbegin();

		auto blockIter = basicBlocks.find(currentPC);
		if (blockIter != basicBlocks.end())
		{
			auto nextBlock = blockIter->second;
			// Terminate the current block by jumping to the next one.
			if (currentBlock != nullptr)
				builder.CreateBr(nextBlock);
			// Insert the next block into the main function.
			mainFunc->getBasicBlockList().push_back(nextBlock);
			builder.SetInsertPoint(nextBlock);
			currentBlock = nextBlock;
		}

		assert(currentBlock != nullptr);

		auto inst = static_cast<Instruction>(*pc);
		switch (inst)
		{

		case Instruction::ADD:
		{
			auto lhs = stack.pop();
			auto rhs = stack.pop();
			auto result = builder.CreateAdd(lhs, rhs);
			stack.push(result);
			break;
		}

		case Instruction::SUB:
		{
			auto lhs = stack.pop();
			auto rhs = stack.pop();
			auto result = builder.CreateSub(lhs, rhs);
			stack.push(result);
			break;
		}

		case Instruction::MUL:
		{
			auto lhs256 = stack.pop();
			auto rhs256 = stack.pop();
			auto lhs128 = builder.CreateTrunc(lhs256, Types.WordLowPrecision);
			auto rhs128 = builder.CreateTrunc(rhs256, Types.WordLowPrecision);
			auto res128 = builder.CreateMul(lhs128, rhs128);
			auto res256 = builder.CreateZExt(res128, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::DIV:
		{
			auto lhs256 = stack.pop();
			auto rhs256 = stack.pop();
			auto lhs128 = builder.CreateTrunc(lhs256, Types.WordLowPrecision);
			auto rhs128 = builder.CreateTrunc(rhs256, Types.WordLowPrecision);
			auto res128 = builder.CreateUDiv(lhs128, rhs128);
			auto res256 = builder.CreateZExt(res128, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::SDIV:
		{
			auto lhs256 = stack.pop();
			auto rhs256 = stack.pop();
			auto lhs128 = builder.CreateTrunc(lhs256, Types.WordLowPrecision);
			auto rhs128 = builder.CreateTrunc(rhs256, Types.WordLowPrecision);
			auto res128 = builder.CreateSDiv(lhs128, rhs128);
			auto res256 = builder.CreateSExt(res128, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::MOD:
		{
			auto lhs256 = stack.pop();
			auto rhs256 = stack.pop();
			auto lhs128 = builder.CreateTrunc(lhs256, Types.WordLowPrecision);
			auto rhs128 = builder.CreateTrunc(rhs256, Types.WordLowPrecision);
			auto res128 = builder.CreateURem(lhs128, rhs128);
			auto res256 = builder.CreateZExt(res128, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::SMOD:
		{
			auto lhs256 = stack.pop();
			auto rhs256 = stack.pop();
			auto lhs128 = builder.CreateTrunc(lhs256, Types.WordLowPrecision);
			auto rhs128 = builder.CreateTrunc(rhs256, Types.WordLowPrecision);
			auto res128 = builder.CreateSRem(lhs128, rhs128);
			auto res256 = builder.CreateSExt(res128, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::LT:
		{
			auto lhs = stack.pop();
			auto rhs = stack.pop();
			auto res1 = builder.CreateICmpULT(lhs, rhs);
			auto res256 = builder.CreateZExt(res1, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::GT:
		{
			auto lhs = stack.pop();
			auto rhs = stack.pop();
			auto res1 = builder.CreateICmpUGT(lhs, rhs);
			auto res256 = builder.CreateZExt(res1, Types.word256);
			stack.push(res256);
			break;
		}

		case Instruction::NOT:
		{
			auto top = stack.pop();
			auto zero = ConstantInt::get(Types.word256, 0);
			auto iszero = builder.CreateICmpEQ(top, zero, "iszero");
			auto result = builder.CreateZExt(iszero, Types.word256);
			stack.push(result);
			break;
		}

		case Instruction::POP:
		{
			stack.pop();
			break;
		}

		case Instruction::PUSH1:
		case Instruction::PUSH2:
		case Instruction::PUSH3:
		case Instruction::PUSH4:
		case Instruction::PUSH5:
		case Instruction::PUSH6:
		case Instruction::PUSH7:
		case Instruction::PUSH8:
		case Instruction::PUSH9:
		case Instruction::PUSH10:
		case Instruction::PUSH11:
		case Instruction::PUSH12:
		case Instruction::PUSH13:
		case Instruction::PUSH14:
		case Instruction::PUSH15:
		case Instruction::PUSH16:
		case Instruction::PUSH17:
		case Instruction::PUSH18:
		case Instruction::PUSH19:
		case Instruction::PUSH20:
		case Instruction::PUSH21:
		case Instruction::PUSH22:
		case Instruction::PUSH23:
		case Instruction::PUSH24:
		case Instruction::PUSH25:
		case Instruction::PUSH26:
		case Instruction::PUSH27:
		case Instruction::PUSH28:
		case Instruction::PUSH29:
		case Instruction::PUSH30:
		case Instruction::PUSH31:
		case Instruction::PUSH32:
		{
			auto numBytes = static_cast<size_t>(inst) - static_cast<size_t>(Instruction::PUSH1) + 1;
			auto value = llvm::APInt(256, 0);
			for (decltype(numBytes) i = 0; i < numBytes; ++i)	// TODO: Use pc as iterator
			{
				++pc;
				value <<= 8;
				value |= *pc;
			}
			auto c = builder.getInt(value);
			stack.push(c);
			break;
		}

		case Instruction::DUP1:
		case Instruction::DUP2:
		case Instruction::DUP3:
		case Instruction::DUP4:
		case Instruction::DUP5:
		case Instruction::DUP6:
		case Instruction::DUP7:
		case Instruction::DUP8:
		case Instruction::DUP9:
		case Instruction::DUP10:
		case Instruction::DUP11:
		case Instruction::DUP12:
		case Instruction::DUP13:
		case Instruction::DUP14:
		case Instruction::DUP15:
		case Instruction::DUP16:
		{
			auto index = static_cast<uint32_t>(inst) - static_cast<uint32_t>(Instruction::DUP1);
			auto value = stack.get(index);
			stack.push(value);
			break;
		}

		case Instruction::SWAP1:
		case Instruction::SWAP2:
		case Instruction::SWAP3:
		case Instruction::SWAP4:
		case Instruction::SWAP5:
		case Instruction::SWAP6:
		case Instruction::SWAP7:
		case Instruction::SWAP8:
		case Instruction::SWAP9:
		case Instruction::SWAP10:
		case Instruction::SWAP11:
		case Instruction::SWAP12:
		case Instruction::SWAP13:
		case Instruction::SWAP14:
		case Instruction::SWAP15:
		case Instruction::SWAP16:
		{
			auto index = static_cast<uint32_t>(inst) - static_cast<uint32_t>(Instruction::SWAP1) + 1;
			auto loValue = stack.get(index);
			auto hiValue = stack.get(0);
			stack.set(index, hiValue);
			stack.set(0, loValue);
			break;
		}

		case Instruction::MLOAD:
		{
			auto addr = stack.pop();
			auto word = memory.loadWord(addr);
			stack.push(word);
			break;
		}

		case Instruction::MSTORE:
		{
			auto addr = stack.pop();
			auto word = stack.pop();
			memory.storeWord(addr, word);
			break;
		}

		case Instruction::MSTORE8:
		{
			auto addr = stack.pop();
			auto word = stack.pop();
			memory.storeByte(addr, word);
			break;
		}

		case Instruction::MSIZE:
		{
			auto word = memory.getSize();
			stack.push(word);
			break;
		}

		case Instruction::SLOAD:
		{
			auto index = stack.pop();
			auto value = ext.store(index);
			stack.push(value);
			break;
		}

		case Instruction::SSTORE:
		{
			auto index = stack.pop();
			auto value = stack.pop();
			ext.setStore(index, value);
			break;
		}

		case Instruction::JUMP:
		{
			// The target address is computed at compile time,
			// just pop it without looking...
			stack.pop();

			auto targetBlock = jumpTargets[currentPC];
			builder.CreateBr(targetBlock);

			currentBlock = nullptr;
			break;
		}

		case Instruction::JUMPI:
		{
			assert(pc + 1 < bytecode.cend());

			// The target address is computed at compile time,
			// just pop it without looking...
			stack.pop();

			auto top = stack.pop();
			auto zero = ConstantInt::get(Types.word256, 0);
			auto cond = builder.CreateICmpNE(top, zero, "nonzero");
			auto targetBlock = jumpTargets[currentPC];
			auto followBlock = basicBlocks[currentPC + 1];
			builder.CreateCondBr(cond, targetBlock, followBlock);

			currentBlock = nullptr;
			break;
		}

		case Instruction::PC:
		{
			auto value = builder.getIntN(256, currentPC);
			stack.push(value);
			break;
		}

		case Instruction::ADDRESS:
		{
			auto value = ext.address();
			stack.push(value);
			break;
		}

		case Instruction::BALANCE:
		{
			auto address = stack.pop();
			auto value = ext.balance(address);
			stack.push(value);
			break;
		}

		case Instruction::CALLER:
		{
			auto value = ext.caller();
			stack.push(value);
			break;
		}

		case Instruction::ORIGIN:
		{
			auto value = ext.origin();
			stack.push(value);
			break;
		}

		case Instruction::CALLVALUE:
		{
			auto value = ext.callvalue();
			stack.push(value);
			break;
		}

		case Instruction::CALLDATASIZE:
		{
			auto value = ext.calldatasize();
			stack.push(value);
			break;
		}

		case Instruction::CALLDATALOAD:
		{
			auto index = stack.pop();
			auto value = ext.calldataload(index);
			stack.push(value);
			break;
		}

		case Instruction::GASPRICE:
		{
			auto value = ext.gasprice();
			stack.push(value);
			break;
		}

		case Instruction::CODESIZE:
		{
			auto value = builder.getIntN(256, bytecode.size());
			stack.push(value);
			break;
		}

		case Instruction::PREVHASH:
		{
			auto value = ext.prevhash();
			stack.push(value);
			break;
		}

		case Instruction::COINBASE:
		{
			auto value = ext.coinbase();
			stack.push(value);
			break;
		}

		case Instruction::TIMESTAMP:
		{
			auto value = ext.timestamp();
			stack.push(value);
			break;
		}

		case Instruction::NUMBER:
		{
			auto value = ext.number();
			stack.push(value);
			break;
		}

		case Instruction::DIFFICULTY:
		{
			auto value = ext.difficulty();
			stack.push(value);
			break;
		}

		case Instruction::GASLIMIT:
		{
			auto value = ext.gaslimit();
			stack.push(value);
			break;
		}

		case Instruction::RETURN:
		{
			auto index = stack.pop();
			auto size = stack.pop();

			// MCJIT does not support returning structs
			//auto index32 = builder.CreateTrunc(index, i32Ty, "index32");
			//auto size32 = builder.CreateTrunc(size, i32Ty, "size32");
			//auto ret = builder.CreateInsertValue(UndefValue::get(retType), index32, 0, "ret");
			//ret = builder.CreateInsertValue(ret, size32, 1, "ret");

			auto ret = builder.CreateTrunc(index, builder.getInt64Ty());
			ret = builder.CreateShl(ret, 32);
			size = builder.CreateTrunc(size, i32Ty);
			size = builder.CreateZExt(size, builder.getInt64Ty());
			ret = builder.CreateOr(ret, size);

			builder.CreateRet(ret);
			finished = true;
			userRet = true;
			break;
		}

		case Instruction::STOP:
		{
			finished = true;
			break;
		}

		}
	}

	// Generate final basic block (may be jumped to).
	auto finalPC = bytecode.size();
	auto it = basicBlocks.find(finalPC);
	if (it != basicBlocks.end())
	{
		auto finalBlock = it->second;

		if (currentBlock != nullptr)
			builder.CreateBr(finalBlock);

		mainFunc->getBasicBlockList().push_back(finalBlock);
		builder.SetInsertPoint(finalBlock);
	}

	if (!userRet)
		builder.CreateRet(builder.getInt64(0));

	return module;
}

}
