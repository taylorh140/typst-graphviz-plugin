AM_CPPFLAGS = \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/cdt \
  -I$(top_srcdir)/lib/cgraph \
  -I$(top_srcdir)/lib/common \
  -I$(top_srcdir)/lib/gvc \
  -I$(top_srcdir)/lib/pathplan

noinst_LTLIBRARIES = libgvplugin_vt100_C.la
if WITH_WIN32
lib_LTLIBRARIES = libgvplugin_vt100.la
else
pkglib_LTLIBRARIES = libgvplugin_vt100.la
endif

libgvplugin_vt100_C_la_SOURCES = gvplugin_vt100.c

libgvplugin_vt100_la_LDFLAGS = -version-info $(GVPLUGIN_VERSION_INFO)
libgvplugin_vt100_la_SOURCES = $(libgvplugin_vt100_C_la_SOURCES)
libgvplugin_vt100_la_LIBADD = \
  $(top_builddir)/lib/gvc/libgvc.la \
  $(top_builddir)/lib/cgraph/libcgraph.la \
  $(top_builddir)/lib/cdt/libcdt.la

if WITH_WIN32
libgvplugin_vt100_la_LDFLAGS += -no-undefined
endif

if WITH_DARWIN9
libgvplugin_vt100_la_LDFLAGS += -Wl,-exported_symbol,_gvplugin_vt100_LTX_library
endif
