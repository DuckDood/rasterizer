#include <SDL3/SDL.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <omp.h>
#include <string>
#include <vector>
#include <sstream>
#define SCR_HEIGHT 480
#define SCR_WIDTH 640
// yeah its from sebastian lague
// but worse in every way


class float3{
	public:
	float x;
	float y;
	float z;
	float3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	float3() {
	}
};

class float2{
	public:
	float2 operator+(float2 b) {
		return float2(this->x + b.x, this->y + b.y);
	}
	float2 operator-(float2 b) {
		return float2(this->x - b.x, this->y - b.y);
	}
	float2 operator*(float2 b) {
		return float2(this->x * b.x, this->y * b.y);
	}
	float x;
	float y;
	float2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	float2() {
	}
};



float Dot(float2 a, float2 b) {
	return a.x*b.x + a.y*b.y;
} 
float2 Perpendicular(float2 a) {
	return float2(a.y, -a.x);
} 

bool PointOnRightSideOfLine(float2 x, float2 y, float2 p) {
	//float2 a(p.x-x.x, p.y-x.y);
	float2 a = p-x;
	//float2 perp = Perpendicular(float2(p.x-y.x, p.y - y.y));
	float2 perp = Perpendicular(p-y);
	return Dot(a, perp) >= 0;
}


bool PointInTriangle(float2 a, float2 b, float2 c, float2 p) {
	bool sideAB = PointOnRightSideOfLine(a,b,p);	
	bool sideBC = PointOnRightSideOfLine(b,c,p);		
	bool sideCA = PointOnRightSideOfLine(c,a,p);	
	return sideAB == sideBC && sideBC == sideCA;
}

bool inTriBounds(float2 a, float2 b, float2 c, float2 p) {
	float minX = fmin(fmin(a.x, b.x), c.x);
	float minY = fmin(fmin(a.y, b.y), c.y);
	float maxX = fmax(fmax(a.x, b.x), c.x);
	float maxY = fmax(fmax(a.y, b.y), c.y);
	return p.x>=minX && p.x <= maxX && p.y >= minY && p.y <= maxY;
}

Uint32 RGBToBin(int r, int g, int b) {
	Uint32 tempC;
	tempC = 0xff000000;
	tempC += r << 16; 
	tempC += g << 8; 
	tempC += b << 0; 
	return tempC;
}

