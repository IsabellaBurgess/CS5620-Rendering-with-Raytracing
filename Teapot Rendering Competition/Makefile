.PHONY: clean format

LIBRARIES = -lopengl32 -lglew32 -lfreeglut -lglu32 
COMPILE_FLAGS = -I include/cy  -I include/libs -I include/headerFiles

CPP_LIBS =  include/libs/lodepng.cpp \
			include/libs/tinyxml2.cpp \
			include/headerFiles/viewport.cpp\
			include/headerFiles/xmlload.cpp\
			include/headerFiles/texture.cpp





main: main.cpp
	g++ -o3 main.cpp $(COMPILE_FLAGS) $(CPP_LIBS) include/ray.cpp include/materials.cpp include/shadeInf.cpp include/objects.cpp include/lights.cpp -g -o out $(LIBRARIES)
clean:
	rm out

format:
	clang-format -i main.cpp --style Microsoft