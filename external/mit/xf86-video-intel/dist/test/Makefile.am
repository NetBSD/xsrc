stress_TESTS = \
	basic-fillrect \
	basic-rectangle \
	basic-string \
	basic-copyarea \
	basic-copyarea-size \
	basic-putimage \
	basic-lines \
	basic-stress \
	render-fill \
	render-trapezoid \
	render-trapezoid-image \
	render-fill-copy \
	render-composite-solid \
	render-copyarea \
	render-copyarea-size \
	render-copy-alphaless \
	mixed-stress \
	dri2-swap \
	dri2-race \
	$(NULL)

check_PROGRAMS = $(stress_TESTS)

noinst_PROGRAMS = lowlevel-blt-bench

AM_CFLAGS = @CWARNFLAGS@ @X11_CFLAGS@ @DRM_CFLAGS@
LDADD = libtest.la @X11_LIBS@ -lXfixes @DRM_LIBS@ @CLOCK_GETTIME_LIBS@

noinst_LTLIBRARIES = libtest.la
libtest_la_SOURCES = \
	test.h \
	test_display.c \
	test_image.c \
	test_log.c \
	test_render.c \
	dri2.c \
	dri2.h \
	$(NULL)

vsync.avi: mkvsync.sh
	./mkvsync.sh $@

clean-vsync-avi:
	rm -rf vsync.avi .build.tmp

EXTRA_DIST = README mkvsync.sh tearing.mp4
clean-local: clean-vsync-avi