float2 randomFloat2() {
	return float2(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
}
float randomFloat() {
	return rand() % 255;
}

class Camera {
	float pitch;
	float yaw;
	float roll;

	float3 position;
};

class Model {
	public:
	float pitch;
	float yaw;
	float roll;
	//std::vector<std::vector<float3>> faces;
	std::vector<float3> triPoints;
	float2 VertexToScreen(float3 vertex, float2 numPixels) {
		int screenHeight_world = 5;
		float pixelsPerWorldUnit = numPixels.y/screenHeight_world;

		float2 pixelOffset = float2(vertex.x * pixelsPerWorldUnit, vertex.y * pixelsPerWorldUnit);
		return float2(numPixels.x /2 + pixelOffset.x, numPixels.y/2 + pixelOffset.y);
	}

	std::vector<float3> LoadObjFile(std::string obj) {
		std::vector<float3> allPoints;
		std::vector<float3> triPoints;

		std::stringstream lines(obj);
		std::string line;

		while(std::getline(lines, line, '\n')) {
			if(line.length() < 2) continue;
			if(line.substr(0, 2) == "v ") {
				std::vector<float> axes;
				std::stringstream words(line);
				std::string word;
				int i = 2;
				while(std::getline(words, word, ' ')) {
					if(i > 2) {
						axes.push_back(std::stof(word));
					}
					i++;
				}
				allPoints.push_back({axes[0], axes[1], axes[2]});
			} else
			if(line.substr(0, 2) == "f ") {
				std::stringstream words(line);
				std::string word;
				std::vector<std::string> faceIndexGroups;
				int c = 2;
				while(std::getline(words, word, ' ')) {
					if(c > 2) {
						faceIndexGroups.push_back(word);
					}
					c++;
				}

				for(int i = 0; i<faceIndexGroups.size(); i++) {
					std::vector<int> indexGroup;
					std::stringstream thisFaceGroup(faceIndexGroups[i]);
					while(std::getline(thisFaceGroup, word, '/')) {
						indexGroup.push_back(std::stoi(word));
					}
					int pointIndex = indexGroup[0]-1;
					if(i>=3) triPoints.push_back(triPoints[triPoints.size()-(3 * i - 6)]);
					if(i>=3) triPoints.push_back(triPoints[triPoints.size()-(2)]);
					triPoints.push_back(allPoints[pointIndex]);
				}

			}
		} 
		return triPoints;
	}
};

int main() {
	srand(time(0));
	if(!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("%s", SDL_GetError());
	}
	SDL_Window* window = SDL_CreateWindow("rasterizer", SCR_WIDTH,SCR_HEIGHT, 0);
	if(window == NULL) {
		SDL_Log("Failed to create window: %s", SDL_GetError());
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if(renderer == NULL) {
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
	}

	bool running = true;
	SDL_Event event;
	SDL_Texture* screenTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);
	Uint32 *pixels;
	void* pix;
	int pitch;
	std::vector<float2> currentFace;
	Model m;
	std::vector<float3> triPoints = m.LoadObjFile("\
mtllib cube.mtl\n\
o Cube\n\
v 1.000000 -1.000000 -1.000000\n\
v 1.000000 -1.000000 1.000000\n\
v -1.000000 -1.000000 1.000000\n\
v -1.000000 -1.000000 -1.000000\n\
v 1.000000 1.000000 -0.999999\n\
v 0.999999 1.000000 1.000001\n\
v -1.000000 1.000000 1.000000\n\
v -1.000000 1.000000 -1.000000\n\
vt 1.000000 0.333333\n\
vt 1.000000 0.666667\n\
vt 0.666667 0.666667\n\
vt 0.666667 0.333333\n\
vt 0.666667 0.000000\n\
vt 0.000000 0.333333\n\
vt 0.000000 0.000000\n\
vt 0.333333 0.000000\n\
vt 0.333333 1.000000\n\
vt 0.000000 1.000000\n\
vt 0.000000 0.666667\n\
vt 0.333333 0.333333\n\
vt 0.333333 0.666667\n\
vt 1.000000 0.000000\n\
vn 0.000000 -1.000000 0.000000\n\
vn 0.000000 1.000000 0.000000\n\
vn 1.000000 0.000000 0.000000\n\
vn -0.000000 0.000000 1.000000\n\
vn -1.000000 -0.000000 -0.000000\n\
vn 0.000000 0.000000 -1.000000\n\
usemtl Material\n\
s off\n\
f 2/1/1 3/2/1 4/3/1\n\
f 8/1/2 7/4/2 6/5/2\n\
f 5/6/3 6/7/3 2/8/3\n\
f 6/8/4 7/5/4 3/4/4\n\
f 3/9/5 7/10/5 8/11/5\n\
f 1/12/6 4/13/6 8/11/6\n\
f 1/4/1 2/1/1 4/3/1\n\
f 5/14/2 8/1/2 6/5/2\n\
f 1/12/3 5/6/3 2/8/3\n\
f 2/12/4 6/8/4 3/4/4\n\
f 4/13/5 3/9/5 8/11/5\n\
f 5/6/6 1/12/6 8/11/6\n\
				");

	std::vector<float3> col;
	for(int i = 0; i<triPoints.size()/3; i++) {
		col.push_back({
				randomFloat(),
				randomFloat(),
				randomFloat()
				});
	}

	int frameCount = 0;
	while(running) {
		frameCount++;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
			}

		}
		SDL_Log("%d", frameCount);
		SDL_RenderClear(renderer);
		//tris.clear();

		SDL_LockTexture(screenTex, NULL, &pix, &pitch);
		pixels = (Uint32*)pix;
		for(unsigned int y = 0; y<SCR_HEIGHT; y++) {
			for(unsigned int x = 0; x<SCR_WIDTH; x++) {
				pixels[y*SCR_WIDTH + x] = RGBToBin(0, 0, 0);
			}
		}

		int s = triPoints.size();
		float2 screen = float2(SCR_WIDTH, SCR_HEIGHT);
		#pragma omp parallel for
		for(int i = 0; i<s; i+=3) {

			//currentFace = tris.at(i);
			float minX = fmin(fmin(m.VertexToScreen(triPoints[i], screen).x,m.VertexToScreen(triPoints[i+1], screen).x),m.VertexToScreen(triPoints[i+2], screen).x);
			float minY = fmin(fmin(m.VertexToScreen(triPoints[i], screen).y,m.VertexToScreen(triPoints[i+1], screen).y),m.VertexToScreen(triPoints[i+2], screen).y);
			float maxX = fmax(fmax(m.VertexToScreen(triPoints[i], screen).x,m.VertexToScreen(triPoints[i+1], screen).x),m.VertexToScreen(triPoints[i+2], screen).x);
			float maxY = fmax(fmax(m.VertexToScreen(triPoints[i], screen).y,m.VertexToScreen(triPoints[i+1], screen).y),m.VertexToScreen(triPoints[i+2], screen).y);
			for(unsigned int y = minY; y<maxY; y++) {
				for(unsigned int x = minX; x<maxX; x++) {
					if(PointInTriangle(m.VertexToScreen(triPoints[i], screen), m.VertexToScreen(triPoints[i+1], screen), m.VertexToScreen(triPoints[i+2], screen), float2(x,y))) {
						pixels[y*SCR_WIDTH + x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
					}
				}
			}
		}
		/*for(unsigned int y = 0; y<SCR_HEIGHT; y++) {
			for(unsigned int x = 0; x<SCR_WIDTH; x++) {
				//tempC += (123 >> 16); 
				//tempC += (43 >> 24); 
				int s = tris.size();
				for(int i = 0; i<s; i++) {

					currentFace = tris.at(i);

					if(inTriBounds(currentFace[0],currentFace[1],currentFace[2],float2(x,y))) {
						if(PointInTriangle(currentFace[0], currentFace[1], currentFace[2], float2(x,y))) {
							pixels[y*SCR_WIDTH + x] = RGBToBin(triCol.at(i).x, triCol.at(i).y, triCol.at(i).z);
						}
					}
				}
			}
		}*/
		SDL_UnlockTexture(screenTex);
		SDL_RenderTextureRotated(renderer, screenTex, NULL,NULL, 0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
