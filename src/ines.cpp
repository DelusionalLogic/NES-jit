#include "ines.h"
#include <string.h>
#include <fstream>
#include <memory>
#include <fmt/format.h>

#include "mapper/filememorybank.h"
#include "mapper/remapmemorybank.h"
#include "mapper/rammemorybank.h"

extern "C" uint64_t foo();

template<class T, class U>
std::unique_ptr<T> unique_static_cast(std::unique_ptr<U>& ptr) {
	return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
}

uint8_t ParserPointer::next() {
	return this->m_mapper.getValue(this->m_pointer++);
}

uint16_t ParserPointer::getLocation() {
	return this->m_pointer;
}

void ParserPointer::jump(size_t newPointer) {
	this->m_pointer = newPointer;
}

INes::INes(std::string path): PrgRomSize(0), ChrRomSize(0), PrgRamSize(0) {
	std::ifstream fs(path.c_str());

	char magic[5];
	fs.get(magic, 5);
	if(strcmp(magic, "NES\x1A") != 0)
		throw std::runtime_error("Not an ines file");
	printf("It's an ines file\n");

	char read;
	fs.get(read);
	this->PrgRomSize = read;

	fs.get(read);
	this->ChrRomSize = read;

	//@Expansion: discard for now
	fs.get(read);

	//@Expansion: discard for now
	fs.get(read);

	fs.get(read);
	this->PrgRamSize = read;

	//@Expansion: discard for now
	fs.get(read);

	//@Expansion: discard for now
	fs.get(read);

	//Discard the padding bytes
	char pad[6];
	fs.get(pad, 6);

	//@Expansion: Here we could have a bunch of stuff
	//I'm just going to assume only the essentials are here
	//At some point we probably need to support all the other
	//garbage - JJ 16/3 2017

	// For now just set up the basic mapping scheme. 0x8000->0xBFFF maps to
	// rom, and 0xC000->Whatever maps back around.
	mapper.setBank(0x00, std::make_shared<RamMemoryBank>(0x800));
	mapper.setBank(0x80, std::make_shared<ReadingMemoryBank>(fs, 16384 * this->PrgRomSize));
	mapper.setBank(0xC0, std::make_shared<RemappingMemoryBank>(mapper, 0x8000, 0x4000));
}

MemoryMapper& INes::getMapper() {
	return mapper;
}
