#include <asmjit/asmjit.h>
#include <stdio.h>
#include <iostream>
#include <fmt/format.h>
#include <unistd.h>
#include "instruction.h"
#include "ines.h"

#include <glad/glad.h>
#include <SDL.h>
#include "imgui/imgui_impl_sdl_gl3.h"
#include "imgui/imgui.h"

#include "polym/msg.hpp"
#include "polym/queue.hpp"

struct CpuState {
	uint8_t SP;
	uint8_t S;
	uint8_t A;
	uint8_t X;
	uint8_t Y;
};

template<class T, class U>
std::unique_ptr<T> unique_static_cast(std::unique_ptr<U>& ptr) {
	return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
}

// Signature of the generated function.
typedef void (*Func)(void);

struct Context {
	INes& game;
	MemoryMapper& mapper;
	asmjit::JitRuntime& rt;
	
	uint16_t location;
	struct CpuState exitState; // Set when the jit function can be reentered
} *context;

// Careful here. These are written to directly from the assembly wrapper
struct Registers {
	uint8_t sp;
	uint8_t s;
	uint8_t a;
	uint8_t x;
	uint8_t y;
};

extern "C" void outer_jit_wrapper(uint16_t target);

#include <thread>

PolyM::Queue jitQueue;
PolyM::Queue guiQueue;

extern "C" uint64_t jit(uint16_t target, struct Registers* saved_registers) {
	// @HACK: Location should be passed in to the context maybe?
	context->location = target;

	guiQueue.put(PolyM::DataMsg<struct Registers>(1, *saved_registers));

	fmt::print("Jitting block starting at {:X}\n", context->location);

	asmjit::StringLogger logger;
	logger.addOptions(asmjit::Logger::kOptionHexImmediate);
	asmjit::CodeHolder code;                       // Holds code and relocation information.
	code.init(context->rt.getCodeInfo());          // Initialize to the same arch as JIT runtime.
	code.setLogger(&logger);

	asmjit::X86Assembler a(&code);                 // Create and attach X86Assembler to `code`.

	ParserPointer pp(context->mapper, context->location);

	auto block = std::make_shared<std::vector<std::unique_ptr<Instr>>>();

	bool cont = true;
	while(cont) {
		uint8_t b = pp.next();
		auto ic = opcodeTable[b];
		if(ic == nullptr) {
			fmt::print("Unknown opcode 0x{0:X} ({0}) at location {1:X}, ABORT\n", b, pp.getLocation());
			return 0;
		}
		auto i = ic(pp);
		cont = !i->stop_jit();
		block->push_back(std::move(i));
	}

	fmt::print("The current block has addr {}\n", (void*)(block.get()));

	// Now that we have a block, we can ask the ui if this should be shown
	guiQueue.put(PolyM::DataMsg<std::shared_ptr<std::vector<std::unique_ptr<Instr>>>>(2, block));
	auto msg = jitQueue.get(-1);
	
	for(auto &instr : *block) {
		a.comment(fmt::format("; {}", instr->format()).c_str());
		instr->exp(a, context->mapper);
	}
	/* fmt::print("\nGenerated code\n"); */
	/* fmt::print("{}\n", logger.getString()); */

	Func fn;
	asmjit::Error err = context->rt.add(&fn, &code);
	if (err) {
		fmt::print("Failed adding the code to the runtime");
		return 0;
	}

	// @COMPLETENESS: Ideally we would also save the jit result to make sure we
	// don't have to do it again.
	// @LEAK: We are leaking the function here. We should save that somewhere
	// and free it when we exit
	return (uint64_t)fn;
}

void call_from_thread() {
	//Compile starting at the progstart location
	outer_jit_wrapper(0xC000);
}

int main(int argc, char* argv[]) {
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// Setup window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	SDL_Window *window = SDL_CreateWindow("ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	gladLoadGL();

	// Setup ImGui binding
	ImGui_ImplSdlGL3_Init(window);

	// Load Fonts
	// (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
	//io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

	ImVec4 clear_color = ImColor(114, 144, 154);

	// Setup jit runtime
	asmjit::JitRuntime rt;                         // Runtime specialized for JIT code execution.

	// Open file
	INes f("game.nes");

	Context con{
		f,
		f.getMapper(),
		rt
	};
	context = &con;

	std::thread jitThread(call_from_thread);

	struct Registers regs;
	std::shared_ptr<std::vector<std::unique_ptr<Instr>>> currentBlock = nullptr;

	// Main loop
	bool done = false;
	while (!done)
	{
		auto msg = guiQueue.get();
		if(msg->getMsgId() != PolyM::MSG_TIMEOUT) {
			switch(msg->getMsgId()) {
				case 1:
					regs = unique_static_cast<PolyM::DataMsg<struct Registers>>(msg)->getPayload();
					break;
				case 2:
					currentBlock = unique_static_cast<PolyM::DataMsg<std::shared_ptr<std::vector<std::unique_ptr<Instr>>>>>(msg)->getPayload();
					break;
			}
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSdlGL3_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
		}
		ImGui_ImplSdlGL3_NewFrame(window);

		{
			ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Registers");

			ImGui::Text("X");
			ImGui::SameLine(50);
			ImGui::Text("0x%02x", regs.x);

			ImGui::Text("Y");
			ImGui::SameLine(50);
			ImGui::Text("0x%02x", regs.y);

			ImGui::Separator();

			ImGui::Text("A");
			ImGui::SameLine(50);
			ImGui::Text("0x%02x", regs.a);

			ImGui::Separator();

			ImGui::Text("S");
			ImGui::SameLine(50);
			ImGui::Text("0x%02x", regs.sp);

			ImGui::Text("SP");
			ImGui::SameLine(50);

			ImGui::Text("%s", fmt::format("{:#010b}", regs.s).c_str());

			ImGui::Separator();
			ImGui::End();
		}

		ImGui::Begin("Compile");
		if(ImGui::Button("Next")) {
			fmt::print("Next Block!\n");
			jitQueue.put(PolyM::Msg(1));
		}
		if(currentBlock != nullptr) {
			for(auto &instr : *currentBlock) {
				ImGui::Text("%s", instr->format().c_str());
			}
		}
		ImGui::End();

		// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowTestWindow();

		// Rendering
		glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		SDL_GL_SwapWindow(window);
	}

	jitThread.join();

	// Cleanup
	ImGui_ImplSdlGL3_Shutdown();
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
