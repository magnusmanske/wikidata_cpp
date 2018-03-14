# sudo apt-get gcc-6 g++-6 libcurl4-gnutls-dev
# Set up googletest, see https://stackoverflow.com/a/13978127
# Set GTEST_DIR env var, like so: export GTEST_DIR=/home/magnus/googletest-release-1.8.0/googletest
# Tested with gcc-6/g++-6

CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-std=c++14 -I${GTEST_DIR}/include -Isrc/shared -g
LDFLAGS=-pthread -DMG_ENABLE_THREADS
LDLIBS=${GTEST_DIR}/libgtest.a  -lcurl

#-O3 `mysql_config --include`


SRCS=src/shared/wikibase_api.cpp src/shared/wikibase_entity.cpp src/shared/wikibase_entities.cpp src/tests/tests.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: tests

tests: $(OBJS)
	$(CXX) $(LDFLAGS) -o tests $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS) tests

distclean: clean
	$(RM) *~ .depend

include .depend
