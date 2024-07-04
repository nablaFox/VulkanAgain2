# paths
SRC_PATH = src
BIN_PATH = bin
SHADERS_PATH = shaders
DEMO_PATH = demos/general_demo

# libs
LIBS_INCLUDE = glm glfw/include vkbootstrap vma/include stb_image fastgltf fastgltf/include fmt/include entt
LIBS_BIN = lib/glfw/src/libglfw3.a lib/fmt/libfmt.a lib/fastgltf/libfastgltf.a

# tool
CXX = g++
CFLAGS = -std=c++20 -Wall -Wextra -Wpedantic $(addprefix -Ilib/, $(LIBS_INCLUDE))
LDFLAGS = $(LIBS_BIN) -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lvulkan
GLSLC = glslc

# libs manual source files
LIBS_SRC = lib/vkbootstrap/VkBootstrap.cpp

# files
SRC = $(wildcard $(SRC_PATH)/*.cpp) $(wildcard $(DEMO_PATH)/*.cpp) $(LIBS_SRC)
OBJ = $(SRC:.cpp=.o)
BIN = $(BIN_PATH)/demo
SHADERS = $(wildcard $(SHADERS_PATH)/*.vert) $(wildcard $(SHADERS_PATH)/*.frag) $(wildcard $(SHADERS_PATH)/*.comp)
SHADERS_OBJ = $(SHADERS:.vert=.vert.spv) $(SHADERS:.frag=.frag.spv) $(SHADERS:.comp=.comp.spv)

# rules
.PHONY: all clean

all: libs shaders demo

libs:
	cd lib/glm && cmake . && make
	cd lib/glfw && cmake . && make
	cd lib/fmt && cmake . && make
	cd lib/fastgltf && cmake . && make

demo: $(OBJ)
	mkdir -p $(BIN_PATH)
	$(CXX) -o $(BIN) $^ $(LDFLAGS)

shaders: $(SHADERS_OBJ)

%.comp.spv: %.comp
	$(GLSLC) $< -o $@

%.vert.spv: %.vert
	$(GLSLC) $< -o $@

%.frag.spv: %.frag
	$(GLSLC) $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(SHADERS_PATH)/*.spv $(BIN)
