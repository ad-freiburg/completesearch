include ../Makefile

# Build gtest with our CXX
build:
	$(MAKE) libgtest.a
	$(MAKE) libgtest_main.a

libgtest.a: gtest-all.o
	ar -rv $@ $<

libgtest_main.a: gtest_main.o
	ar -rv $@ $<

%.o: src/%.cc
	$(CXX) -isystem include -I. -o $@ -c $<

clean:
	rm -f *.o
	rm -f *.a
	rm -f src/*.o
	rm -f src/*.lo
	rm -rf src/.libs
	rm -rf src/.deps
	rm -rf src/.dirstamp
	rm -f a.out
