#include <asmjit/asmjit.h>
#include <stdio.h>
#include <iostream>
#include <fmt/format.h>
#include "instruction.h"
#include "ines.h"

template<class T, class U>
std::unique_ptr<T> unique_static_cast(std::unique_ptr<U>& ptr) {
	return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
}

// Signature of the generated function.
typedef void (*Func)(void);

struct Context {
	INes& game;
	MemoryMapper& mapper;
	asmjit::JitRuntime& rt;
} *context;

extern "C" void outer_jit_wrapper(uint16_t target);

extern "C" uint64_t jit(uint16_t target) {
	fmt::print("Jitting block starting at {:X}\n", target);

	asmjit::StringLogger logger;
	logger.addOptions(asmjit::Logger::kOptionHexImmediate);
	asmjit::CodeHolder code;                       // Holds code and relocation information.
	code.init(context->rt.getCodeInfo());          // Initialize to the same arch as JIT runtime.
	code.setLogger(&logger);

	asmjit::X86Assembler a(&code);                 // Create and attach X86Assembler to `code`.

	ParserPointer pp(context->mapper, target);
	bool cont = true;

	while(cont) {
		uint8_t b = pp.next();
		auto ic = opcodeTable[b];
		if(ic == nullptr) {
			fmt::print("Unknown opcode 0x{0:X} ({0}) at location {1:X}, ABORT\n", b, pp.getLocation());
			return 0;
		}
		auto i = ic(pp);
		a.comment(fmt::format("; {}", i->format()).c_str());
		cont = i->exp(a, context->mapper);
	}
	fmt::print("\nGenerated code\n");
	fmt::print("{}\n", logger.getString());

	Func fn;
	asmjit::Error err = context->rt.add(&fn, &code);
	if (err) {
		fmt::print("Failed adding the code to the runtime");
		return 0;
	}

	// @COMPLETENESS: Ideally we would also save the jit result to make sure we
	// don't have to do it again.
	// @LEAK: We are leaking the function here. We should save that somewhere
	// and free it when we exit
	return (uint64_t)fn;
}

int main(int argc, char* argv[]) {
	asmjit::JitRuntime rt;                         // Runtime specialized for JIT code execution.

	asmjit::CodeHolder code;                       // Holds code and relocation information.
	code.init(rt.getCodeInfo());                   // Initialize to the same arch as JIT runtime.

	asmjit::X86Assembler a(&code);                 // Create and attach X86Assembler to `code`.

	INes f("game.nes");

	Context con{
		f,
		f.getMapper(),
		rt
	};
	context = &con;

	//Compile starting at the progstart location
	outer_jit_wrapper(0xC000);

	return 0;
}
