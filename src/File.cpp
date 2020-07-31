#pragma once
#include <stdlib.h>
#include <string.h>
#include <fstream>

const uint32 PATH_SIZE_LIMIT = 512;

struct FileInfo {
  char* memory;
  uint32 size;
};

struct Bitmap {
  uint16 width;
  uint16 height;
  char* memory;
  char path[PATH_SIZE_LIMIT];
};

struct ModelInfo {
  uint32 vSize;
  uint32 stride;
  uint32 iSize;
  float32* vertices = nullptr;
  uint16* indices = nullptr;
  char path[PATH_SIZE_LIMIT];
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

FileInfo load_file(const char* path) {
  char fPath[512];
  full_path(fPath, path);
  FILE* file = fopen(fPath, "rb");
  assert_(file, "Trying to load a file that doesn't exist!");

  uint32 size;
  char* memory;
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);
  memory = (char*)malloc(size + 1);
  if(size != fread(memory, 1, size, file)) {
    free(memory);
    fclose(file);
    assert_(false, "Filesize is not correct. Something went wrong when reading the file");
    return { nullptr, 0 };
  }
  fclose(file);
  memory[size] = 0;

  FileInfo fileInfo = {}; 
  fileInfo.memory = memory;
  fileInfo.size = size;
  return fileInfo;
}

Bitmap load_bitmap(const char* path) {
  FileInfo info = load_file(path);
  assert_(info.size, "Trying to load a bitmap that's of size 0");

  BitmapHeader* header = (BitmapHeader*)info.memory;

  Bitmap bmp = {};
  bmp.width = (uint16)header->width;
  bmp.height = (uint16)header->height;
  bmp.memory = info.memory + header->offset;
  strcpy(bmp.path, path);
  return bmp;
}

ModelInfo load_obj(const char* path) {
  char fPath[512];
  full_path(fPath, path);
  FILE* file = fopen(fPath, "r");
  assert_(file, "Trying to load a file that doesn't exist!");

  uint32 vOffset = 0;
  uint32 vnOffset = 3;
  uint32 vtOffset = 6;
  uint32 indexOffset = 0;
  
  float32 vertices[16000];
  uint16 indices[16000];
  const uint16 stride = 8;

  while(1) {
    char line[256];
    int result = fscanf(file, "%s", line);
    if(result == EOF) {
      break;
    }
    
    if(!strcmp(line, "v")) {
      fscanf(file, "%f %f %f\n", 
	     &(vertices[vOffset + 0]),
	     &(vertices[vOffset + 1]),
	     &(vertices[vOffset + 2]));
      vOffset += stride;
    }
    else if(!strcmp(line, "vn")) {
      fscanf(file, "%f %f %f\n", 
	     &(vertices[vnOffset + 0]),
	     &(vertices[vnOffset + 1]),
	     &(vertices[vnOffset + 2]));
      vnOffset += stride;
    }
    else if(!strcmp(line, "vt")) {
      fscanf(file, "%f %f\n", 
	     &(vertices[vtOffset + 0]),
	     &(vertices[vtOffset + 1]));
      vtOffset += stride;
    }
    else if(!strcmp(line, "f")) {
      int32 temp_indices[3];
      fscanf(file, "%hu %hu %hu\n", 
	     &(indices[indexOffset + 0]),
	     &(indices[indexOffset + 1]),
	     &(indices[indexOffset + 2]));
      indices[indexOffset + 0]--;
      indices[indexOffset + 1]--;
      indices[indexOffset + 2]--;
      indexOffset += 3;
    }
  }

  fclose(file);

  ModelInfo info = {};
  info.vertices = vertices;
  info.stride = stride * sizeof(float32);
  info.vSize = vOffset * sizeof(float32);
  info.indices = indices;
  info.iSize = indexOffset * sizeof(uint16);
  strcpy(info.path, path);
  return info;
}
