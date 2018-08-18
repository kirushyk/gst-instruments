gst_top_sources = [
  'gst-top.c'
]

gst_top = library('gst-top-' + gst_api_version, 
  gst_top_sources,
  dependencies : [
    glib
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)

gst_report_sources = [
  'gst-report.c',
  'formatters.c'
]

gst_report = library('gst-report-' + gst_api_version, 
  gst_report_sources,
  dependencies : [
    glib,
    gstreamer
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)