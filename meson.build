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

exe = executable(
  'straightface',
  'straightface.cpp',
  dependencies: provided_deps,
  install: true,
)

# test('basic', exe)
