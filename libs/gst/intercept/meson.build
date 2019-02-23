gstintercept_sources = [
  'gstintercept.c'
]

gstintercept_deps = [
  glib,
  gstreamer
]

if host_machine.system() == 'linux'
  gstinstruments_deps += cc.find_library('dl', required : true)
endif

gstintercept = library('gstintercept', 
  gstintercept_sources,
  dependencies : gstintercept_deps,
  c_args : [
    '-DGST_USE_UNSTABLE_API'
  ],
  include_directories : inc,
  link_with : gsttrace,
  install : true
)
