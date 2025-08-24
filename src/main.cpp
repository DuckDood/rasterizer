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
		return TransformVector(ihat, jhat, khat, p);
	}

	float2 VertexToScreen(float3 vertex, float2 numPixels) {
		float3 vertex_world = ToWorldPoint(vertex);
		int screenHeight_world = 5;
		float pixelsPerWorldUnit = numPixels.y/screenHeight_world;

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
	std::vector<float3> triPoints = m.LoadObjFile("\
			# Blender 4.5.2 LTS\n\
# www.blender.org\n\
mtllib bevel.mtl\n\
o Cube\n\
v 0.568232 0.568232 -1.000000\n\
v 0.568232 1.000000 -0.568232\n\
v 1.000000 0.568232 -0.568232\n\
v 0.568232 -1.000000 -0.568232\n\
v 0.568232 -0.568232 -1.000000\n\
v 1.000000 -0.568232 -0.568232\n\
v 1.000000 0.568232 0.568232\n\
v 0.568232 1.000000 0.568232\n\
v 0.568232 0.568232 1.000000\n\
v 1.000000 -0.568232 0.568232\n\
v 0.568232 -0.568232 1.000000\n\
v 0.568232 -1.000000 0.568232\n\
v -0.568232 0.568232 -1.000000\n\
v -1.000000 0.568232 -0.568232\n\
v -0.568232 1.000000 -0.568232\n\
v -1.000000 -0.568232 -0.568232\n\
v -0.568232 -0.568232 -1.000000\n\
v -0.568232 -1.000000 -0.568232\n\
v -1.000000 0.568232 0.568232\n\
v -0.568232 0.568232 1.000000\n\
v -0.568232 1.000000 0.568232\n\
v -0.568232 -1.000000 0.568232\n\
v -0.568232 -0.568232 1.000000\n\
v -1.000000 -0.568232 0.568232\n\
vn 0.5774 0.5774 -0.5774\n\
vn 0.5774 -0.5774 -0.5774\n\
vn 0.5774 0.5774 0.5774\n\
vn 0.5774 -0.5774 0.5774\n\
vn -0.5774 0.5774 -0.5774\n\
vn -0.5774 -0.5774 -0.5774\n\
vn -0.5774 0.5774 0.5774\n\
vn -0.5774 -0.5774 0.5774\n\
vn -0.7071 -0.7071 -0.0000\n\
vn -0.0000 -0.7071 -0.7071\n\
vn 0.7071 -0.0000 -0.7071\n\
vn -0.7071 -0.0000 0.7071\n\
vn 0.7071 -0.0000 0.7071\n\
vn -0.7071 -0.0000 -0.7071\n\
vn -0.0000 0.7071 0.7071\n\
vn 0.7071 0.7071 -0.0000\n\
vn -0.0000 -0.7071 0.7071\n\
vn -0.7071 0.7071 -0.0000\n\
vn -0.0000 0.7071 -0.7071\n\
vn 0.7071 -0.7071 -0.0000\n\
vn -0.0000 -0.0000 -1.0000\n\
vn 1.0000 -0.0000 -0.0000\n\
vn -0.0000 1.0000 -0.0000\n\
vn -0.0000 -1.0000 -0.0000\n\
vn -0.0000 -0.0000 1.0000\n\
vn -1.0000 -0.0000 -0.0000\n\
vt 0.571029 0.446029\n\
vt 0.662720 0.521470\n\
vt 0.571029 0.550904\n\
vt 0.321029 0.553971\n\
vt 0.396470 0.462280\n\
vt 0.425904 0.553971\n\
vt 0.571029 0.699096\n\
vt 0.662720 0.728530\n\
vt 0.571029 0.803971\n\
vt 0.425904 0.696029\n\
vt 0.396470 0.787720\n\
vt 0.321029 0.696029\n\
vt 0.571029 0.300904\n\
vt 0.571029 0.196029\n\
vt 0.625000 0.244376\n\
vt 0.125000 0.553971\n\
vt 0.178971 0.500000\n\
vt 0.178971 0.553971\n\
vt 0.571029 0.053971\n\
vt 0.571029 0.000000\n\
vt 0.625000 0.053971\n\
vt 0.178971 0.696029\n\
vt 0.178971 0.750000\n\
vt 0.125000 0.696029\n\
vt 0.428971 0.000000\n\
vt 0.428971 0.053971\n\
vt 0.428971 0.303971\n\
vt 0.428971 0.196029\n\
vt 0.625000 0.946029\n\
vt 0.571029 0.946029\n\
vt 0.821029 0.553971\n\
vt 0.821029 0.696029\n\
vt 0.428971 0.946029\n\
s 0\n\
usemtl Material\n\
f 1/1/1 2/2/1 3/3/1\n\
f 4/4/2 5/5/2 6/6/2\n\
f 7/7/3 8/8/3 9/9/3\n\
f 10/10/4 11/11/4 12/12/4\n\
f 13/13/5 14/14/5 15/15/5\n\
f 16/16/6 17/17/6 18/18/6\n\
f 19/19/7 20/20/7 21/21/7\n\
f 22/22/8 23/23/8 24/24/8\n\
f 18/18/9 22/22/9 24/24/9 16/16/9\n\
f 4/4/10 18/18/10 17/17/10 5/5/10\n\
f 3/3/11 6/6/11 5/5/11 1/1/11\n\
f 23/25/12 20/20/12 19/19/12 24/26/12\n\
f 9/9/13 11/11/13 10/10/13 7/7/13\n\
f 13/13/14 17/27/14 16/28/14 14/14/14\n\
f 8/8/15 21/29/15 20/30/15 9/9/15\n\
f 2/2/16 8/8/16 7/7/16 3/3/16\n\
f 22/22/17 12/12/17 11/11/17 23/23/17\n\
f 21/21/18 15/15/18 14/14/18 19/19/18\n\
f 15/15/19 2/2/19 1/1/19 13/13/19\n\
f 12/12/20 4/4/20 6/6/20 10/10/20\n\
f 17/27/21 13/13/21 1/1/21 5/5/21\n\
f 6/6/22 3/3/22 7/7/22 10/10/22\n\
f 2/2/23 15/31/23 21/32/23 8/8/23\n\
f 18/18/24 4/4/24 12/12/24 22/22/24\n\
f 11/11/25 9/9/25 20/30/25 23/33/25\n\
f 24/26/26 19/19/26 14/14/26 16/28/26\n\
\n\
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
		int s = triPoints.size();
		float2 screen = float2(SCR_WIDTH, SCR_HEIGHT);
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
							m.yaw-=0.1;
							break;
						case SDL_SCANCODE_A:
							m.yaw+=0.1;
							break;
						case SDL_SCANCODE_W:
							m.pitch-=0.1;
							break;
						case SDL_SCANCODE_S:
							m.pitch+=0.1;
							break;
						default:
							break;
						
					}
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
