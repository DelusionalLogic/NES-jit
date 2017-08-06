#include "mapper/rammemorybank.h"

#include <fmt/format.h>

RamMemoryBank::RamMemoryBank(size_t size) : memory(std::unique_ptr<char>(new char[size])), size(size) {
}

uint8_t RamMemoryBank::getValue(size_t addr) {
	return memory.get()[addr];
}

void RamMemoryBank::setValue(size_t addr, uint8_t value) {
	memory.get()[addr] = value;
}

uint16_t RamMemoryBank::getSize() {
	return size >> 8;
}

void RamMemoryBank::emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest) {
	auto temp = asmjit::x86::rax;
	a.mov(temp, (uint64_t)this->memory.get() + addr);
	a.mov(dest, asmjit::x86::byte_ptr(temp));
}

void RamMemoryBank::emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src) {
	auto temp = asmjit::x86::rax;
	a.mov(temp, (uint64_t)this->memory.get() + addr);
	a.mov(asmjit::x86::byte_ptr(temp), src);
}
