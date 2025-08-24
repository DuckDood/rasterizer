all: obj/ build/ obj/main.o build/raster 

clean:
	rm -r obj/ build/ 
.PHONY: clean

obj/: 
	mkdir -p obj/

build/: 
	mkdir -p build/

obj/main.o: src/main.cpp
	${CXX} src/main.cpp -c -o obj/main.o  -O2 

build/raster: obj/main.o 
	${CXX} obj/main.o  -o build/raster -lSDL3  

