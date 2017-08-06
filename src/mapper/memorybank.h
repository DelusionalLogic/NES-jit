#pragma once

#include <asmjit/asmjit.h>
#include <fmt/format.h>

class MemoryBank {
	protected:
	public:
		// @CLEANUP: This should really take a uint16_t
		virtual uint8_t getValue(size_t addr) = 0;
		virtual void setValue(size_t addr, uint8_t value) = 0;

		virtual void emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest) = 0;
		virtual void emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src) = 0;

		virtual uint16_t getSize() = 0;

		virtual ~MemoryBank() {};

};

