project(
	'jit', ['cpp', 'c'],
	version : '1.0.0',
	license : 'GPL',
	default_options : [
		'cpp_std=c++1z',
	]
)
inc = include_directories('include')

add_global_arguments('-fpermissive', language : 'cpp')

asmjit_dep = dependency('asmjit')
fmt_dep = dependency('fmt', static: true)
thread_dep = dependency('threads')

subdir('include')
subdir('src')
subdir('test')
