CXXFLAGS = -std=c++11

ArgSpecExample: ArgSpec.cpp ArgSpec.hpp ArgSpecExample.cpp
	$(CXX) $(CXXFLAGS) -o $@ ArgSpecExample.cpp

test: ArgSpecExample
	@./ArgSpecExample -h brief > test.txt
	@./ArgSpecExample      >> test.txt || true
	@./ArgSpecExample simple   >> test.txt
	@./ArgSpecExample -v4 3 2 -cats on -gamma 2.4 mainArg -words what on earth \
        -colours red blue black green -v3s 1 2 3 4 5 6 7 8 9 10 -counts 1 \
        -countArray "1 2 3 4 5" /tmp -colour red -v3 888 -v2 1 0 -v -size 999 \
        -latLong 30 40 -v3s 1 2 3 4 5 6 7 8 9 -scale 0.333 >> test.txt
	@diff test.txt test-ref.txt

clean:
	$(RM) ArgSpecExample
