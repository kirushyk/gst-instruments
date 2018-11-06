gstintercept_sources = [
  'gstintercept.c'
]

gstintercept = library('gstintercept', 
  gstintercept_sources,
  dependencies : [
    glib,
    gstreamer
  ],
  c_args : [
    '-DGST_USE_UNSTABLE_API'
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
