require 'mkmf'

LIBDIR      = Config::CONFIG['libdir']
INCLUDEDIR  = Config::CONFIG['includedir']

HEADER_DIRS = [
  # First search /opt/local for macports
  '/opt/local/include',

  # Then search /usr/local for people that installed from source
  '/usr/local/include',

  # Check the ruby install locations
  INCLUDEDIR,

  # Finally fall back to /usr
  '/usr/include',
]

LIB_DIRS = [
  # First search /opt/local for macports
  '/opt/local/lib',

  # Then search /usr/local for people that installed from source
  '/usr/local/lib',

  # Check the ruby install locations
  LIBDIR,

  # Finally fall back to /usr
  '/usr/lib',
]

dir_config('rsox', HEADER_DIRS, LIB_DIRS)

if have_library("sox", "sox_format_init") && have_header("sox.h")
    create_makefile('rsox')
end
