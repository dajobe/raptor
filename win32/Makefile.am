EXTRA_DIST=README.txt \
raptor.dsp raptor.dsw \
raptortest.cpp raptortest.dsp

# Make some empty dirs that may be needed
dist-hook:
	for dir in Debug Release; do \
	  mkdir $(distdir)/$$dir; \
        done
