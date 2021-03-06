EXEC := voxel
OBJECTS := src/main.o
DEPENDS := $(OBJECTS:.o=.d)

CPPFLAGS := -std=c++14 -Wall -Wextra -g -Og -MMD `sdl2-config --cflags`
LDFLAGS := `sdl2-config --libs` -lGL -lGLEW

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) -o $@ $^

release: CPPFLAGS += -DNDEBUG -O3
release: all

clean:
	rm -f $(EXEC) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)
