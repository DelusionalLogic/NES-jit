#include "instruction.h"
//@CLEANUP only include memorymapper when split
#include "ines.h"
#include <fmt/format.h>

#define REG_SP asmjit::x86::r10b
#define REG_S  asmjit::x86::r11b
#define REG_PC asmjit::x86::r12w
#define REG_A  asmjit::x86::r13b
#define REG_X  asmjit::x86::r14b
#define REG_Y  asmjit::x86::r15b

#define REG_TMP asmjit::x86::rax

#define S_CARRY         0
#define S_ZERO          1
#define S_INTER_DISABLE 2
#define S_DECIMAL       3
#define S_INTERRUPT     4
#define S_ALWAYS        5
#define S_OVERFLOW      6
#define S_NEGATIVE      7

extern "C" uint64_t jit_and_jump();

static void dump(uint8_t A, uint8_t X, uint8_t Y, uint8_t status) {
	fmt::print("A: 0x{:X}, X: 0x{:X}, Y: 0x{:X}, Status: 0b{:B}\n", A, X, Y, status);
}

static void emitDump(asmjit::X86Assembler& a) {
	a.push(asmjit::x86::r10);
	a.push(asmjit::x86::r11);

	a.mov(asmjit::x86::dil, REG_A);
	a.mov(asmjit::x86::sil, REG_X);
	a.mov(asmjit::x86::dl, REG_Y);
	a.mov(asmjit::x86::cl, REG_S);
	a.call((uint64_t)&dump);

	a.pop(asmjit::x86::r11);
	a.pop(asmjit::x86::r10);
}

std::string Instr::format() {
	return fmt::format("{}", this->m_name);
}

std::unique_ptr<Instr> LDAImmInstr::create(ParserPointer& pp) {
	uint8_t value = pp.next();
	return std::make_unique<LDAImmInstr>(value);
}

std::unique_ptr<Instr> LDAAbsXInstr::create(ParserPointer& pp) {
	return std::make_unique<LDAAbsXInstr>();
}

std::unique_ptr<Instr> LDXImmInstr::create(ParserPointer& pp) {
	return std::make_unique<LDXImmInstr>(pp.next());
}

std::unique_ptr<Instr> JMPAbsInstr::create(ParserPointer& pp) {
	uint16_t target = pp.next() | (pp.next() << 8);
	return std::make_unique<JMPAbsInstr>(target);
}

std::unique_ptr<Instr> JSRAbsInstr::create(ParserPointer& pp) {
	uint16_t target = pp.next() | (pp.next() << 8);
	fmt::print("TARGET: {:X}\n", target);
	uint16_t next = pp.getLocation();
	return std::make_unique<JSRAbsInstr>(target, next);
}

std::unique_ptr<Instr> STAAbsXInstr::create(ParserPointer& pp) {
	return std::make_unique<STAAbsXInstr>();
}

std::unique_ptr<Instr> STXZeroPInstr::create(ParserPointer& pp) {
	uint8_t operand = pp.next();
	return std::make_unique<STXZeroPInstr>(operand);
}

template<class T>
std::unique_ptr<Instr> BranchInstr::create(ParserPointer& pp) {
	uint8_t operand = pp.next();
	uint16_t next = pp.getLocation();
	return std::make_unique<T>(operand, next);
}

template<class T>
std::unique_ptr<Instr> SingleByte::create(ParserPointer& pp) {
	uint8_t operand = pp.next();
	return std::make_unique<T>(operand);
}

template<class T>
std::unique_ptr<Instr> NoArg::create(ParserPointer& pp) {
	return std::make_unique<T>();
}

bool Instr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	throw std::logic_error(
		fmt::format(
			"Instruction {} in addressing mode {} is not yet supported",
			this->m_name,
			this->m_addrMode
		)
	);
}

bool JMPAbsInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.mov(asmjit::x86::di, this->m_target);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

void virtual_push(asmjit::X86Assembler& a, MemoryMapper& m, auto value) {
	a.mov(REG_TMP, 0x0100);
	// @CLEANUP TMP is rax, but we can't or with a larger register
	a.add(asmjit::x86::al, REG_SP);
	m.emitDynamicStore(a, REG_TMP, value);
	a.dec(REG_SP);
}

