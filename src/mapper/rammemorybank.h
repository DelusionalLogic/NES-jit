#pragma once

#include <memory>

#include "mapper/memorybank.h"

class RamMemoryBank : public MemoryBank {
	private:
		std::unique_ptr<char> memory;
		size_t size;

	public:
		RamMemoryBank(size_t size);

		uint16_t getSize();

		uint8_t getValue(size_t addr);
		void setValue(size_t addr, uint8_t value);

		virtual void emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest);
		virtual void emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest);
};
