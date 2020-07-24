gst_instruments_sources = [
  'main.vala',
  'mainmenu.vala',
  'timeline.vala',
  'mainwindow.vala'
]

have_vala = add_languages('vala', native: false, required: get_option('ui'))
if have_vala
  gst_instruments_deps = [
    dependency('gtk+-3.0'),
    meson.get_compiler('vala').find_library('posix')
  ]
  executable('gst-instruments-' + gst_api_version,
    gst_instruments_sources,
    dependencies : gst_instruments_deps,
    install : true
  )
endif