gstinstruments_sources = [
  'gstinstruments.c'
]

gstinstruments = library('gstinstruments', 
  gstinstruments_sources,
  dependencies : [
    glib,
    gstreamer
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
