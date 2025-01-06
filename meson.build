project('gst-instruments', 'c',
  version : '0.3.2',
  license : ['LGPL'],
  meson_version : '>= 0.49.0',
  default_options : [
    'warning_level=2',
    'buildtype=debug',
    'c_std=gnu99'
  ]
)

cc = meson.get_compiler('c')

gst_api_version = '1.0'

glib = dependency('glib-2.0')
gstreamer = dependency('gstreamer-' + gst_api_version)

plugins_install_dir = get_option('libdir') / 'gstreamer-1.0'
data_install_dir = get_option('datadir') / meson.project_name()

conf = configuration_data()
conf.set_quoted('GST_TOP_TRACE_FILENAME_BASE', 'gst-top')
conf.set('GST_ELEMENT_TYPE_NAME_LENGTH_MAX', 128)
conf.set('GST_ELEMENT_NAME_LENGTH_MAX', 128)
conf.set('GST_TRACE_ENTRY_SIZE', 512)
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('GST_API_VERSION', gst_api_version)
conf.set_quoted('BINDIR', get_option('prefix') / get_option('bindir'))
conf.set_quoted('LIBDIR', get_option('prefix') / get_option('libdir'))
conf.set_quoted('DATADIR', get_option('prefix') / get_option('datadir') / meson.project_name())
conf.set_quoted('GST_PLUGIN_PATH', get_option('prefix') / plugins_install_dir)

configure_file(output : 'config.h', configuration : conf)

inc = include_directories('.')

subdir('libs/gst')
subdir('plugins')
subdir('tools')
subdir('data')
