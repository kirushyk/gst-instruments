project('gst-instruments', 'c',
  version : '0.3.0',
  license : ['LGPL'],
  meson_version : '>= 0.37.1',
  default_options : [
    'warning_level=3',
    'buildtype=debug',
    'c_std=c99'
  ]
)

cc = meson.get_compiler('c')

gst_api_version = '1.0'

glib = dependency('glib-2.0')
gstreamer = dependency('gstreamer-' + gst_api_version)

conf = configuration_data()
conf.set_quoted('GST_TOP_TRACE_FILENAME_BASE', 'gst-top')
conf.set('GST_ELEMENT_TYPE_NAME_LENGTH_MAX', 128)
conf.set('GST_ELEMENT_NAME_LENGTH_MAX', 128)
conf.set('GST_TRACE_ENTRY_SIZE', 512)
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('GST_API_VERSION', gst_api_version)
conf.set_quoted('BINDIR', join_paths(get_option('prefix'), get_option('bindir')))
conf.set_quoted('LIBDIR', join_paths(get_option('prefix'), get_option('libdir')))
conf.set_quoted('DATADIR', join_paths(get_option('prefix'), get_option('datadir'), meson.project_name()))
configure_file(output : 'config.h', configuration : conf)

inc = include_directories('.')

subdir('libs/gst')
subdir('plugins')
subdir('tools')
subdir('ui')
