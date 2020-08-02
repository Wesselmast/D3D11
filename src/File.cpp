#pragma once
#include <stdlib.h>
#include <string.h>
#include <fstream>

const uint32 MAX_VERTEX_COUNT = 12500;
const uint32 MAX_INDEX_COUNT = 100000;
const uint32 OBJECT_NAME_LIMIT = 256;

struct FileInfo {
  uint8* memory;
  uint32 size;
};

struct Bitmap {
  uint16 width;
  uint16 height;
  uint8* memory = nullptr;
};

struct ModelInfo {
  uint32 vSize;
  uint32 stride;
  uint32 iSize;
  float32* vertices = nullptr;
  uint16* indices = nullptr;
  char name[OBJECT_NAME_LIMIT];
}; 

#pragma pack(push, 1)
struct BitmapHeader {
  uint16 fileType;
  uint32 fileSize;
  uint16 reserved1;
  uint16 reserver2;
  uint32 offset;
  uint32 size;
  int32  width;
  int32  height;
  uint16 planes;
  uint16 bitsPerPixel;
};
#pragma pack(pop)

void load_file(GameMemory* memory, const char* path, FileInfo& info) {
  char fPath[512];
  full_path(fPath, path);
  FILE* file = fopen(fPath, "rb");
  assert_(file, "Trying to load a file that doesn't exist!");

  uint32 size;
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  uint8* m = allocate(memory, size + 1);
  if(size != fread(m, 1, size, file)) {
    fclose(file);
    assert_(false, "Filesize is not correct. Something went wrong when reading the file");
    return;
  }
  fclose(file);

  m[size] = 0;
  info.memory = m;
  info.size = size;
}

void load_bitmap(GameMemory* memory, const char* path, Bitmap& bmp) {
  FileInfo info;
  load_file(memory, path, info);
  assert_(info.size, "Trying to load a bitmap that's of size 0");

  BitmapHeader* header = (BitmapHeader*)info.memory;

  bmp.width = (uint16)header->width;
  bmp.height = (uint16)header->height;
  bmp.memory = info.memory + header->offset;
}

void load_obj(GameMemory* memory, const char* path, ModelInfo& info) {
  char fPath[512];
  full_path(fPath, path);
  FILE* file = fopen(fPath, "r");
  assert_(file, "Trying to load a file that doesn't exist!");

  const uint16 stride = 8;

  uint32 vpOffset = 0;
  uint32 vnOffset = 0;
  uint32 vtOffset = 0;
  uint32 indexOffset = 0;
  uint32 vertexOffset = 0;
  
  Vec3 vP[MAX_VERTEX_COUNT];
  Vec3 vN[MAX_VERTEX_COUNT];
  Vec2 vT[MAX_VERTEX_COUNT];

  float32* vertices = (float32*)allocate(memory, MAX_VERTEX_COUNT * stride * sizeof(float32));
  uint16* indices = (uint16*)allocate(memory, MAX_INDEX_COUNT * sizeof(uint16));
  char name[OBJECT_NAME_LIMIT];

  while(1) {
    char line[256];
    int result = fscanf(file, "%s", line);
    if(result == EOF) {
      break;
    }
    else if(!strcmp(line, "o")) {
      fscanf(file, "%s\n", name);
    } 
    else if(!strcmp(line, "v")) {
      fscanf(file, "%f %f %f\n", 
	     &(vP[vpOffset].x),
	     &(vP[vpOffset].y),
	     &(vP[vpOffset].z));
      vpOffset++;
    }
    else if(!strcmp(line, "vn")) {
      fscanf(file, "%f %f %f\n", 
	     &(vN[vnOffset].x),
	     &(vN[vnOffset].y),
	     &(vN[vnOffset].z));
      vnOffset++;
    }
    else if(!strcmp(line, "vt")) {
      fscanf(file, "%f %f\n", 
	     &(vT[vtOffset].x),
	     &(vT[vtOffset].y));
      vtOffset++;
    }
    else if(!strcmp(line, "f")) {
      uint16 faces[9];
      int32 matches = fscanf(file, "%hu/%hu/%hu %hu/%hu/%hu %hu/%hu/%hu\n", 
			     &faces[0], &faces[1], &faces[2],
			     &faces[3], &faces[4], &faces[5],
			     &faces[6], &faces[7], &faces[8]);
      if(matches == 9) {
	for(int32 i = 0; i < 9; i += 3) {
	  Vec3 v1p = vP[faces[i + 0] - 1];
	  Vec2 v1t = vT[faces[i + 1] - 1];
	  Vec3 v1n = vN[faces[i + 2] - 1];
	  
	  bool found = false;
	  for(int32 j = 0; j < vertexOffset + stride; j += stride) {
	    if(are_near(v1p.x, vertices[j + 0]) &&
	       are_near(v1p.y, vertices[j + 1]) &&
	       are_near(v1p.z, vertices[j + 2]) &&
	       are_near(v1n.x, vertices[j + 3]) &&
	       are_near(v1n.y, vertices[j + 4]) &&
	       are_near(v1n.z, vertices[j + 5]) &&
	       are_near(v1t.x, vertices[j + 6]) &&
	       are_near(v1t.y, vertices[j + 7])) {
	      indices[indexOffset] = j / stride;
	      found = true;
	      break;
	    }
	  }
	  if(!found) {
	    vertices[vertexOffset + 0] = v1p.x;
	    vertices[vertexOffset + 1] = v1p.y;
	    vertices[vertexOffset + 2] = v1p.z;
	    
	    vertices[vertexOffset + 3] = v1n.x;
	    vertices[vertexOffset + 4] = v1n.y;
	    vertices[vertexOffset + 5] = v1n.z;
	    
	    vertices[vertexOffset + 6] = v1t.x;
	    vertices[vertexOffset + 7] = v1t.y;
	    
	    vertexOffset += stride;
	    
	    indices[indexOffset] = (vertexOffset / stride) - 1;  
	  }
	  indexOffset++;
	}
      }
      else {
	assert_(false, "invalid input! cannot read file\n");
      }
    }
  }
  fclose(file);

  info.vertices = vertices;
  info.stride = stride * sizeof(float32);
  info.vSize = vertexOffset * sizeof(float32);
  info.indices = indices;
  info.iSize = indexOffset * sizeof(uint16);
  strcpy(info.name, name);
}
