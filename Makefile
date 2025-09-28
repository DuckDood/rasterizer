all: obj/ build/ obj/main.o build/raster 

clean:
	rm -r obj/ build/ 
.PHONY: clean

obj/: 
	mkdir -p obj/

build/: 
	mkdir -p build/

ifndef SDLIMG
SDLIMG := 1
endif
obj/main.o: src/main.cpp
	${CXX} src/main.cpp -c -o obj/main.o -DSDLIMG=${SDLIMG}  -O2 

build/raster: obj/main.o
ifeq (${SDLIMG},1)
	g++ obj/main.o -lSDL3 -lSDL3_image -o build/raster
else
	g++ obj/main.o -lSDL3 -o build/raster
endif

