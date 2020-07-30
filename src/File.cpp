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
  const char* path;
};

struct ModelInfo {
  uint32 vSize;
  uint32 stride;
  uint32 iSize;
  float32* vertices = nullptr;
  uint16* indices = nullptr;
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
  bmp.path = path;
  return bmp;
}

ModelInfo load_obj(const char* path) {
  char fPath[512];
  full_path(fPath, path);
  FILE* file = fopen(fPath, "r");
  assert_(file, "Trying to load a file that doesn't exist!");

  char* pLine;
  char line[256]; 
  uint32 vOffset = 0;
  uint32 vnOffset = 0;
  uint32 vtOffset = 0;
  uint32 indexCount = 0;
  uint32 length = 0;
  
  float32 vertices[192];
  uint16 indices[50000];
  const uint16 stride = 8;

  while((pLine = fgets(line, sizeof(line), file))) {
    if(line[0] == 'v' && line[1] == ' ') {
      char* num;
      uint32 field = 0;
      while((num = strtok(pLine, " "))) {
	pLine = nullptr; 
	if(!strcmp(num, "v")) continue;
	vertices[vOffset + field] = atof(num);
	field++;
      }
      vOffset += stride;
    }
    if(line[0] == 'v' && line[1] == 'n') {
      char* num;
      uint32 field = 3;
      while((num = strtok(pLine, " "))) {
	pLine = nullptr; 
	if(!strcmp(num, "vn")) continue;
	vertices[vnOffset + field] = atof(num);
	field++;
      }
      vnOffset += stride;
    }
    if(line[0] == 'v' && line[1] == 't') {
      char* num;
      uint32 field = 6;
      while((num = strtok(pLine, " "))) {
	pLine = nullptr; 
	if(!strcmp(num, "vt")) continue;
	vertices[vtOffset + field] = atof(num);
	field++;
      }
      vtOffset += stride;
    }
    if(line[0] == 'f' && line[1] == ' ') {
      char* num;
      char* buf[3];
      uint8 index = 0;
      while((num = strtok(pLine, " "))) {
	pLine = nullptr; 
	if(!strcmp(num, "f")) continue;
	buf[index] = num;
	index++;
      }
      index = 0;
      while(index < 3) {
	strchr(buf[index], '/');
	//if(ptr) *ptr = '\0';
	indices[indexCount] = (uint16)atoi(buf[index]);
	indexCount++;
	index++;
      }
    }
    length++;
  }
  fclose(file);

  for(int32 i = 0; i < indexCount; i += 6) {
    log_("%d, %d, %d,    %d, %d, %d\n", 
	 indices[i + 0],
	 indices[i + 1],
	 indices[i + 2],
	 indices[i + 3],
	 indices[i + 4],
	 indices[i + 5]
	 );
  }

  ModelInfo info = {};
  info.vertices = vertices;
  info.stride = stride * sizeof(float32);
  info.vSize = vOffset * sizeof(float32);
  info.indices = indices;
  info.iSize = indexCount * sizeof(uint16);
  return info;
}
