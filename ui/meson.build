gst_instruments_sources = [
  'main.vala',
  'mainmenu.vala',
  'timeline.vala',
  'mainwindow.vala'
]

gst_instruments_deps = [
  dependency('gtk+-3.0'),
  meson.get_compiler('vala').find_library('posix')
]

have_vala = add_languages('vala', required: false)
if have_vala
  executable('gst-instruments-' + gst_api_version,
    gst_instruments_sources,
    dependencies: gst_instruments_deps
  )
endif