#include "mapper/remapmemorybank.h"

#include "mapper/memorymapper.h"

RemappingMemoryBank::RemappingMemoryBank(MemoryMapper& mapper, size_t addr, size_t size) :
	mapper(mapper),
	addr(addr),
	size(size) {
}

uint8_t RemappingMemoryBank::getValue(size_t addr) {
	return mapper.getValue(this->addr + addr);
}

void RemappingMemoryBank::setValue(size_t addr, uint8_t value) {
	mapper.setValue(this->addr + addr, value);
}

uint16_t RemappingMemoryBank::getSize() {
	return size >> 8;
}

void RemappingMemoryBank::emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest) {
	mapper.emitLoad(a, this->addr + addr, dest);
}

void RemappingMemoryBank::emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src) {
	mapper.emitStore(a, this->addr + addr, src);
}


