INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ $(LOCAL_OPTS) -DSYSCONFDIR=\"$(sysconfdir)/\"

noinst_PROGRAMS = testsig testbuffer testmemory heavy heavywq heavythread \
		aspathtest testprivs teststream
testsig_SOURCES = test-sig.c
testbuffer_SOURCES = test-buffer.c
testmemory_SOURCES = test-memory.c
testprivs_SOURCES = test-privs.c
teststream_SOURCES = test-stream.c
heavy_SOURCES = heavy.c main.c
heavywq_SOURCES = heavy-wq.c main.c
heavythread_SOURCES = heavy-thread.c main.c
aspathtest_SOURCES = aspath_test.c

testsig_LDADD = ../lib/librtm.la @LIBCAP@
testbuffer_LDADD = ../lib/librtm.la @LIBCAP@
testmemory_LDADD = ../lib/librtm.la @LIBCAP@
testprivs_LDADD = ../lib/librtm.la @LIBCAP@
teststream_LDADD = ../lib/librtm.la @LIBCAP@
heavy_LDADD = ../lib/librtm.la @LIBCAP@ -lm
heavywq_LDADD = ../lib/librtm.la @LIBCAP@ -lm
heavythread_LDADD = ../lib/librtm.la @LIBCAP@ -lm
aspathtest_LDADD = ../lib/librtm.la @LIBCAP@ -lm ../bgpd/libbgp.a
