jit_sources = [
	'main.cpp',
	'ines.cpp',
	'instruction.cpp',
	'mapper/memorymapper.cpp',
	'mapper/filememorybank.cpp',
	'mapper/remapmemorybank.cpp',
	'mapper/rammemorybank.cpp',
	'fun.S'
]
exe = executable(
	'jit',
	jit_sources,
	include_directories : inc,
	dependencies: [asmjit_dep, fmt_dep]
)
test('jit', exe)
