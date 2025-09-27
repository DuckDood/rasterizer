#include <SDL3/SDL.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <omp.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#ifndef SDLIMG
#define SDLIMG 1
#endif

#if SDLIMG == 1
#include <SDL3_image/SDL_image.h>
#endif
//#define SCR_HEIGHT 720
//#define SCR_WIDTH 1280
//#define SCR_HEIGHT 540
//#define SCR_WIDTH 960
//#define SCR_HEIGHT 480
//#define SCR_WIDTH 640
#define SCR_HEIGHT 240
#define SCR_WIDTH 320
//#define SCR_HEIGHT 120
//#define SCR_WIDTH 160
// yeah its from sebastian lague
// but worse in every way


class float2{
	public:
	float2 operator+(float2 b) {
		return float2(this->x + b.x, this->y + b.y);
	}
	float2 operator-(float2 b) {
		return float2(this->x - b.x, this->y - b.y);
	}
		#pragma omp parallel for
	float2 operator*(float2 b) {
		return float2(this->x * b.x, this->y * b.y);
	}
	float2 operator*(float b) {
		return float2(this->x * b, this->y * b);
	}
	float2 operator/(float b) {
		return float2(this->x / b, this->y / b);
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
class float3{
	public:
	float x = 0;
	float y = 0;
	float z = 0;
	operator float2() const {
		return float2(this->x, this->y);
	}
	float3 operator*(float b) {
		return float3(this->x * b, this->y * b, this->z * b);
	}
	float3 operator*(float3 b) {
		return float3(this->x * b.x, this->y * b.y, this->z * b.z);
	}
	float3 operator/(float b) {
		return float3(this->x / b, this->y / b, this->z / b);
	}
	float3 operator+(float3 b) {
		return float3(this->x + b.x, this->y + b.y, this->z + b.z);
	}
	/*void operator+=(float3 b) {
		*this = *this + b;
	}*/
	float3 operator-(float3 b) {
		return float3(this->x - b.x, this->y - b.y, this->z - b.z);
	}
	float3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	float3() {
	}
};



inline float Dot(float2 a, float2 b) {
	return a.x*b.x + a.y*b.y;
} 
inline float Dot3(float3 a, float3 b) {
	return a.x*b.x + a.y*b.y + a.z * b.z;
} 
inline float2 Perpendicular(float2 a) {
	return float2(a.y, -a.x);
} 

inline float3 Normalize(float3 v)
{
		float sqrLength = Dot3(v, v);
		float length = sqrt(sqrLength);
		if (length == 0) return {0,0,0};
		return v / length;
}

/*bool PointOnRightSideOfLine(float2 x, float2 y, float2 p) {
	//float2 a(p.x-x.x, p.y-x.y);
	float2 a = p-x;
	//float2 perp = Perpendicular(float2(p.x-y.x, p.y - y.y));
	float2 perp = Perpendicular(p-y);
	return Dot(a, perp) >= 0;
}*/

inline float SignedTriangleArea(float2 a, float2 b, float2 c) {
	float2 ac = c - a;
	float2 abPerp = Perpendicular(b-a);//
	return Dot(ac, abPerp) * 0.5;        //
                                       //
}
		


inline bool PointInTriangle(float2 a, float2 b, float2 c, float2 p, float3& weights) {
	/*bool sideAB = PointOnRightSideOfLine(a,b,p);	
	bool sideBC = PointOnRightSideOfLine(b,c,p);		
	bool sideCA = PointOnRightSideOfLine(c,a,p);	*/
	float areaABP = SignedTriangleArea(a,b,p);
	float areaBCP = SignedTriangleArea(b,c,p);
	float areaCAP = SignedTriangleArea(c,a,p);

	bool inTri = (areaABP >= 0) && (areaBCP >= 0) && (areaCAP >= 0);

	float totalArea = areaABP + areaBCP + areaCAP;

	float invAreaSum = 1 / totalArea;
	float weightA = areaBCP * invAreaSum;
	float weightB = areaCAP * invAreaSum;
	float weightC = areaABP * invAreaSum;
	weights = float3(weightA, weightB, weightC);
	//return sideAB == sideBC && sideBC == sideCA;
	//return sideAB && sideBC && sideCA;
	return inTri && totalArea > 0;
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


/*float2 randomFloat2() {
	return float2(rand() % SCR_WIDTH, rand() % SCR_HEIGHT);
}
float randomFloat() {
	return rand() % 255;
}*/

void sampleSurface(float x, float y, SDL_Surface* surface, Uint8 * r, Uint8 * g, Uint8 * b) {
	// theres a problem with ceiling stuff going outside the ranges hopefully i fix that at some point
	Uint8 tempR, tempG, tempB;
	Uint8 tempRt = 0, tempGt = 0, tempBt = 0;
	Uint8 tempRs = 0, tempGs = 0, tempBs = 0;
	float yWeight = y*surface->h - floor(y*surface->h);
	float xWeight = x*surface->w - floor(x*surface->w);
	SDL_ReadSurfacePixel(surface, floor(x*surface->w), floor(y*surface->h), &tempR, &tempG, &tempB, NULL);
	tempRt += tempR * ( 1- yWeight);
	tempGt += tempG * ( 1- yWeight);
	tempBt += tempB * ( 1- yWeight);
	SDL_ReadSurfacePixel(surface, floor(x*surface->w), ceil(y*surface->h), &tempR, &tempG, &tempB, NULL);
	tempRt += tempR * yWeight;
	tempGt += tempG * yWeight;
	tempBt += tempB * yWeight;


	SDL_ReadSurfacePixel(surface, ceil(x*surface->w), floor(y*surface->h), &tempR, &tempG, &tempB, NULL);
	tempRs += tempR * (1 - yWeight);
	tempGs += tempG * (1 - yWeight);
	tempBs += tempB * (1 - yWeight);
	SDL_ReadSurfacePixel(surface, ceil(x*surface->w), ceil(y*surface->h), &tempR, &tempG, &tempB, NULL);
	tempRs += tempR * yWeight;
	tempGs += tempG * yWeight;
	tempBs += tempB * yWeight;

	*r = (tempRt * (1-xWeight)) + (tempRs * (xWeight));
	*g = (tempGt * (1-xWeight)) + (tempGs * (xWeight));
	*b = (tempBt * (1-xWeight)) + (tempBs * (xWeight));

}

void getSurfacePixel(SDL_Surface * surface, int x, int y, Uint8 * r, Uint8 * g, Uint8 * b) {
	// assume texture is locked
	// why did i add comment above this it makes it look like i used chatgpt but i swear i didnt
	// here at least some bull somewhere used gemini when i couldnt be fricked
	Uint32* pixels = (Uint32*)surface->pixels;
	int bpp = SDL_BYTESPERPIXEL(surface->format);
	Uint32 pixel_value = pixels[((y * surface->pitch) + (x))/bpp];

	SDL_GetRGBA(pixel_value, SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), r, g, b, NULL);
}


class Transform {
	public:
	float pitch = 0;
	float yaw = 0;
	float roll = 0;
	float3 position = {0,0,0};
	float3 scale = {1,1,1};

	float3 ihat, khat, jhat;
	float3 ihat_inv, khat_inv, jhat_inv;
	//std::vector<std::vector<float3>> faces;
	//std::vector<float3> triPoints;
	Transform* parent = NULL;
	Transform() {
		UpdateRotation();
	}
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

		float3 ihat_roll = float3(std::cos(roll), std::sin(roll), 0);
		float3 jhat_roll = float3(-std::sin(roll), std::cos(roll), 0);
		float3 khat_roll = float3(0, 0, 1);

		float3 ihat_py = TransformVector(ihat_yaw, jhat_yaw, khat_yaw, ihat_pitch);
		float3 jhat_py = TransformVector(ihat_yaw, jhat_yaw, khat_yaw, jhat_pitch);
		float3 khat_py = TransformVector(ihat_yaw, jhat_yaw, khat_yaw, khat_pitch);

		float3 ihat = TransformVector(ihat_py, jhat_py, khat_py, ihat_roll);
		float3 jhat = TransformVector(ihat_py, jhat_py, khat_py, jhat_roll);
		float3 khat = TransformVector(ihat_py, jhat_py, khat_py, khat_roll);
		//float3 ihat = ihat_py;
		//float3 jhat = jhat_py;
		//float3 khat = khat_py;

		return {ihat, jhat, khat};
	}
	std::tuple<float3, float3, float3> GetInverseBasisVectors() {
		auto [ihat, jhat, khat] = GetBasisVectors();
		float3 ihat_inverse = float3(ihat.x, jhat.x, khat.x);
		float3 jhat_inverse = float3(ihat.y, jhat.y, khat.y);
		float3 khat_inverse = float3(ihat.z, jhat.z, khat.z);
		return {ihat_inverse, jhat_inverse, khat_inverse};
	}
	float3 ApplyRotationVectors(float3 p) {
		//auto [ihat, jhat, khat] = GetBasisVectors();
		float3 world = TransformVector(ihat, jhat, khat, p);
		if(parent != NULL) world = parent->ApplyRotationVectors(world);
		return world;
	}

	float3 ToWorldPoint(float3 p) {
		//auto [ihat, jhat, khat] = GetBasisVectors();
		if(parent != NULL) {
			scale.x = -scale.x;
			scale.y = -scale.y;
			scale.z = -scale.z;
		}
		float3 ihatT = ihat * scale.x;
		float3 jhatT = jhat * scale.y;
		float3 khatT = khat * scale.z;
		float3 world = TransformVector(ihatT, jhatT, khatT, p) + position;
		if(parent != NULL) world = parent->ToWorldPoint(world);
		if(parent != NULL) {
			scale.x = -scale.x;
			scale.y = -scale.y;
			scale.z = -scale.z;
		}
		return world;
	}
	float3 ToLocalPoint(float3 worldPoint) {
		//pitch += 1.5708;
		//auto [ihat, jhat, khat] = GetInverseBasisVectors();
		//pitch -= 1.5708;
		if(parent != NULL) worldPoint = parent->ToLocalPoint(worldPoint);
		float3 local = TransformVector(ihat_inv, jhat_inv, khat_inv, worldPoint - position);

		local.x /= scale.x;
		local.y /= scale.y;
		local.z /= scale.z;

		return local;
	}
	void UpdateRotation() {
		auto [ihatTemp, jhatTemp, khatTemp] = GetBasisVectors();
		auto [ihatInvTemp, jhatInvTemp, khatInvTemp] = GetInverseBasisVectors();

		ihat = ihatTemp;
		jhat = jhatTemp;
		khat = khatTemp;

		ihat_inv = ihatInvTemp;
		jhat_inv = jhatInvTemp;
		khat_inv = khatInvTemp;
		
	};


	/*float3 VertexToScreen(float3 vertex, float2 numPixels, float fov = 1) {
		float3 vertex_world = ToWorldPoint(vertex);
		int screenHeight_world = std::tan(fov/2) * 2;
		float pixelsPerWorldUnit = numPixels.y/screenHeight_world / vertex_world.z;

		float2 pixelOffset = float2(vertex_world.x * pixelsPerWorldUnit, vertex_world.y * pixelsPerWorldUnit);
		return float3(numPixels.x /2 + pixelOffset.x, numPixels.y/2 + pixelOffset.y, vertex_world.z);
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
	}*/
};


