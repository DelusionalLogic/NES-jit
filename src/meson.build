jit_sources = [
	'main.cpp',
	'ines.cpp',
	'instruction.cpp',

	'mapper/memorymapper.cpp',
	'mapper/filememorybank.cpp',
	'mapper/remapmemorybank.cpp',
	'mapper/rammemorybank.cpp',

	'imgui/imgui.cpp',
	'imgui/imgui_demo.cpp',
	'imgui/imgui_draw.cpp',
	'imgui/imgui_impl_sdl_gl3.cpp',

	'polym/msg.cpp',
	'polym/queue.cpp',

	'glad/src/glad.c',

	'fun.S'
]

glad = include_directories('glad/include', is_system: true)

cc = meson.get_compiler('c')
libdl = cc.find_library('dl', required : false)

sdl = dependency('SDL2')
exe = executable(
	'jit',
	jit_sources,
	include_directories : [inc, glad],
	dependencies: [asmjit_dep, fmt_dep, thread_dep, libdl, sdl]
)
test('jit', exe)
