#include "mapper/memorymapper.h"

#include <fmt/format.h>

#define BANK_SIZE 0x4000

void MemoryMapper::setBank(uint8_t startPage, std::shared_ptr<MemoryBank> bank) {
	uint8_t endPage = startPage + bank->getSize()-1;
	fmt::print("Adding bank starting at 0x{:X} and ending at 0x{:X}\n", startPage, endPage);
	// Use a 16 bit variable to avoid overflow
	for(uint16_t i = startPage; i <= endPage; i++) {
		pageTable[i] = bank;
	}
}

std::shared_ptr<MemoryBank> MemoryMapper::getBank(uint16_t addr, uint16_t& relAddr) {
	// @HACK: Find the first page mapped to this bank. This should be
	// reconsidered
	uint8_t page = (addr >> 8);
	auto pageEntry = pageTable[page];
	for(uint16_t i = page; i > 0; i--) {
		auto thisEntry = pageTable[i-1];
		if(thisEntry != pageEntry) {
			page = i;
			break;
		}
	}
	relAddr = addr - (page << 8);
	return pageEntry;
}

uint8_t MemoryMapper::getValue(size_t addr) {
	uint16_t relAddr;
	return getBank(addr, relAddr)->getValue(relAddr);
}

void MemoryMapper::setValue(size_t addr, uint8_t value) {
	uint16_t relAddr;
	return getBank(addr, relAddr)->setValue(relAddr, value);
}

void MemoryMapper::emitLoad(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp dest) {
	uint16_t relAddr;
	auto bank = getBank(addr, relAddr);
	bank->emitLoad(a, relAddr, dest);
}

void MemoryMapper::emitStore(asmjit::X86Assembler& a, uint16_t addr, asmjit::X86Gp src) {
	uint16_t relAddr;
	auto bank = getBank(addr, relAddr);
	bank->emitStore(a, relAddr, src);
}

static uint8_t getHelper(MemoryMapper* mapper, uint16_t addr) {
	return mapper->getValue(addr);
}
static void setHelper(MemoryMapper* mapper, uint16_t addr, uint8_t value) {
	mapper->setValue(addr, value);
}

void MemoryMapper::emitDynamicLoad(asmjit::X86Assembler& a, asmjit::X86Gp addr, asmjit::X86Gp dest) {
	a.push(asmjit::x86::r10);
	a.push(asmjit::x86::r11);
	/* a.sub(asmjit::x86::rsp, 8); // Align stack pointer to 16 byte boundry */

	// First param
	a.mov(asmjit::x86::rdi, (uint64_t)this);
	// Second param
	a.mov(asmjit::x86::rsi, addr);
	a.call((uint64_t)&getHelper);

	/* a.add(asmjit::x86::rsp, 8); */
	a.pop(asmjit::x86::r11);
	a.pop(asmjit::x86::r10);

	// Move return value into dest register
	a.mov(dest, asmjit::x86::al);
}

template <class T>
void MemoryMapper::emitDynamicStore(asmjit::X86Assembler& a, asmjit::X86Gp addr, T src) {
	a.push(asmjit::x86::r10);
	a.push(asmjit::x86::r11);
	/* a.sub(asmjit::x86::rsp, 8); // Align stack pointer to 16 byte boundry */

	// First param
	a.mov(asmjit::x86::rdi, (uint64_t)this);
	// Second param
	a.mov(asmjit::x86::rsi, addr);
	// Third param
	a.mov(asmjit::x86::dl, src);
	a.call((uint64_t)&setHelper);

	/* a.add(asmjit::x86::rsp, 8); */
	a.pop(asmjit::x86::r11);
	a.pop(asmjit::x86::r10);
}

template void MemoryMapper::emitDynamicStore(asmjit::X86Assembler&, asmjit::X86Gp, uint8_t);
template void MemoryMapper::emitDynamicStore(asmjit::X86Assembler&, asmjit::X86Gp, asmjit::X86Gp);
