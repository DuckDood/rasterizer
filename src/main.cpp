#include <SDL3/SDL.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <omp.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#define SCR_HEIGHT 480
#define SCR_WIDTH 640
// yeah its from sebastian lague
// but worse in every way


class float3{
	public:
	float x = 0;
	float y = 0;
	float z = 0;
	float3 operator*(float b) {
		return float3(this->x * b, this->y * b, this->z * b);
	}
	float3 operator+(float3 b) {
		return float3(this->x + b.x, this->y + b.y, this->z + b.z);
	}
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
	//return sideAB == sideBC && sideBC == sideCA;
	return sideAB && sideBC && sideCA;
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
	float pitch = 0;
	float yaw = 0;
	float roll = 0;
	float3 position;
	//std::vector<std::vector<float3>> faces;
	std::vector<float3> triPoints;
	float3 TransformVector(float3 ihat, float3 khat, float3 jhat, float3 v) {
		return ihat * v.x + jhat * v.y + khat * v.z;
	}

	std::tuple<float3, float3, float3> GetBasisVectors() {
		float3 ihat_yaw = float3(std::cos(yaw), 0, std::sin(yaw));
		float3 jhat_yaw = float3(0, 1, 0);
		float3 khat_yaw = float3(-std::sin(yaw), 0, std::cos(yaw));

		float3 ihat_pitch = float3(1, 0, 0);
		float3 jhat_pitch = float3(0, std::cos(pitch), -std::sin(pitch));
		float3 khat_pitch = float3(0, std::sin(pitch), std::cos(pitch));

		float3 ihat = TransformVector(ihat_yaw, jhat_yaw, khat_yaw, ihat_pitch);
		float3 jhat = TransformVector(ihat_yaw, jhat_yaw, khat_yaw, jhat_pitch);
		float3 khat = TransformVector(ihat_yaw, jhat_yaw, khat_yaw, khat_pitch);

		return {ihat, jhat, khat};
	}

	float3 ToWorldPoint(float3 p) {
		std::tuple< float3, float3, float3> ihjhkh =  GetBasisVectors();
		auto [ihat, jhat, khat] = ihjhkh;
		return TransformVector(ihat, jhat, khat, p) + position;
	}

