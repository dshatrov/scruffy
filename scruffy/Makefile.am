COMMON_CFLAGS =				\
	-D_POSIX_C_SOURCE=199309L	\
	-D_XOPEN_SOURCE=600		\
	-ggdb				\
	-Wno-long-long -Wall -Wextra	\
	-rdynamic			\
	$(THIS_CFLAGS)

AM_CFLAGS = -std=c99 $(COMMON_CFLAGS)
AM_CXXFLAGS = -std=gnu++0x $(COMMON_CFLAGS) #-fno-default-inline -fkeep-inline-functions

INCLUDES = -I$(top_srcdir)

PARGEN_GENFILES =		\
	cpp_pargen.h		\
	cpp_pargen.cpp		\
	cpp_cond_pargen.h	\
	cpp_cond_pargen.cpp

#BUILT_SOURCES = $(PARGEN_GENFILES)
BUILT_SOURCES = cpp_pargen.h	\
		cpp_cond_pargen.h

CLEANFILES = $(PARGEN_GENFILES)

cpp_pargen.h: cpp.par
	pargen --module-name scruffy --header-name cpp $^

cpp_cond_pargen.h: cpp_cond.par
	pargen --module-name scruffy --header-name cpp_cond $^

scruffy_private_headers =		\
	cpp.h				\
	cpp_util.h			\
	dump_context.h			\
	cpp_parser.h			\
	cpp_dispatcher.h		\
	test_dispatcher.h		\
					\
	util.h

scruffy_target_headers =		\
	parsing_exception.h		\
					\
	pp_item_stream.h		\
	unichar_pp_item_stream.h	\
	list_pp_item_stream.h		\
	phase3_item_stream.h		\
	list_token_stream.h		\
					\
	pp_item_stream_token_stream.h	\
					\
	macro_definition.h		\
	preprocessor_types.h		\
	preprocessor.h			\
					\
	checkpoint_tracker.h

bin_PROGRAMS = scruffy
scruffy_DEPENDENCIES = libscruffy-1.0.la
scruffy_SOURCES =			\
	cpp.cpp				\
	cpp_util.cpp			\
	dump_context.cpp		\
	cpp_parser.cpp			\
					\
	main.cpp			\
					\
	cpp_pargen.cpp


# MOVED TO LIBSCRUFFY
# BEGIN
#	pp_item_stream.cpp
#	unichar_pp_item_stream.cpp
#	list_pp_item_stream.cpp
#	phase3_item_stream.cpp
#
#	preprocessor.cpp
#	preprocessor_util.cpp
#
#	util.cpp
#
#	cpp_cond_pargen.cpp
# END


#	cpp_type_conversions.cpp

scruffy_LDADD = $(top_builddir)/scruffy/libscruffy-1.0.la	\
		$(THIS_LIBS)
# Adding -rpath breaks testing changes in the library without doing make install.
#scruffy_LDFLAGS = -Wl,-rpath,'$(libdir)'

lib_LTLIBRARIES = libscruffy-1.0.la
libscruffy_1_0_la_SOURCES =		\
	pp_item_stream.cpp		\
	unichar_pp_item_stream.cpp	\
	list_pp_item_stream.cpp		\
	phase3_item_stream.cpp		\
	pp_item_stream_token_stream.cpp	\
					\
	preprocessor.cpp		\
	preprocessor_util.cpp		\
					\
	checkpoint_tracker.cpp		\
					\
	util.cpp			\
					\
	cpp_cond_pargen.cpp


libscruffy_1_0_la_LDFLAGS = -no-undefined -version-info "0:0:0"
libscruffy_1_0_la_LIBADD = $(THIS_LIBS)

scruffy_includedir = $(includedir)/scruffy-1.0/scruffy
scruffy_include_HEADERS = $(scruffy_target_headers)

EXTRA_DIST = $(scruffy_private_headers)

