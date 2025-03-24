project('tachyon', 'cpp',
    default_options: ['cpp_std=c++23', 'O=fast']
)
src_files = [ 'src/ast/visitor.cpp'
            , 'src/ir/visitor.cpp'
            , 'src/parser/tokenizer.cpp'
            , 'src/parser/parser.cpp'
            , 'src/vm/vm.cpp'
            ]

mimalloc = dependency('mimalloc')
spdlog = dependency('spdlog')
debug_args = []

if get_option('buildtype') == 'debug'
    debug_args += ['-DDEBUG']
endif

inc_dirs = include_directories('src')

executable('tachyon', ['src/main.cpp'] + src_files, include_directories: inc_dirs, dependencies : [mimalloc, spdlog], cpp_args: debug_args)