	std::tuple<std::vector<float3>, std::vector<float2>, std::vector<float3>> LoadObjFile(std::string obj) {
		std::vector<float3> allPoints;
		std::vector<float3> triPoints;
		std::vector<float2> allTexCoords;
		std::vector<float2> texCoords;
		std::vector<float3> allNormals;
		std::vector<float3> normals;

		std::stringstream lines(obj);
		std::string line;
		int a=0;

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
			} else if(line.substr(0,3) == "vt ") {

				std::vector<std::string> axes;
				std::stringstream words(line);
				std::string word;
				int i = 2;
				while(std::getline(words, word, ' ')) {
					if(i > 2) {
						axes.push_back(word);
					}
					i++;
				}
				float2 t = float2(std::stof(axes[0]), 1-std::stof(axes[1]));
				allTexCoords.push_back(t);
				
			}else if(line.substr(0,3) == "vn ") {

				std::vector<std::string> axes;
				std::stringstream words(line);
				std::string word;
				int i = 2;
				while(std::getline(words, word, ' ')) {
					if(i > 2) {
						axes.push_back(word);
					}
					i++;
				}
				float3 t = float3(std::stof(axes[0]), std::stof(axes[1]), std::stof(axes[2]));
				allNormals.push_back(t);
				
			}else

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
						if(word.empty()) indexGroup.push_back(0); else
						indexGroup.push_back(std::stoi(word));
					}
					int pointIndex = indexGroup[0]-1;
					int texIndex = indexGroup.size() > 1? indexGroup.at(1)-1 : -1;
					int normIndex = indexGroup.size() > 2? indexGroup.at(2)-1 : -1;
					if(i>=3) triPoints.push_back(triPoints[triPoints.size()-(3 * i - 6)]);
					if(i>=3) triPoints.push_back(triPoints[triPoints.size()-(2)]);

					if(i>=3) texCoords.push_back(texCoords[texCoords.size()-(3 * i - 6)]);
					if(i>=3) texCoords.push_back(texCoords[texCoords.size()-(2)]);

					if(i>=3) normals.push_back(normals[normals.size()-(3 * i - 6)]);
					if(i>=3) normals.push_back(normals[normals.size()-(2)]);

					triPoints.push_back(allPoints[pointIndex]);
					texCoords.push_back(texIndex >= 0? allTexCoords[texIndex] : float2(0,0));
					normals.push_back(normIndex >= 0? allNormals[normIndex] : float3(0,0,0));
				}

			}
		} 
		return {triPoints, texCoords, normals};
	}





