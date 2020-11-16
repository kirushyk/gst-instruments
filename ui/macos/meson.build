gst_instruments_macos_sources = [
  'AppDelegate.swift',
  'ContentView.swift'
]

have_swift = add_languages('swift', native: false, required: false)

if have_swift
  gst_instruments_macos_deps = [
    dependency('swiftui'),
  ]
  executable('GstInstruments',
    gst_instruments_macos_sources,
    dependencies : gst_instruments_macos_deps,
    install : true
  )
  install_data('Info.plist', install_dir : 'Contents')
endif
