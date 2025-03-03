project('tachyon', 'cpp',
    default_options: ['optimization=2', 'cpp_std=c++23']
)
src_files = [ 'src/ast/visitor.cpp'
            , 'src/ir/visitor.cpp'
            , 'src/vm.cpp'
            ]
inc_dirs = include_directories('src')
executable('tachyon', ['src/main.cpp'] + src_files, include_directories: inc_dirs)