class Camera : public Transform {
	public: 
	float fov;
	// hoping this blocks using camera having parent because it no work correctly for some reason
	const Transform* parent = NULL;
};

struct vertData {
	float3 norms[3];
	float2 coords[3];
};
struct lightData {
	enum types {
		directional = 0,
		point = 1,
	};
	float3 vector;
	float strength;
	bool type;
};

class Model : public Transform {
	public:
	Model() {
		UpdateRotation();

	}
	//float pitch = 0;
	//float yaw = 0;
	//float roll = 0;
	//float3 position;
	//std::vector<std::vector<float3>> faces;
	void (*sample)(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* model, std::vector<lightData> lightInfo);
	std::vector<float3> triPoints;
	std::vector<float2> texCoords;
	std::vector<float3> normals;
	Model(std::tuple<std::vector<float3>, std::vector<float2>, std::vector<float3>> data, void (*samp)(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* model, std::vector<lightData> lightInfo)) {
		auto [tri, tex, norm] = data;
		triPoints = tri;
		texCoords = tex;
		normals = norm;
		sample = samp;
		UpdateRotation();

	}
	void init(std::tuple<std::vector<float3>, std::vector<float2>, std::vector<float3>> data, void (*samp)(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* m, std::vector<lightData> lightInfo)) {
		auto [tri, tex, norm] = data;
		triPoints = tri;
		texCoords = tex;
		normals = norm;
		sample = samp;
		UpdateRotation();

	}
	/*float3 TransformVector(float3 ihat, float3 khat, float3 jhat, float3 v) {
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
	}*/
	float3 VertexToView(float3 vertex, Camera cam) {
		float3 vertex_world = ToWorldPoint(vertex);
		float3 vertex_view = cam.ToLocalPoint(vertex_world);
		return vertex_view;
	}
	float3 ViewToWorld(float3 vertex, Camera cam) {
		float3 vertex_world = cam.ToWorldPoint(vertex);
		return vertex_world;
	}

	float3 ViewToScreen(float3 vertex_view, float2 numPixels, Camera cam) {
		//float3 vertex_world = ToWorldPoint(vertex);
		//float3 vertex_view = cam.ToLocalPoint(vertex_world);
		//float3 vertex_view = cam.ToWorldPoint(vertex_world);


		float screenHeight_world = std::tan(cam.fov/2) * 2;
		float pixelsPerWorldUnit = numPixels.y/screenHeight_world / vertex_view.z;

		float2 pixelOffset = float2(vertex_view.x * pixelsPerWorldUnit, vertex_view.y * pixelsPerWorldUnit);
		return float3(numPixels.x /2 + pixelOffset.x, numPixels.y/2 + pixelOffset.y, vertex_view.z);
	}





};


float3 applyLighting(std::vector<lightData> lightInfo, float3 normal, Model* model) {
		static float3 lightPosition;

		static float3 dir;
		static float3 lightDir;

		static float level;
		static float strength;
		static float lightLevel;
		float3 l = {0,0,0};

		for(int i = 0; i < lightInfo.size(); i++) {
			lightPosition = lightInfo.at(i).vector;

			//if(lightInfo.at(i).type == lightData::point) {
			// its an enum of 1 or 0 so i can just check it directly
			if(lightInfo.at(i).type) {
				lightDir = Normalize((dir = lightPosition - model->position)); // close *enough* instead of doing slightly more work and basing it on vertex positions
																					  // actually, its a lot harder to convert it with vertex positions because NEAR PLANE CLIPPING DOIFJISODJFSJDFO
				dir.x = fabs(dir.x);
				dir.y = fabs(dir.y);
				dir.z = fabs(dir.z);
				level = 1/sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z + dir.z); // chat should i use the quake 3 algorithm
				strength = lightInfo.at(i).strength;
				level *= strength;
			//} else if(lightInfo.at(i).type == lightData::directional) {
			// theres only 2 values so like
			} else {

				lightDir = Normalize(lightPosition);
				strength = lightInfo.at(i).strength;
				level = strength;
			}

			lightLevel = (1+Dot3(Normalize(model->ApplyRotationVectors(normal)), lightDir)) * 0.5;
			l = l + (float3(level,level,level) * lightLevel);
				
		}

		return l;
}


