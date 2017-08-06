#include "mapper/filememorybank.h"

#include <fstream>

ReadingMemoryBank::ReadingMemoryBank(std::ifstream &fs, size_t size) :
	m_memory(std::shared_ptr<char>(new char[size])),
	size(size) {
	//@SPEED: This is slow and could be replaced with a mmap
	fs.read(m_memory.get(), size);
}

uint8_t ReadingMemoryBank::getValue(size_t addr) {
	return m_memory.get()[addr];
}

void ReadingMemoryBank::setValue(size_t addr, uint8_t value) {
	m_memory.get()[addr] = value;
}

uint16_t ReadingMemoryBank::getSize() {
	return size >> 8;
}

void ReadingMemoryBank::emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest) {
	auto temp = asmjit::x86::rax;
	a.mov(temp, (uint64_t)this->m_memory.get() + addr);
	a.mov(dest, asmjit::x86::byte_ptr(temp));
}

void ReadingMemoryBank::emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src) {
	auto temp = asmjit::x86::rax;
	a.mov(temp, (uint64_t)this->m_memory.get() + addr);
	a.mov(asmjit::x86::byte_ptr(temp), src);
}
