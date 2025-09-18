.PHONY: clean format

LIBRARIES = -lopengl32 -lglew32 -lfreeglut -lglu32 
COMPILE_FLAGS = -I include/cy -I include/libs -I include

CPP_LIBS =  include/libs/lodepng.cpp \
			include/libs/tinyxml2.cpp \
			include/headerFiles/viewport.cpp\
			include/headerFiles/xmlload.cpp




main: triangleIntersect.cpp
	g++ -g triangleIntersect.cpp $(COMPILE_FLAGS) $(CPP_LIBS) include/materials.cpp include/objects.cpp include/lights.cpp include/ray.cpp -o out $(LIBRARIES)
clean:
	rm out

format:
	clang-format -i triangleIntersect.cpp --style Microsoft