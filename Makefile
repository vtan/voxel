CPPFLAGS := -std=c++14 -Wall -Wextra -g `sdl2-config --cflags`
LDFLAGS := `sdl2-config --libs` -lGL -lGLEW

voxel: src/main.o
	$(CXX) $(LDFLAGS) src/main.o $(LOADLIBES) $(LDLIBS) -o voxel

src/main.o: src/main.cpp src/uniform.hpp src/camera.hpp src/volume.hpp \
    src/mesh.hpp src/mesh_builder.hpp src/voxel.hpp

clean:
	rm -f voxel
	rm -f src/*.o