void virtual_pop(asmjit::X86Assembler& a, MemoryMapper& m, auto dst) {
	a.inc(REG_SP);
	a.mov(REG_TMP, 0x0100);
	// @CLEANUP TMP is rax, but we can't or with a larger register
	a.add(asmjit::x86::al, REG_SP);
	m.emitDynamicLoad(a, REG_TMP, dst);
}

bool JSRAbsInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	// Push the virtual PC to the stack
	// @COMPLETENESS: This order is likely wrong!
	virtual_push(a, m, static_cast<uint8_t>(next & 0xFF));
	virtual_push(a, m, static_cast<uint8_t>(next >> 8));

	a.mov(asmjit::x86::di, this->target);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool RTS::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	// Use rbx because that's safe during a call
	virtual_pop(a, m, asmjit::x86::bl);
	a.shl(asmjit::x86::bx, 8);
	virtual_pop(a, m, asmjit::x86::dl);
	a.or_(asmjit::x86::bx, asmjit::x86::dx);
	a.mov(asmjit::x86::rdi, asmjit::x86::rbx);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool SEI::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.bts(REG_S, S_INTER_DISABLE);
	return true;
}

bool SED::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.bts(REG_S, S_DECIMAL);
	return true;
}

bool CLD::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.btr(REG_S, S_DECIMAL);
	return true;
}

bool PHP::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	virtual_push(a, m, REG_S);
	return true;
}

bool PLA::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	virtual_pop(a, m, REG_A);
	return true;
}

bool PLP::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	virtual_pop(a, m, REG_S);
	return true;
}

bool PHA::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	virtual_push(a, m, REG_A);
	return true;
}

bool STXZeroPInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	m.emitStore(a, operand, REG_X);
	return true;
}

bool ANDImm::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.and_(REG_A, this->operand);
	return true;
}

bool CMPImm::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.cmp(REG_A, this->operand);

	{
		a.pushfd();
		a.btr(REG_S, S_CARRY);
		a.popfd();
		auto End = a.newLabel();
		a.jnc(End);
		a.pushfd();
		a.bts(REG_S, S_CARRY);
		a.popfd();
		a.bind(End);
	}

	{
		a.pushfd();
		a.btr(REG_S, S_ZERO);
		a.popfd();


		auto End = a.newLabel();
		a.jne(End);
		a.pushfd();
		a.bts(REG_S, S_ZERO);
		a.popfd();
		a.bind(End);
	}

	{
		a.pushfd();
		a.btr(REG_S, S_NEGATIVE);
		a.popfd();


		auto End = a.newLabel();
		a.jns(End);
		a.pushfd();
		a.bts(REG_S, S_NEGATIVE);
		a.popfd();
		a.bind(End);
	}
	return true;
}

bool LDAImmInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.mov(REG_A, this->value);

	// Immediate mode knows the value at compile time, so just emit the right
	// thing
	if(this->value == 0)
		a.bts(REG_S, S_ZERO);
	else
		a.btr(REG_S, S_ZERO);

	if((this->value & 0x80) == 0)
		a.btr(REG_S, S_NEGATIVE);
	else
		a.bts(REG_S, S_NEGATIVE);
	return true;
}

bool LDAAbsXInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto temp = asmjit::x86::rax;
	a.mov(temp, this->base);
	a.add(temp, REG_X);
	auto tmpPtr = asmjit::x86::byte_ptr(temp);

	{
		auto NotZero = a.newLabel();
		auto Exit = a.newLabel();
		a.cmp(tmpPtr, 0);
		a.jne(NotZero);
		// Value was 0
		a.bts(REG_S, S_ZERO);
		a.jmp(Exit);

		// Value was not 0
		a.bind(NotZero);
		a.btr(REG_S, S_ZERO);
		//No jmp required, just fall though

		a.bind(Exit);
	}

	a.mov(REG_A, tmpPtr);
	return true;
}