namespace modelSamples {
	void smoothLightingAtPoint(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* model, std::vector<lightData> lightInfo) {
		// btw i use pointers instead of references because 1. sdl uses pointers since its for c instead of c++ and 2. i dont trusts them references their syntax make me unsure


		float depthX = 1/depths.x;
		float depthY = 1/depths.y;
		float depthZ = 1/depths.z;

		float2 texCoord = {0,0};
		texCoord = texCoord + (data.coords[0] * depthX) * weights.x;
		texCoord = texCoord + (data.coords[0+1] * depthY) * weights.y;
		texCoord = texCoord + (data.coords[0+2] *  depthZ) * weights.z;

		texCoord = texCoord * depth;

		float3 normal = {0,0,0};

		normal = normal + (data.norms[0] * depthX) * weights.x;
		normal = normal + (data.norms[0+1] * depthY) * weights.y;
		normal = normal + (data.norms[0+2] *  depthZ) * weights.z;
		normal = normal * depth;
		normal = Normalize(normal);


		/*float3 lightPosition;

		float3 dir;
		float3 lightDir;
																			  // actually, its a lot harder to convert it with vertex positions because NEAR PLANE CLIPPING DOIFJISODJFSJDFO
		float level;
		float strength;
		
		//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);
		//
		float lightLevel;
		float3 l = {0,0,0};

		for(int i = 0; i < lightInfo.size(); i++) {
		lightPosition = lightInfo.at(i).vector;

		if(lightInfo.at(i).type == lightData::point) {
			lightDir = Normalize((dir = lightPosition - model->position)); // close *enough* instead of doinng slightly more work and basing it on vertex positions
																				  // actually, its a lot harder to convert it with vertex positions because NEAR PLANE CLIPPING DOIFJISODJFSJDFO
			dir.x = fabs(dir.x);
			dir.y = fabs(dir.y);
			dir.z = fabs(dir.z);
			level = 1/sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z + dir.z); // chat should i use the quake 3 algorithm
			strength = lightInfo.at(i).strength;
			level *= strength;
		} else if(lightInfo.at(i).type == lightData::directional) {
			lightDir = Normalize(lightPosition);
			strength = lightInfo.at(i).strength;
			level = strength;
		}

		lightLevel = (1+Dot3(Normalize(model->ApplyRotationVectors(normal)), lightDir)) * 0.5;
		l = l + (float3(level,level,level) * lightLevel);
			
		}*/

		float3 l = applyLighting(lightInfo, normal, model);


		// directional lighting from the sun which is a mess so im commenting it out
		/*lightLevel = (1+Dot3(Normalize(model->ApplyRotationVectors(normal)), {0,0,1})) * 0.5;
		l = l + float3(1,1,1) * lightLevel;*/
		
		//if(sr) {

		SDL_ReadSurfacePixel(surface, floor(texCoord.x*surface->w), floor(texCoord.y*surface->h), r, g, b, NULL);
		//sampleSurface(texCoord.x, texCoord.y, surface, r, g, b);
		*r= fmin(255,*r*l.x);
		*g= fmin(255,*g*l.y);
		*b= fmin(255,*b*l.z);
		//getSurfacePixel(surface, round(texCoord.x*surface->w), round(texCoord.y*surface->h), &r, &g, &b);
		//sampleSurface(texCoord.x, texCoord.y, surface, &r,&g,&b);

		//SDL_Log("%zu, %d", m.texCoords.size(), i);

		//pixels[(y)*SCR_WIDTH+ x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
		//} else {
			//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);
			//normal = normal + float3(1,1,1);
			//normal = normal * 0.5;
			/*float3 normal = {0,0,0};
		normal = normal + m.normals[i] / depths.x * weights.x;
		normal = normal + m.normals[i+1] / depths.y * weights.y;
		normal = normal + m.normals[i+2] /  depths.z * weights.z;
		normal = normal * depth;
		normal = Normalize(normal);
		//normal = (normal + float3(1,1,1)) * 0.5;
		float lightLevel =  (Dot3(normal, {0,0,1})+1)*0.5;
		float3 l = float3(255,255,255) * lightLevel;*/
		//r = fmin(255,l.x*255);//normal.x*255;
		//g = fmin(255,l.y*255);//normal.y*255;
		//b = fmin(255,l.z*255);//normal.z*255;
	}
	void jaggedLightingAtPoint(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* model, std::vector<lightData> lightInfo) {
		// btw i use pointers instead of references because 1. sdl uses pointers since its for c instead of c++ and 2. i dont trust references they syntax make me unsure
		//float3 normal = {0,0,0};
		//normal = normal + norms[index] / depths.x * weights.x;
		//normal = normal + norms[index+1] / depths.y * weights.y;
		//normal = normal + norms[index+2] /  depths.z * weights.z;
		//normal = normal * depth;
		//normal = Normalize(normal);
		float3 normal = Normalize((data.norms[0] + data.norms[0+1] + data.norms[0+2])*0.33);

		/*float3 lightPosition = float3(0, 0, 1);

		float3 dir;
		float3 lightDir = Normalize((dir = lightPosition - model->position)); // close *enough* instead of doinng slightly more work and basing it on vertex positions
		float level = 1/sqrt(dir.x * dir.x + dir.y * dir.y + dir.z + dir.z);
		float strength = 1;
		level *= strength;
		//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);
		//
		float lightLevel = (1+Dot3(Normalize(model->ApplyRotationVectors(normal)), lightDir)) * 0.5;
		float3 l = float3(level,level,level) * lightLevel;*/

		/*lightLevel = (1+Dot3(Normalize(model->ApplyRotationVectors(normal)), {0,0,1})) * 0.5;
		l = l + float3(1,1,1) * lightLevel;*/
		float3 l = applyLighting(lightInfo, normal, model);
		
		//if(sr) {
		float2 texCoord = {0,0};
		texCoord = texCoord + data.coords[0] / depths.x * weights.x;
		texCoord = texCoord + data.coords[0+1] / depths.y * weights.y;
		texCoord = texCoord + data.coords[0+2] /  depths.z * weights.z;

		texCoord = texCoord * depth;

		SDL_ReadSurfacePixel(surface, floor(texCoord.x*surface->w), floor(texCoord.y*surface->h), r, g, b, NULL);
		//sampleSurface(texCoord.x, texCoord.y, surface, r,g, b);
		*r= fmin(255,*r*l.x);
		*g= fmin(255,*g*l.y);
		*b= fmin(255,*b*l.z);
		//getSurfacePixel(surface, round(texCoord.x*surface->w), round(texCoord.y*surface->h), &r, &g, &b);
		//sampleSurface(texCoord.x, texCoord.y, surface, &r,&g,&b);

		//SDL_Log("%zu, %d", m.texCoords.size(), i);

		//pixels[(y)*SCR_WIDTH+ x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
		//} else {
			//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);
			//normal = normal + float3(1,1,1);
			//normal = normal * 0.5;
			/*float3 normal = {0,0,0};
		normal = normal + m.normals[i] / depths.x * weights.x;
		normal = normal + m.normals[i+1] / depths.y * weights.y;
		normal = normal + m.normals[i+2] /  depths.z * weights.z;
		normal = normal * depth;
		normal = Normalize(normal);
		//normal = (normal + float3(1,1,1)) * 0.5;
		float lightLevel =  (Dot3(normal, {0,0,1})+1)*0.5;
		float3 l = float3(255,255,255) * lightLevel;*/
		//r = fmin(255,l.x*255);//normal.x*255;
		//g = fmin(255,l.y*255);//normal.y*255;
		//b = fmin(255,l.z*255);//normal.z*255;
	}

	void noLightingAtPoint(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* m, std::vector<lightData> lightInfo) {
		// btw i use pointers instead of references because 1. sdl uses pointers since its for c instead of c++ and 2. i dont trust references they syntax make me unsure
		//if(sr) {
		float2 texCoord = {0,0};
		texCoord = texCoord + data.coords[0] / depths.x * weights.x;
		texCoord = texCoord + data.coords[0+1] / depths.y * weights.y;
		texCoord = texCoord + data.coords[0+2] /  depths.z * weights.z;

		texCoord = texCoord * depth;

		SDL_ReadSurfacePixel(surface, floor(texCoord.x*surface->w), floor(texCoord.y*surface->h), r, g, b, NULL);
		//sampleSurface(texCoord.x, texCoord.y, surface, &r, &g, &b);
		//getSurfacePixel(surface, round(texCoord.x*surface->w), round(texCoord.y*surface->h), &r, &g, &b);
		//sampleSurface(texCoord.x, texCoord.y, surface, r,g,b);

		//SDL_Log("%zu, %d", m.texCoords.size(), i);

		//pixels[(y)*SCR_WIDTH+ x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
		//} else {
			//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);
			//normal = normal + float3(1,1,1);
			//normal = normal * 0.5;
			/*float3 normal = {0,0,0};
		normal = normal + m.normals[i] / depths.x * weights.x;
		normal = normal + m.normals[i+1] / depths.y * weights.y;
		normal = normal + m.normals[i+2] /  depths.z * weights.z;
		normal = normal * depth;
		normal = Normalize(normal);
		//normal = (normal + float3(1,1,1)) * 0.5;
		float lightLevel =  (Dot3(normal, {0,0,1})+1)*0.5;
		float3 l = float3(255,255,255) * lightLevel;*/
		//r = fmin(255,l.x*255);//normal.x*255;
		//g = fmin(255,l.y*255);//normal.y*255;
		//b = fmin(255,l.z*255);//normal.z*255;
	}

	
}



