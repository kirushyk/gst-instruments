gstintercept_sources = [
  'gstintercept.c'
]

gstintercept = library('gstintercept', 
  gstintercept_sources,
  dependencies : [
    glib,
    gstreamer
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
