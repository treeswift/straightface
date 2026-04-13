project(
  'straightface',
  'cpp',
  meson_version: '>= 1.3.0',
  version: '0.1',
  default_options: ['warning_level=3'],
)

provided_deps = [
  dependency('opencv4'),
]

gui = static_library('cvguiwrapper',
  [
    'windowframe.cpp',
  ],
  dependencies: provided_deps)

exe = executable(
  'straightface',
  [
    'straightface.cpp',
  ],
  dependencies: provided_deps,
  link_with: [gui],
  install: true,
)

# test('basic', exe)