	float2 VertexToScreen(float3 vertex, float2 numPixels, float fov = 1.57) {
		float3 vertex_world = ToWorldPoint(vertex);
		int screenHeight_world = std::tan(fov/2) * 2;
		float pixelsPerWorldUnit = numPixels.y/screenHeight_world / vertex_world.z;

		float2 pixelOffset = float2(vertex_world.x * pixelsPerWorldUnit, vertex_world.y * pixelsPerWorldUnit);
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
	m.position.z = -10;
	std::string objstr = "";
	std::ifstream objfile("suzanne.obj");
	for(std::string line; std::getline(objfile, line); objstr+=line+"\n");
	m.triPoints = m.LoadObjFile(objstr);

	std::vector<float3> col;
	for(int i = 0; i<m.triPoints.size()/3; i++) {
		col.push_back({
				randomFloat(),
				randomFloat(),
				randomFloat()
				});
	}

	int frameCount = 0;
	int s = m.triPoints.size();
	float2 screen = float2(SCR_WIDTH, SCR_HEIGHT);
	enum keys{
		W = 0,
		A = 1,
		S = 2,
		D = 3,
		UP = 4,
		DOWN = 5,
		LEFT = 6,
		RIGHT = 7,
		COMMA = 8,
		PERIOD = 9
	};
	bool keyDown[10] = {false,false,false,false,false,false,false,false,false,false,};
	while(running) {
		frameCount++;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;

				case SDL_EVENT_KEY_DOWN:
					switch(event.key.scancode) {
						case SDL_SCANCODE_D:
							keyDown[D] = true;
							break;
						case SDL_SCANCODE_A:
							keyDown[A] = true;
							break;
						case SDL_SCANCODE_W:
							keyDown[W] = true;
							break;
						case SDL_SCANCODE_S:
							keyDown[S] = true;
							break;
						case SDL_SCANCODE_UP:
							keyDown[UP] = true;
							break;
						case SDL_SCANCODE_DOWN:
							keyDown[DOWN] = true;
							break;
						case SDL_SCANCODE_LEFT:
							keyDown[LEFT] = true;
							break;
						case SDL_SCANCODE_RIGHT:
							keyDown[RIGHT] = true;
							break;
						case SDL_SCANCODE_COMMA:
							keyDown[COMMA] = true;
							break;
						case SDL_SCANCODE_PERIOD:
							keyDown[PERIOD] = true;
							break;
						default:
							break;
						
					}
					break;
				case SDL_EVENT_KEY_UP:
					switch(event.key.scancode) {
						case SDL_SCANCODE_D:
							keyDown[D] = false;
							break;
						case SDL_SCANCODE_A:
							keyDown[A] = false;
							break;
						case SDL_SCANCODE_W:
							keyDown[W] = false;
							break;
						case SDL_SCANCODE_S:
							keyDown[S] = false;
							break;
						case SDL_SCANCODE_UP:
							keyDown[UP] = false;
							break;
						case SDL_SCANCODE_DOWN:
							keyDown[DOWN] = false;
							break;
						case SDL_SCANCODE_LEFT:
							keyDown[LEFT] = false;
							break;
						case SDL_SCANCODE_RIGHT:
							keyDown[RIGHT] = false;
							break;
						case SDL_SCANCODE_COMMA:
							keyDown[COMMA] = false;
							break;
						case SDL_SCANCODE_PERIOD:
							keyDown[PERIOD] = false;
							break;
						default:
							break;
						
					}
					break;
			}

		}
		if(keyDown[D]) {
			m.yaw+=0.02;
		}
		if(keyDown[A]) {
			m.yaw-=0.02;
		}
		if(keyDown[W]) {
			m.pitch-=0.02;
		}
		if(keyDown[S]) {
			m.pitch+=0.02;
		}
		if(keyDown[UP]) {
			m.position.y-=0.02;
		}
		if(keyDown[DOWN]) {
			m.position.y+=0.02;
		}
		if(keyDown[LEFT]) {
			m.position.x+=0.02;
		}
		if(keyDown[RIGHT]) {
			m.position.x-=0.02;
		}
		if(keyDown[COMMA]) {
			m.position.z+=0.02;
		}
		if(keyDown[PERIOD]) {
			m.position.z-=0.02;
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

		#pragma omp parallel for
		for(int i = 0; i<s; i+=3) {

			//currentFace = tris.at(i);
			float minX = fmin(fmin(m.VertexToScreen(m.triPoints[i], screen).x,m.VertexToScreen(m.triPoints[i+1], screen).x),m.VertexToScreen(m.triPoints[i+2], screen).x);
			float minY = fmin(fmin(m.VertexToScreen(m.triPoints[i], screen).y,m.VertexToScreen(m.triPoints[i+1], screen).y),m.VertexToScreen(m.triPoints[i+2], screen).y);
			float maxX = fmax(fmax(m.VertexToScreen(m.triPoints[i], screen).x,m.VertexToScreen(m.triPoints[i+1], screen).x),m.VertexToScreen(m.triPoints[i+2], screen).x);
			float maxY = fmax(fmax(m.VertexToScreen(m.triPoints[i], screen).y,m.VertexToScreen(m.triPoints[i+1], screen).y),m.VertexToScreen(m.triPoints[i+2], screen).y);
			for(unsigned int y = minY; y<maxY; y++) {
				for(unsigned int x = minX; x<maxX; x++) {
					if(PointInTriangle(m.VertexToScreen(m.triPoints[i], screen), m.VertexToScreen(m.triPoints[i+1], screen), m.VertexToScreen(m.triPoints[i+2], screen), float2(x,y))) {
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
