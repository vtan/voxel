CPPFLAGS := -std=c++14 -Wall -Wextra -g -Og `sdl2-config --cflags`
LDFLAGS := `sdl2-config --libs` -lGL -lGLEW

voxel: src/main.o
	$(CXX) $(LDFLAGS) src/main.o $(LOADLIBES) $(LDLIBS) -o voxel

src/main.o: src/main.cpp src/uniform.hpp src/camera.hpp src/volume.hpp \
    src/mesh.hpp src/mesh_builder.hpp src/voxel.hpp src/chunk.hpp \
    src/chunk_volume_repository.hpp src/chunk_mesh_repository.hpp \
    src/volume_sampler.hpp

clean:
	rm -f voxel
	rm -f src/*.o
