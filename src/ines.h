#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <asmjit/asmjit.h>

#include "mapper/memorymapper.h"

class ParserPointer {
	private:
		MemoryMapper& m_mapper;
		size_t m_pointer;
	public:
		ParserPointer(MemoryMapper& mapper, size_t pointer) : m_mapper(mapper), m_pointer(pointer) {};
		uint8_t next();
		uint16_t getLocation();
		void jump(size_t newPointer);
};

class INes {
	private:
		MemoryMapper mapper;
		std::shared_ptr<char> memory;
	public:
		uint16_t PrgRomSize;
		uint8_t ChrRomSize;
		uint8_t PrgRamSize;

		INes(std::string path);

		MemoryMapper& getMapper();
};
