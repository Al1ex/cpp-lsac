
#include "VM.h"

#include <libevm/VM.h>

#include "ExecutionEngine.h"
#include "Compiler.h"

namespace dev
{
namespace eth
{
namespace jit
{

bytes VM::go(ExtVMFace& _ext)
{
	auto module = Compiler().compile(_ext.code);
	module->dump();

	ExecutionEngine engine;
	auto exitCode = engine.run(std::move(module));

	switch (exitCode)
	{
	case 101:
		BOOST_THROW_EXCEPTION(BadJumpDestination());
	case 102:
		BOOST_THROW_EXCEPTION(OutOfGas());
	}

	return std::move(engine.returnData);
}

}
}
}
