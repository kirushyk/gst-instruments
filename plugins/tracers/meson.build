gstinstruments_sources = [
  'gstinstruments.c'
]

gstinstruments_deps = [
  glib,
  gstreamer
]

gstinstruments = library('gstinstruments', 
  gstinstruments_sources,
  dependencies : gstinstruments_deps,
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