bool LDXImmInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.mov(REG_X, this->m_value);
	// Immediate mode knows the value at compile time, so just emit the right
	// thing
	if(this->m_value == 0)
		a.bts(REG_S, S_ZERO);
	else
		a.btr(REG_S, S_ZERO);
	return true;
}

bool NOP::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.nop();
	return true;
}

bool SEC::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.bts(REG_S, S_CARRY);
	return true;
}

bool CLC::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	a.btr(REG_S, S_CARRY);
	return true;
}

bool BCSRelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto NotSet = a.newLabel();

	a.bt(REG_S, S_CARRY);
	a.jnc(NotSet);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(NotSet);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool BCCRelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto Set = a.newLabel();

	a.bt(REG_S, S_CARRY);
	a.jc(Set);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(Set);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool BVSRelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto NotSet = a.newLabel();

	a.bt(REG_S, S_OVERFLOW);
	a.jnc(NotSet);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(NotSet);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool BVCRelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto Set = a.newLabel();

	a.bt(REG_S, S_OVERFLOW);
	a.jc(Set);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(Set);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool BEQRelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto NotSet = a.newLabel();

	a.bt(REG_S, S_ZERO);
	a.jnc(NotSet);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(NotSet);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool BNERelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto Set = a.newLabel();

	a.bt(REG_S, S_ZERO);
	a.jc(Set);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(Set);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool BPLRelInstr::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	auto Set = a.newLabel();

	a.bt(REG_S, S_NEGATIVE);
	a.jc(Set);
	a.mov(asmjit::x86::di, this->next + this->target);
	a.jmp((uint64_t)&jit_and_jump);
	a.bind(Set);
	a.mov(asmjit::x86::di, this->next);
	a.jmp((uint64_t)&jit_and_jump);
	return false;
}

bool STAZeroP::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	m.emitStore(a, operand, REG_A);
	return true;
}

bool BITZeroP::exp(asmjit::X86Assembler& a, MemoryMapper& m) {
	m.emitLoad(a, operand, asmjit::x86::al);
	// @COMPLETENESS: We should check and set the S_ flags here before the and
	a.push(asmjit::x86::rax);

	a.and_(asmjit::x86::al, REG_A);
	auto Zero = a.newLabel();
	auto Exit = a.newLabel();
	a.jz(Zero);

	//Not zero
	a.btr(REG_S, S_ZERO);
	a.jmp(Exit);
	a.bind(Zero);
	a.bts(REG_S, S_ZERO);
	a.bind(Exit);

	a.pop(asmjit::x86::rax);

	{
		a.btr(REG_S, S_OVERFLOW);
		a.bt(asmjit::x86::al, 6);
		auto End = a.newLabel();
		a.jnc(End);
		a.bts(REG_S, S_OVERFLOW);
		a.bind(End);
	}

	{
		a.btr(REG_S, S_NEGATIVE);
		a.bt(asmjit::x86::al, 7);
		auto End = a.newLabel();
		a.jnc(End);
		a.bts(REG_S, S_NEGATIVE);
		a.bind(End);
	}
	return true;
}

std::string JMPAbsInstr::format() {
	return fmt::format("{} #{:X}", m_name, m_target);
}

std::string JSRAbsInstr::format() {
	return fmt::format("{} #{:X}", m_name, target);
}

std::string LDXImmInstr::format() {
	return fmt::format("{} #{:X}", m_name, m_value);
}

std::string LDAImmInstr::format() {
	return fmt::format("{} #{:X}", m_name, value);
}

std::string LDAAbsXInstr::format() {
	return fmt::format("{} {:X},X", m_name, base);
}

std::string STXZeroPInstr::format() {
	return fmt::format("{} ${:X}", m_name, operand);
}

std::string BCSRelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string BVSRelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string BCCRelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string BVCRelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string BEQRelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string BNERelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string BPLRelInstr::format() {
	return fmt::format("{} *{:X}", m_name, target);
}

std::string SingleByte::format() {
	return fmt::format("{} {}", m_name, operand);
}

std::string NoArg::format() {
	return fmt::format("{}", m_name);
}
