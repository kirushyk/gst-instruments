project('gst-instruments', 'c', 'vala',
  version : '0.2.4',
  license : ['LGPL'],
  meson_version : '>= 0.37.1',
  default_options : [
    'warning_level=3',
    'buildtype=debug',
    'c_std=c99'
  ]
)

conf = configuration_data()
conf.set_quoted('GST_TOP_TRACE_FILENAME_BASE', 'gst-top')
conf.set('GST_ELEMENT_TYPE_NAME_LENGTH_MAX', 128)
conf.set('GST_ELEMENT_NAME_LENGTH_MAX', 128)
conf.set('GST_TRACE_ENTRY_SIZE', 512)
conf.set_quoted('GST_API_VERSION', '1.0')
configure_file(output : 'config.h', configuration : conf)