class Scene {

};

float Clamp(float p, float low, float high) {
	if( p<low) {
		return low;
	}
	if( p>high) {
		return high;
	}
	return p;
};


constexpr float toRadians(float x) {
	return (x/360) * 3.1415926*2;
};
inline float3 Lerp3(float3 a, float3 b, float t)
{
		t = Clamp(t, 0, 1);
		return a + (b - a) * t;
}
inline float2 Lerp2(float2 a, float2 b, float t){
	t = Clamp(t, 0, 1);
	return a + (b - a) * t;
}


void RenderModel(Model m, Camera cam, float2 screen, Uint32* pixels, float depthBuffer[], SDL_Surface * surface = NULL, std::vector<lightData> lightInfo = {{{0,0,1}, 1, lightData::directional}}) {
	SDL_LockSurface(surface);
	
		// stands for "surface real" trust
		//bool sr = true;
		if(surface == NULL) {
				surface = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_ABGR32);
				SDL_FillSurfaceRect(surface, NULL, 0xffffffff);
		}
		int modelSize = m.triPoints.size();
		#pragma omp parallel for
		float closeTriDepth;
		for(int i = 0; i<modelSize; i+=3) {
			float3 point1pre = m.VertexToView(m.triPoints.at(i),  cam);
			float3 point2pre = m.VertexToView(m.triPoints.at(i+1), cam);
			float3 point3pre = m.VertexToView(m.triPoints.at(i+2), cam);

			float3 point1world = m.ToWorldPoint(m.triPoints.at(i));
			float3 point2world = m.ToWorldPoint(m.triPoints.at(i+1));
			float3 point3world = m.ToWorldPoint(m.triPoints.at(i+2));

			//if(point1.z <= 0 || point2.z <= 0 || point3.z <= 0) continue;
			//
			float3 points[6];
			float2 coords[6];
			float3 norms[6];
			float3 vpoints[3];
			vpoints[0] = point1pre;
			vpoints[1] = point2pre;
			vpoints[2] = point3pre;

			const float nearClipDist = 0.30;
			bool clip0 = point1pre.z <= nearClipDist;
			bool clip1 = point2pre.z <= nearClipDist;
			bool clip2 = point3pre.z <= nearClipDist;
			int clipCount = clip0 + clip1 + clip2;
			float3 point1;
			float3 point2;
			float3 point3;

			int triCount = 0;
			switch (clipCount) {
				case 0:
				points[0] = point1pre;
				points[1] = point2pre;
				points[2] = point3pre;
				norms[0] = m.normals[i];
				norms[1] = m.normals[i+1];
				norms[2] = m.normals[i+2];
				coords[0] = m.texCoords[i];
				coords[1] = m.texCoords[i+1];
				coords[2] = m.texCoords[i+2];
				triCount = 1;
				break;

				case 1:{
				int indexClip = clip0? 0 : clip1? 1 : 2;
				int indexNext = (indexClip + 1) % 3;
				int indexPrev = (indexClip - 1 + 3) % 3;
				float3 pointClipped = vpoints[indexClip];
				float3 pointA = vpoints[indexNext];
				float3 pointB = vpoints[indexPrev];

				float fracA = (nearClipDist - pointClipped.z) / (pointA.z - pointClipped.z);
				float fracB = (nearClipDist - pointClipped.z) / (pointB.z - pointClipped.z);

				float3 clipAlongA = Lerp3(pointClipped, pointA, fracA);
				float3 clipAlongB = Lerp3(pointClipped, pointB, fracB);
				//AddRasterizerPoint(model, clipPointAlongEdgeB, i + indexClip, i + indexPrev, fracB);
				//AddRasterizerPoint(model, clipPointAlongEdgeA, i + indexClip, i + indexNext, fracA);
				//AddRasterizerPoint(model, pointB, i + indexPrev);

				//AddRasterizerPoint(model, clipPointAlongEdgeA, i + indexClip, i + indexNext, fracA);
				//AddRasterizerPoint(model, pointA, i + indexNext);
				//AddRasterizerPoint(model, pointB, i + indexPrev);

				points[0] = clipAlongB;
				coords[0] = Lerp2(m.texCoords[i+indexClip], m.texCoords[i+indexPrev], fracB);
				norms[0] = Lerp3(m.normals[i+indexClip], m.normals[i+indexPrev], fracB);

				points[1] = clipAlongA;
				coords[1] = Lerp2(m.texCoords[i+indexClip], m.texCoords[i+indexNext], fracA);
				norms[1] = Lerp3(m.normals[i+indexClip], m.normals[i+indexNext], fracA);

				points[2] = pointB;
				coords[2] = m.texCoords[i+indexPrev];
				norms[2] = m.normals[i+indexPrev];


				points[3] = clipAlongA;
				coords[3] = Lerp2(m.texCoords[i+indexClip], m.texCoords[i+indexNext], fracA);
				norms[3] = Lerp3(m.normals[i+indexClip], m.normals[i+indexNext], fracA);

				points[4] = pointA;
				coords[4] = m.texCoords[i+indexNext];
				norms[4] = m.normals[i+indexNext];

				points[5] = pointB;
				coords[5] = m.texCoords[i+indexPrev];
				norms[5] = m.normals[i+indexPrev];

				triCount = 2;
				break;}
				case 2:
				{
					// Figure out which point will not be clipped, and the two that will be
					int indexNonClip = !clip0 ? 0 : !clip1 ? 1 : 2;
					int indexNext = (indexNonClip + 1) % 3;
					int indexPrev = (indexNonClip - 1 + 3) % 3;

					float3 pointNotClipped = vpoints[indexNonClip];
					float3 pointA = vpoints[indexNext];
					float3 pointB = vpoints[indexPrev];

					// Fraction along triangle edge at which the depth is equal to the clip distance
					float fracA = (nearClipDist - pointNotClipped.z) / (pointA.z - pointNotClipped.z);
					float fracB = (nearClipDist - pointNotClipped.z) / (pointB.z - pointNotClipped.z);

					// New triangle points (in view space)
					float3 clipPointAlongEdgeA = Lerp3(pointNotClipped, pointA, fracA);
					float3 clipPointAlongEdgeB = Lerp3(pointNotClipped, pointB, fracB);

					// Create new triangle
					//AddRasterizerPoint(model, clipPointAlongEdgeB, i + indexNonClip, i + indexPrev, fracB);
					//AddRasterizerPoint(model, pointNotClipped, i + indexNonClip);
					//AddRasterizerPoint(model, clipPointAlongEdgeA, i + indexNonClip, i + indexNext, fracA);

					points[0] = clipPointAlongEdgeB;
					coords[0] = Lerp2(m.texCoords[i+indexNonClip], m.texCoords[i+indexPrev], fracB);
					norms[0] = Lerp3(m.normals[i+indexNonClip], m.normals[i+indexPrev], fracB);

					points[1] = pointNotClipped;
					coords[1] = m.texCoords[i+indexNonClip];
					norms[1] = m.normals[i+indexNonClip];

					points[2] = clipPointAlongEdgeA;
					coords[2] = Lerp2(m.texCoords[i+indexNonClip], m.texCoords[i+indexNext], fracA);
					norms[2] = Lerp3(m.normals[i+indexNonClip], m.normals[i+indexNext], fracA);

					triCount = 1;
					break;
				}

				//default:
				//triCount = 0;
				//	break;
			}
					

			//currentFace = tris.at(i);
			int index;
			for(int tri = 0; tri<triCount; tri++) {
			index = tri*3;
			point1 = m.ViewToScreen(points[index], screen, cam);
			point2 = m.ViewToScreen(points[index+1], screen, cam);
			point3 = m.ViewToScreen(points[index+2], screen, cam);
			float minX = fmin(fmin(point1.x,point2.x),point3.x);
			float minY = fmin(fmin(point1.y,point2.y),point3.y);
			float maxX = fmax(fmax(point1.x,point2.x),point3.x);
			float maxY = fmax(fmax(point1.y,point2.y),point3.y);

			maxX = fmin(maxX, SCR_WIDTH);
			minX = fmax(minX, 0);

			maxY = fmin(maxY, SCR_HEIGHT);
			minY = fmax(minY, 0);
			//float3 colSeed = m.triPoints.at(i) + m.triPoints.at(i+1) + m.triPoints.at(i+2) + m.position;
			//srand(colSeed.y + colSeed.z + colSeed.x);

						//SDL_ReadSurfacePixel(j,100, 100, &r, &g, &b, NULL);
						float3 triweight;
			PointInTriangle(point1, point2, point3, point1, triweight);
			float tridep = 1/Dot3(float3(1/point1.z, 1/point2.z, 1/point3.z), triweight);
			closeTriDepth = tridep;
			PointInTriangle(point1, point2, point3, point2, triweight);
			tridep = 1/Dot3(float3(1/point1.z, 1/point2.z, 1/point3.z), triweight);
			closeTriDepth = fmin(closeTriDepth, tridep);
			PointInTriangle(point1, point2, point3, point3, triweight);
			tridep = 1/Dot3(float3(1/point1.z, 1/point2.z, 1/point3.z), triweight);
			closeTriDepth = fmin(closeTriDepth, tridep);
			Uint8 r,g,b;
			//Uint8 rt,gt,bt;
		#pragma omp parallel for
			for(unsigned int y = minY; y<maxY; y++) {
		#pragma omp parallel for
				for(unsigned int x = minX; x<maxX; x++) {
					if(closeTriDepth > depthBuffer[(y)*SCR_WIDTH + x]) continue;
					float3 weights;
					if(PointInTriangle(point1, point2, point3, float2(x,y), weights)) {
						float3 depths = float3(point1.z, point2.z, point3.z);
						float depth = 1/Dot3(float3(1/depths.x, 1/depths.y, 1/depths.z), weights);
						if(depth > depthBuffer[(y)*SCR_WIDTH + x]) continue;

						
						// replace this with function passing in rgb pointers
						/*float3 normal = {0,0,0};
						normal = normal + norms[index] / depths.x * weights.x;
						normal = normal + norms[index+1] / depths.y * weights.y;
						normal = normal + norms[index+2] /  depths.z * weights.z;
						normal = normal * depth;
						normal = Normalize(normal);
						//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);

						float lightLevel = (1+Dot3(normal, {0,1,0}))*0.5;
						float3 l = float3(1.5,1.5,1.5) * lightLevel;
						
						//if(sr) {
						float2 texCoord = {0,0};
						texCoord = texCoord + coords[index] / depths.x * weights.x;
						texCoord = texCoord + coords[index+1] / depths.y * weights.y;
						texCoord = texCoord + coords[index+2] /  depths.z * weights.z;

						texCoord = texCoord * depth;

						SDL_ReadSurfacePixel(surface, round(texCoord.x*surface->w), round(texCoord.y*surface->h), &r, &g, &b, NULL);
						//sampleSurface(texCoord.x, texCoord.y, surface, &r, &g, &b);
						r= fmin(255,r*l.x);
						g= fmin(255,g*l.y);
						b= fmin(255,b*l.z);*/
//void (*samp)(float3 weights, float3 depths, float depth, std::vector<float3> norms, std::vector<float2> coords, int index, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b)) {
						vertData data;
						data.norms[0] = norms[0 + index];
						data.norms[1] = norms[1 + index];
						data.norms[2] = norms[2 + index];

						data.coords[0] = coords[0 + index];
						data.coords[1] = coords[1 + index];
						data.coords[2] = coords[2 + index];

						m.sample(weights, depths, depth, data, surface, &r, &g, &b, &m, lightInfo);
						//getSurfacePixel(surface, round(texCoord.x*surface->w), round(texCoord.y*surface->h), &r, &g, &b);
						//sampleSurface(texCoord.x, texCoord.y, surface, &r,&g,&b);

						//SDL_Log("%zu, %d", m.texCoords.size(), i);

						//pixels[(y)*SCR_WIDTH+ x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
						//} else {
							//float3 normal = Normalize((m.normals[i] + m.normals[i+1] + m.normals[i+2])/3);
							//normal = normal + float3(1,1,1);
							//normal = normal * 0.5;
							/*float3 normal = {0,0,0};
						normal = normal + m.normals[i] / depths.x * weights.x;
						normal = normal + m.normals[i+1] / depths.y * weights.y;
						normal = normal + m.normals[i+2] /  depths.z * weights.z;
						normal = normal * depth;
						normal = Normalize(normal);
						//normal = (normal + float3(1,1,1)) * 0.5;
						float lightLevel =  (Dot3(normal, {0,0,1})+1)*0.5;
						float3 l = float3(255,255,255) * lightLevel;*/
						//r = fmin(255,l.x*255);//normal.x*255;
						//g = fmin(255,l.y*255);//normal.y*255;
						//b = fmin(255,l.z*255);//normal.z*255;
						//}
						pixels[(y)*SCR_WIDTH + x] = RGBToBin(r,g,b);
						depthBuffer[(y)*SCR_WIDTH + x] = depth;
					}
				}
			}
			}
		}

	SDL_UnlockSurface(surface);
}

