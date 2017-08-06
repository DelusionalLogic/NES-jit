#pragma once

#include <memory>
#include <stdint.h>

#include <asmjit/asmjit.h>
#include <fmt/format.h>

#include "mapper/memorybank.h"

class MemoryMapper {
	private:
		std::shared_ptr<MemoryBank> pageTable[0xFF];

		std::shared_ptr<MemoryBank> getBank(uint16_t addr, uint16_t& relAddr);
	public:
		void setBank(uint8_t page, std::shared_ptr<MemoryBank> bank);
		uint8_t getValue(size_t addr);
		void setValue(size_t addr, uint8_t value);

		void emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest);
		void emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src);

		void emitDynamicLoad(asmjit::X86Assembler& a, asmjit::X86Gp addr, asmjit::X86Gp dest);
		template<class T> void emitDynamicStore(asmjit::X86Assembler& a, asmjit::X86Gp addr, T src);
};
