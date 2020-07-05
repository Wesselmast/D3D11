#pragma once

struct FileInfo {
  char* memory;
  uint32 size;
};

struct Bitmap {
  uint16 width;
  uint16 height;
  char* memory;
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
  assert(file, "Trying to load a file that doesn't exist!");

  uint32 size;
  char* memory;
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);
  memory = (char*)malloc(size + 1);
  if(size != fread(memory, 1, size, file)) {
    free(memory);
    fclose(file);
    assert(false, "Filesize is not correct. Something went wrong when reading the file");
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
  assert(info.size, "Trying to load a bitmap that's of size 0");

  BitmapHeader* header = (BitmapHeader*)info.memory;

  Bitmap bmp = {};
  bmp.width = (uint16)header->width;
  bmp.height = (uint16)header->height;
  bmp.memory = info.memory + header->offset;
  return bmp;
}
