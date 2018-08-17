gsttrace_sources = [
  'gstelementheadstone.c',
  'gstgraveyard.c',
  'gstpadheadstone.c',
  'gsttaskheadstone.c',
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
