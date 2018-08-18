gst_top_sources = [
  'gst-top.c'
]

gst_top_sources = library('gst-top-' + gst_api_version, 
  gst_top_sources,
  dependencies : [
    glib,
    gstreamer
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
