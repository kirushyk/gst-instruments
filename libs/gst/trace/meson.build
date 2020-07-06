gsttrace_sources = [
  'gstelementheadstone.c',
  'gstgraveyard.c',
  'gsttrace.c',
  'gsttraceentry.c',
  'spycommon.c'
]

gsttrace = static_library('gsttrace', 
  gsttrace_sources,
  dependencies : [
    glib,
    gstreamer
  ],
  include_directories : inc,
  install : false
)
