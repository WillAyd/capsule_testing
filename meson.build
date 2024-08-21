project(
    'capsule_bug',
    'cpp',
    default_options: [
        'warning_level=2',
        'cpp_std=c++20',
    ],
)

py = import('python').find_installation()
nanoarrow_dep = dependency(
    'nanoarrow',
)
nanobind_dep = dependency('nanobind')

py.extension_module(
    'capsule_bug',
    sources: ['src/capsule_bug/capsule_bug.cc'],
    dependencies: [nanoarrow_dep, nanobind_dep],
    install: true,
)
