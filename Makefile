CPPFLAGS := -std=c++14 -Wall -Wextra `sdl2-config --cflags`
LDFLAGS := `sdl2-config --libs` -lGL -lGLEW

voxel: src/main.o
	$(CXX) $(LDFLAGS) src/main.o $(LOADLIBES) $(LDLIBS) -o voxel

clean:
	rm -f voxel
	rm -f src/*.o