void initialiseModel(Model * model, const char* modelFilename, void (*sampleType)(float3 weights, float3 depths, float depth, vertData data, SDL_Surface * surface, Uint8 * r, Uint8 * g, Uint8 * b, Model* model, std::vector<lightData> lightInfo)) {
	std::string objstr = "";
	std::ifstream objfile(modelFilename);
	for(std::string line; std::getline(objfile, line); objstr+=line+"\n");
	model->init(LoadObjFile(objstr), sampleType);
	objfile.close();
}

SDL_Texture * createScreen(SDL_Renderer* renderer) {
       SDL_Texture * tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);
       SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
       return tex;
};

float* createDepthBuffer() {
       static float buffer[SCR_WIDTH * SCR_HEIGHT];
       return buffer;
}

void clearBuffers(Uint32* pixels, float* depthBuffer) {
	#pragma omp parallel for
	for(unsigned int y = 0; y<SCR_HEIGHT; y++) {
		#pragma omp parallel for
		for(unsigned int x = 0; x<SCR_WIDTH; x++) {
			pixels[y*SCR_WIDTH + x] = RGBToBin(0, 0, 0);
			depthBuffer[y*SCR_WIDTH + x] = std::numeric_limits<double>::infinity();
		}
	}
}


int main(int argc, char**argv) {
	constexpr int frameRate = 30;
	constexpr float targetFrameTime = 1000.f/frameRate;

	int realWinWidth;
	int realWinHeight;

	std::vector<lightData> lightInfo; 
	lightInfo.push_back({{0,0,1}, 2, lightData::point});
	lightInfo.push_back({{2,0,0}, 2, lightData::point});
	lightInfo.push_back({{0,0,1}, 1, lightData::directional});

	srand(time(0));
	if(!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("%s", SDL_GetError());
	}
	//SDL_Window* window = SDL_CreateWindow("rasterizer", SCR_WIDTH,SCR_HEIGHT, 0);
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
	SDL_Texture* screenTex = createScreen(renderer);
	SDL_Surface * uvtex;
	Uint32 *pixels;
	void* pix;
	int pitch;
	Model m;
	Camera cam;
	cam.fov = toRadians(90);
	//m.scale = {5,5,5};
	Model c;

	//c.position.z = 2;
	//m.position.y = 10;
	//m.position.z = 1;
	//c.pitch = toRadians(90);
	m.roll = 3.141;

	SDL_Rect mouseRect = {SCR_WIDTH/2, SCR_HEIGHT/2, 3, 3};
	c.parent = &m;


	//std::vector<Model> models;
	//m.position.z = 10;
	//m.position.y = -10;
	/*std::string objstr = "";
	std::ifstream objfile("stars.obj");
	for(std::string line; std::getline(objfile, line); objstr+=line+"\n");
	c.init(LoadObjFile(objstr), modelSamples::noLightingAtPoint);
	objfile.close();*/

	/*objfile.open("skysub.obj");
	objstr = "";
	for(std::string line; std::getline(objfile, line); objstr+=line+"\n");
	m.init(LoadObjFile(objstr), modelSamples::smoothLightingAtPoint);

	objfile.close();*/

	initialiseModel(&m, "resources/earth.obj", modelSamples::smoothLightingAtPoint);
	initialiseModel(&c, "stars.obj", modelSamples::noLightingAtPoint);

	#if SDLIMG == 1
	uvtex = IMG_Load(argv[2]);
	#else
	uvtex = SDL_LoadBMP(argv[2]);
	#endif


	int frameCount = 0;
	int s = m.triPoints.size();
	int s2 = c.triPoints.size();
	float2 screen = float2(SCR_WIDTH, SCR_HEIGHT);
	float* depthBuffer = createDepthBuffer();
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
	float2 mouseDelta = {0,0};
	float2 usedMouseDelta = {0,0};
	float mouseSensitivity = 1;
	float camSpeed = 0.3;
	bool keyDown[10] = {false,false,false,false,false,false,false,false,false,false,};
	SDL_SetWindowRelativeMouseMode(window, true);
	SDL_Surface* screenshot;

	int start_time = 0;
	int endtime = 0;
	while(running) {
		start_time = SDL_GetTicks();
		SDL_GetWindowSize(window, &realWinWidth, &realWinHeight);
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

						case SDL_SCANCODE_PRINTSCREEN:
							screenshot = SDL_RenderReadPixels(renderer, NULL);

							#if SDLIMG == 1
							IMG_SavePNG(screenshot, "screenshot.png");
							#else
							SDL_SaveBMP(screenshot, "screenshot.bmp");
							#endif

							SDL_DestroySurface(screenshot);

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

				case SDL_EVENT_MOUSE_MOTION:
					float2 mouseDelta(event.motion.xrel / SCR_WIDTH * mouseSensitivity, event.motion.yrel / SCR_WIDTH * mouseSensitivity);
					cam.pitch += mouseDelta.y;
					cam.yaw +=  mouseDelta.x;
					cam.pitch = Clamp(cam.pitch, -1.5708, 1.5708);
					cam.UpdateRotation();
					break;


			}

		}
		m.roll+=0.01;
		m.UpdateRotation();
		//m.position.z += sin(frameCount*0.1)*0.01;
		/*if(keyDown[D]) {
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
			m.position.x-=0.02;
		}
		if(keyDown[RIGHT]) {
			m.position.x+=0.02;
		}
		if(keyDown[COMMA]) {
			m.position.z+=0.02;
		}
		if(keyDown[PERIOD]) {
			m.position.z-=0.02;
		}*/
		//float up;
		//https://stackoverflow.com/questions/10569659/camera-pitch-yaw-to-direction-vector

		if(keyDown[A]) {
			cam.position.x += cos(cam.yaw) * camSpeed;
			cam.position.y += sin(cam.yaw) * camSpeed;
		}
		if(keyDown[D]) {
			cam.position.x -= cos(cam.yaw) * camSpeed;
			cam.position.y -= sin(cam.yaw) * camSpeed;
		}
		float xzlen = cos(cam.pitch);
		float y = xzlen * cos(cam.yaw);
		float z = -sin(cam.pitch);
		float x = xzlen * sin(-cam.yaw);
		//up = sin(cam.pitch);
		//float ps = 1 - fabs(up);
		//ps = cbrt(ps);
		//SDL_Log("ps, %f", ps);
		if(keyDown[W]) {
			//cam.position.x -= sin(cam.yaw) * ps;
			//cam.position.y += cos(cam.yaw) * ps;

			//cam.position.z -= up;
			
			cam.position.x += x * camSpeed;
			cam.position.y += y * camSpeed;
			cam.position.z += z * camSpeed;
		}
		if(keyDown[S]) {
			//cam.position.x += sin(cam.yaw) * ps;
			//cam.position.y -= cos(cam.yaw) * ps;

			//cam.position.z += up;
			cam.position.x -= x * camSpeed;
			cam.position.y -= y * camSpeed;
			cam.position.z -= z * camSpeed;


		}
		if(keyDown[COMMA]) {
			cam.fov+=toRadians(1);
		}
		if(keyDown[PERIOD]) {
			cam.fov-=toRadians(1);
		}

		if(keyDown[LEFT]) {
			m.position.x-=0.1;
		}
		if(keyDown[RIGHT]) {
			m.position.x+=0.1;
		}
		if(keyDown[UP]) {
			m.position.y-=0.1;
		}
		if(keyDown[DOWN]) {
			m.position.y+=0.1;
		}

			
		

		SDL_RenderClear(renderer);
		//tris.clear();

		SDL_LockTexture(screenTex, NULL, &pix, &pitch);
		pixels = (Uint32*)pix;
		
		clearBuffers(pixels, depthBuffer);
		

		//#pragma omp parallel for
		/*for(int i = 0; i<s; i+=3) {
			float3 point1 = m.VertexToScreen(m.triPoints.at(i), screen, cam);
			float3 point2 = m.VertexToScreen(m.triPoints.at(i+1), screen, cam);
			float3 point3 = m.VertexToScreen(m.triPoints.at(i+2), screen, cam);
			if(point1.z <= 0 || point2.z <= 0 || point3.z <= 0) continue;

			//currentFace = tris.at(i);
			float minX = fmin(fmin(point1.x,point2.x),point3.x);
			float minY = fmin(fmin(point1.y,point2.y),point3.y);
			float maxX = fmax(fmax(point1.x,point2.x),point3.x);
			float maxY = fmax(fmax(point1.y,point2.y),point3.y);

			maxX = fmin(maxX, SCR_WIDTH);
			minX = fmax(minX, 0);

			maxY = fmin(maxY, SCR_HEIGHT);
			minY = fmax(minY, 0);
			//srand(m.triPoints.at(i).x + m.triPoints.at(i+1).y + m.triPoints.at(i+2).z);
			//int r = rand()%255,g = rand()%255,b=rand()%255;
			for(unsigned int y = minY; y<maxY; y++) {
				for(unsigned int x = minX; x<maxX; x++) {
					float3 weights;
					if(PointInTriangle(point1, point2, point3, float2(x,y), weights)) {
						float3 depths = float3(point1.z, point2.z, point3.z);
						float depth = 1/Dot3(float3(1/depths.x, 1/depths.y, 1/depths.z), weights);
						if(depth > depthBuffer[(y)*SCR_WIDTH + x]) continue;
						pixels[(y)*SCR_WIDTH + x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
						//pixels[(y)*SCR_WIDTH + x] = RGBToBin(r,g,b);
						depthBuffer[(y)*SCR_WIDTH + x] = depth;
					}
				}
			}
		}*/
		//m.UpdateRotation();
		RenderModel(m, cam, screen, pixels, depthBuffer, uvtex, lightInfo);
		Model s = m;
		s.position = {0,0,1};
		s.scale = {0.2, 0.2 ,0.2};
		RenderModel(s, cam, screen, pixels, depthBuffer, NULL, lightInfo);
		s.position = {2,0,0};
		s.scale = {0.2, 0.2 ,0.2};
		RenderModel(s, cam, screen, pixels, depthBuffer, NULL, lightInfo);
		RenderModel(c, cam, screen, pixels, depthBuffer, NULL, lightInfo);
		//RenderModel(c, s2, cam, screen, pixels, depthBuffer, col);
		/*#pragma omp parallel for
		for(int i = 0; i<s2; i+=3) {
			float3 point1 = c.VertexToScreen(c.triPoints.at(i), screen, cam);
			float3 point2 = c.VertexToScreen(c.triPoints.at(i+1), screen, cam);
			float3 point3 = c.VertexToScreen(c.triPoints.at(i+2), screen, cam);
			if(point1.z <= 0 || point2.z <= 0 || point3.z <= 0) continue;

			//currentFace = tris.at(i);
			float minX = fmin(fmin(point1.x,point2.x),point3.x);
			float minY = fmin(fmin(point1.y,point2.y),point3.y);
			float maxX = fmax(fmax(point1.x,point2.x),point3.x);
			float maxY = fmax(fmax(point1.y,point2.y),point3.y);

			maxX = fmin(maxX, SCR_WIDTH);
			minX = fmax(minX, 0);

			maxY = fmin(maxY, SCR_HEIGHT);
			minY = fmax(minY, 0);
			for(unsigned int y = minY; y<maxY; y++) {
				for(unsigned int x = minX; x<maxX; x++) {
					float3 weights;
					if(PointInTriangle(point1, point2, point3, float2(x,y), weights)) {
						float3 depths = float3(point1.z, point2.z, point3.z);
						float depth = 1/Dot3(float3(1/depths.x, 1/depths.y, 1/depths.z), weights);
						if(depth > depthBuffer[(y)*SCR_WIDTH + x]) continue;
						pixels[(y)*SCR_WIDTH + x] = RGBToBin(col[i/3].x, col[i/3].y, col[i/3].z);
						depthBuffer[(y)*SCR_WIDTH + x] = depth;
					}
				}
			}
		}*/


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

		SDL_RenderTextureRotated(renderer, screenTex, NULL,NULL, 180, NULL, SDL_FLIP_NONE);
		SDL_SetRenderDrawColor(renderer, 255,0,0,255);
		

		SDL_RenderPresent(renderer);
		endtime = SDL_GetTicks();
		int tickspassed = endtime - start_time; 
		int amountToWait = targetFrameTime - tickspassed;
		//SDL_Log("%d", frameCount);
		//SDL_Log("fps: %f", 1000.f / tickspassed);
		if(amountToWait > 0) {
			SDL_Delay(amountToWait);
		}
		endtime = SDL_GetTicks();
		tickspassed = endtime - start_time; 
		SDL_Log("%d", frameCount);
		SDL_Log("fps: %f", 1000.f / tickspassed);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
