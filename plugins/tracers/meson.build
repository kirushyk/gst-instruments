gstinstruments_sources = [
  'gstinstruments.c'
]

gstinstruments_deps = [
  glib,
  gstreamer
]

gstinstruments = library('gstinstruments', 
  gstinstruments_sources,
  c_args : '-DGST_USE_UNSTABLE_API',
  dependencies : gstinstruments_deps,
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
