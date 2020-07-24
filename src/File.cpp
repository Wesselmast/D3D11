#pragma once
#include <stdlib.h>
#include <string.h>

struct FileInfo {
  char* memory;
  uint32 size;
};

struct Bitmap {
  uint16 width;
  uint16 height;
  char* memory;
};

struct ModelInfo {
  float32* vertices = nullptr;
  uint32 vSize;
  uint32 stride;
  uint16* indices = nullptr;
  uint32 iSize;
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
  return bmp;
}

ModelInfo load_obj(const char* path) {
  char fPath[512];
  full_path(fPath, path);
  FILE* file = fopen(fPath, "r");
  assert_(file, "Trying to load a file that doesn't exist!");

  char* pLine;
  char line[256]; 
  uint32 offset = 0;
  uint32 indexCount = 0;
  uint32 length = 0;
  
  float32 vertices[160000] = {};
  uint16 indices[160000] = {};

  while((pLine = fgets(line, sizeof(line), file))) {
    if(line[0] == 'v' && line[1] == ' ') {
      char* num;
      uint32 field = 0;
      while((num = strtok(pLine, " "))) {
	pLine = nullptr; 
	if(!strcmp(num, "v")) continue;
	vertices[offset + field] = atof(num);
	field++;
      }
      offset += 5;
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
	char* ptr = strchr(buf[index], '/');
	if(ptr) *ptr = '\0';
	indices[indexCount] = atoi(buf[index]);
	indexCount++;
	index++;
      }
    }
    length++;
  }
  fclose(file);

  ModelInfo info = {};
  info.vertices = &vertices[0];
  info.vSize = offset * sizeof(float32);
  info.stride = 5 * sizeof(float32);
  info.indices = &indices[0];
  info.iSize = indexCount * sizeof(uint16);
  return info;
}
