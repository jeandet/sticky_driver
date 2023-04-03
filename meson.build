project(
  'sticky',
  'cpp',
  meson_version: '>= 0.63.0',
  version :'1.0.0',
  default_options : ['warning_level=3', 'cpp_std=c++17', 'default_library=static'],
  license : 'GPL3'
)

if get_option('buildtype').contains('debug')
    add_project_arguments('-DSTICKY_ENABLE_ASSERT', language : ['cpp'])
    add_project_arguments('-DSTICKY_HEDLEY', language : ['cpp'])
    #add_project_arguments('-DSTICKY_VERBOSE', language : ['cpp'])
endif


conf_data = configuration_data()
conf_data.set_quoted('STICKY_VERSION', meson.project_version())
configure_file(output : 'config.h',
               configuration : conf_data)

cpp = meson.get_compiler('cpp')
if('clang'==cpp.get_id())
    add_global_arguments('-fsized-deallocation', language : 'cpp')
endif


pybind11_dep = dependency('pybind11', required : true, fallback:['pybind11','pybind11_dep'])
hedley_dep = dependency('hedley', main : true, fallback : ['hedley', 'hedley_dep'])
fmt_dep = dependency('fmt')

if build_machine.system() == 'windows'
    link_args = ['-static-libstdc++','-static-libgcc','-static']
elif build_machine.system() == 'darwin'
    link_args = ['-static-libstdc++','-static-libgcc']
else
    link_args = []
endif




pymod = import('python')
python3 = pymod.find_installation('python3')


stick_lib = static_library('sticky','src/sticky.cpp',
                            dependencies: [fmt_dep],
                            link_args: link_args,
                            include_directories : ['src'],
                            install: true,
                            extra_files: [
                            'src/sticky.hpp',
                            'src/filter.hpp',
                            'pyproject.toml'
                            ]
                          )

stick_dep = declare_dependency(
    link_with: [stick_lib],
    include_directories : ['src']
)

python3.extension_module('sticky', 'src/wrapper.cpp',
                         dependencies: [pybind11_dep, fmt_dep, python3.dependency(), stick_dep],
                         link_args: link_args,
                         install: true
                        )

if get_option('with_tests')

    catch_dep = dependency('catch2', main : true)

    foreach test:['decode_data']
        exe = executable('test-'+test,'tests/'+test+'/main.cpp',
                        dependencies:[catch_dep, stick_dep],
                        install: false
                        )
        test(test, exe)
    endforeach

endif

summary({'C': meson.get_compiler('c').cmd_array(),
        'C++': meson.get_compiler('cpp').cmd_array()
        }, section: 'Compilers')
