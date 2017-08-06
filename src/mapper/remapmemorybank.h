#pragma once

#include <memory>

#include "mapper/memorybank.h"
#include "mapper/memorymapper.h"

class RemappingMemoryBank : public MemoryBank {
	private:
		MemoryMapper& mapper;
		size_t addr;
		size_t size;
	public:
		// Just a default for now that doesn't allocate anything
		RemappingMemoryBank(MemoryMapper& mapper, size_t addr, size_t size);
		uint16_t getSize();
		uint8_t getValue(size_t addr);
		void setValue(size_t addr, uint8_t value);
		void emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest);
		void emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src);
};

