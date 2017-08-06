#pragma once

#include "ines.h"
#include <asmjit/asmjit.h>

enum AddrMode {
	ACCUMULATOR,
	ABSOLUTE,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	IMMEDIATE,
	IMPLIED,
	INDIRECT,
	X_INDIRECT,
	INDIRECT_Y,
	RELATIVE,
	ZEROPAGE,
	ZEROPAGE_X,
	ZEROPAGE_Y,
};

class Instr {
	private:

	public:
		enum AddrMode m_addrMode;
		std::string m_name;
		Instr(AddrMode addrMode, std::string name) : m_addrMode(addrMode), m_name(name) {};
		virtual ~Instr() {};
		virtual std::string format();
		virtual bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class NoArg : public Instr {
	public:
		NoArg(AddrMode addrMode, std::string name) : Instr(addrMode, name) {};
		template<class T> static std::unique_ptr<Instr> create(ParserPointer& pp);
		std::string format();
};

class SingleByte : public Instr {
	protected:
		uint8_t operand;
	public:
		SingleByte(AddrMode addrMode, std::string name, uint8_t operand) : Instr(addrMode, name), operand(operand) {};
		template<class T> static std::unique_ptr<Instr> create(ParserPointer& pp);
		std::string format();
};

class BranchInstr : public Instr {
	protected:
		uint16_t target;
		uint16_t next;
	public:
		BranchInstr(AddrMode addrMode, std::string name, uint16_t target, uint16_t next) : Instr(addrMode, name), target(target), next(next) {};
		template<class T> static std::unique_ptr<Instr> create(ParserPointer& pp);
};

class LDAImmInstr : public Instr {
	private:
		uint8_t value;
	public:
		LDAImmInstr(uint8_t value) : Instr(AddrMode::IMMEDIATE, "LDA"), value(value) {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
		virtual std::string format();
		virtual bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class LDAAbsXInstr : public Instr {
	private:
		uint16_t base;
	public:
		LDAAbsXInstr() : Instr(AddrMode::ABSOLUTE_X, "LDA") {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
		virtual std::string format();
		virtual bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class LDXImmInstr : public Instr {
	public:
		uint8_t m_value;
		LDXImmInstr(uint8_t value) : Instr(AddrMode::IMMEDIATE, "LDX"), m_value(value) {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class JMPAbsInstr : public Instr {
	private:
	public:
		uint16_t m_target;
		JMPAbsInstr(uint16_t target) : Instr(AddrMode::ABSOLUTE, "JMP"), m_target(target) {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class JSRAbsInstr : public BranchInstr {
	public:
		JSRAbsInstr(uint16_t target, uint16_t next) : BranchInstr(AddrMode::ABSOLUTE, "JSR", target, next) {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class RTS : public NoArg {
	public:
		RTS() : NoArg(AddrMode::IMPLIED, "RTS") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class ADCImmInstr : public NoArg {
	public:
		ADCImmInstr() : NoArg(AddrMode::IMMEDIATE, "ADC") {};
};

class NOP : public NoArg {
	public:
		NOP() : NoArg(AddrMode::IMPLIED, "NOP") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class STAAbsXInstr : public Instr {
	public:
		STAAbsXInstr() : Instr(AddrMode::ABSOLUTE_X, "STA") {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
};

class STXZeroPInstr : public Instr {
	private:
		uint8_t operand;
	public:
		STXZeroPInstr(uint8_t operand) : Instr(AddrMode::ZEROPAGE, "STX"), operand(operand) {};
		static std::unique_ptr<Instr> create(ParserPointer& pp);
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class SEC : public NoArg {
	public:
		SEC() : NoArg(AddrMode::IMPLIED, "SEC") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class CLC : public NoArg {
	public:
		CLC() : NoArg(AddrMode::IMPLIED, "CLC") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class SEI : public NoArg {
	public:
		SEI() : NoArg(AddrMode::IMPLIED, "SEI") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class SED : public NoArg {
	public:
		SED() : NoArg(AddrMode::IMPLIED, "SED") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class CLD : public NoArg {
	public:
		CLD() : NoArg(AddrMode::IMPLIED, "CLD") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class PHP : public NoArg {
	public:
		PHP() : NoArg(AddrMode::IMPLIED, "PHP") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class PLA : public NoArg {
	public:
		PLA() : NoArg(AddrMode::IMPLIED, "PLA") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class PLP : public NoArg {
	public:
		PLP() : NoArg(AddrMode::IMPLIED, "PLP") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class PHA : public NoArg {
	public:
		PHA() : NoArg(AddrMode::IMPLIED, "PHA") {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BCSRelInstr : public BranchInstr {
	public:
		BCSRelInstr(uint8_t operand, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BCS", operand, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BCCRelInstr : public BranchInstr {
	public:
		BCCRelInstr(uint8_t target, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BCC", target, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BVSRelInstr : public BranchInstr {
	public:
		BVSRelInstr(uint8_t target, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BVS", target, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BVCRelInstr : public BranchInstr {
	public:
		BVCRelInstr(uint8_t target, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BVC", target, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BEQRelInstr : public BranchInstr {
	public:
		BEQRelInstr(uint8_t target, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BEQ", target, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BNERelInstr : public BranchInstr {
	public:
		BNERelInstr(uint8_t target, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BNE", target, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BPLRelInstr : public BranchInstr {
	public:
		BPLRelInstr(uint8_t target, uint16_t next) : BranchInstr(AddrMode::RELATIVE, "BPL", target, next) {};
		std::string format();
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class STAZeroP : public SingleByte {
	public:
		STAZeroP(uint8_t operand) : SingleByte(AddrMode::ZEROPAGE, "STA", operand) {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class BITZeroP : public SingleByte {
	public:
		BITZeroP(uint8_t operand) : SingleByte(AddrMode::ZEROPAGE, "BIT", operand) {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class ANDImm : public SingleByte {
	public:
		ANDImm(uint8_t operand) : SingleByte(AddrMode::IMMEDIATE, "AND", operand) {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};

class CMPImm : public SingleByte {
	public:
		CMPImm(uint8_t operand) : SingleByte(AddrMode::IMMEDIATE, "CMP", operand) {};
		bool exp(asmjit::X86Assembler& assembler, MemoryMapper& m);
};


typedef std::unique_ptr<Instr> (*CompileFunc)(ParserPointer& pp);

static const CompileFunc opcodeTable[256] = {
//  0x00                             , 0x01                       , 0x02                , 0x03                , 0x04                         , 0x05                         , 0x06                  , 0x07 ,
//  0x08                             , 0x09                       , 0x0A                , 0x0B                , 0x0C                         , 0x0D                         , 0x0E                  , 0x0F ,
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<PHP>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 00h
	BranchInstr::create<BPLRelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<CLC>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 10h
	JSRAbsInstr::create              , NULL                       , NULL                , NULL                , SingleByte::create<BITZeroP> , NULL                         , NULL                  , NULL , 
	NoArg::create<PLP>               , SingleByte::create<ANDImm> , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 20h
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<SEC>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 30h
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<PHA>               , NULL                       , NULL                , NULL                , JMPAbsInstr::create          , NULL                         , NULL                  , NULL , // 40h
	BranchInstr::create<BVCRelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 50h
	NoArg::create<RTS>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<PLA>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 60h
	BranchInstr::create<BVSRelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<SEI>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 70h
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , SingleByte::create<STAZeroP> , STXZeroPInstr::create , NULL ,
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // 80h
	BranchInstr::create<BCCRelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , STAAbsXInstr::create         , NULL                  , NULL , // 90h
	NULL                             , NULL                       , LDXImmInstr::create , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NULL                             , LDAImmInstr::create        , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // A0h
	BranchInstr::create<BCSRelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , LDAAbsXInstr::create         , NULL                  , NULL , // B0h
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NULL                             , SingleByte::create<CMPImm> , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // C0h
	BranchInstr::create<BNERelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<CLD>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // D0h
	NULL                             , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NULL                             , NULL                       , NoArg::create<NOP>  , NULL                , NULL                         , NULL                         , NULL                  , NULL , // E0h
	BranchInstr::create<BEQRelInstr> , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL ,
	NoArg::create<SED>               , NULL                       , NULL                , NULL                , NULL                         , NULL                         , NULL                  , NULL , // F0h
};

