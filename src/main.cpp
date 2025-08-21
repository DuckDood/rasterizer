#include <SDL3/SDL.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <omp.h>
#include <vector>
#define SCR_HEIGHT 480
#define SCR_WIDTH 640



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
	float x;
	float y;
	float2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	float2() {
	}
};

class Model {
	
};

float Dot(float2 a, float2 b) {
	return a.x*b.x + a.y*b.y;
} 
float2 Perpendicular(float2 a) {
	return float2(a.y, -a.x);
} 

bool PointOnRightSideOfLine(float2 x, float2 y, float2 p) {
	float2 a(p.x-x.x, p.y-x.y);
	float2 perp = Perpendicular(float2(p.x-y.x, p.y - y.y));
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
	std::vector<std::vector<float2>> tris(12);
	std::vector<float3> triCol(12);
	std::vector<float2> currentFace;

	while(running) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
			}

		}
		SDL_RenderClear(renderer);
		//tris.clear();
		tris.at(0) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(0) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(1) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(1) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(2) ={
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(2) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(3) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(3) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(4) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(4) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(5) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(5) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(6) ={
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(6) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(7) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(7) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(8) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(8) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(9) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(9) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(10) ={
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(10) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;
		tris.at(11) = {
		randomFloat2(),
		randomFloat2(),
		randomFloat2()}
		;
		triCol.at(11) = {
		randomFloat(),
		randomFloat(),
		randomFloat()}
		;

		SDL_LockTexture(screenTex, NULL, &pix, &pitch);
		SDL_Log("hi");
		pixels = (Uint32*)pix;
		for(unsigned int y = 0; y<SCR_HEIGHT; y++) {
			for(unsigned int x = 0; x<SCR_WIDTH; x++) {
					pixels[y*SCR_WIDTH + x] = RGBToBin(0, 0, 0);
				}
			}

		for(unsigned int y = 0; y<SCR_HEIGHT; y++) {
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
		}
		SDL_UnlockTexture(screenTex);
		SDL_RenderTextureRotated(renderer, screenTex, NULL,NULL, 0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
