#pragma once

#include <memory>
#include <iostream>

#include "mapper/memorybank.h"

class ReadingMemoryBank : public MemoryBank {
	private:
		std::shared_ptr<char> m_memory;
		size_t size;
	public:
		ReadingMemoryBank(std::ifstream &fs, size_t size);

		uint16_t getSize();

		uint8_t getValue(size_t addr);
		void setValue(size_t addr, uint8_t value);

		void emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest);
		void emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src);
};

