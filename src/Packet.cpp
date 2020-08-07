#pragma once

const uint32 MAX_PACKET_SIZE = kilobytes(8);

enum PacketType {
  INVALID,
  PLAYER_TRANSFORM,
  PLAYER_AMOUNT,
};

struct Packet {
  uint32 length = 0;
  uint32 offset = 0;
  uint8 data[MAX_PACKET_SIZE];
};

void packet_append(Packet& packet, const uint8* data, uint32 size) {
  assert_(packet.length + size <= MAX_PACKET_SIZE, "Tried to send a package that exceeds the max packet size!")
  for(uint32 i = 0; i < size; i++) {
    packet.data[packet.length + i] = (uint8)data[i];
  }
  packet.length += size;
}

void empty_packet(Packet& packet) {
  packet.length = 0;
  packet.offset = 0;
  memset(&packet.data, 0x00, MAX_PACKET_SIZE);
}

void packet_insert(Packet& packet, uint32 data) {
  data = htonl(data);
  packet_append(packet, (const uint8*)&data, sizeof(uint32));
}

void packet_insert(Packet& packet, PacketType data) {
  packet_insert(packet, (uint32)data);
}

void packet_insert(Packet& packet, float32 data) {
  packet_append(packet, (const uint8*)&data, sizeof(float32));
}

void packet_insert(Packet& packet, Vec3 data) {
  packet_append(packet, (const uint8*)&data, sizeof(Vec3));
}

void packet_insert(Packet& packet, const char* data) {
  uint32 length = (uint32)strlen(data);
  packet_insert(packet, length);
  packet_append(packet, (const uint8*)data, length);
}

void packet_extract(Packet& packet, uint32& data) {
  assert_(packet.offset + sizeof(data) > packet.length, "Tried to extract more then the buffer allows!");

  data = *(uint32*)&packet.data[packet.offset];
  data = ntohl(data);
  packet.offset += sizeof(uint32);
}

void packet_extract(Packet& packet, PacketType& data) {
  uint32 type;
  packet_extract(packet, type);
  data = (PacketType)type;
}

void packet_extract(Packet& packet, float32& data) {
  assert_(packet.offset + sizeof(data) > packet.length, "Tried to extract more then the buffer allows!");

  data = *(float32*)&packet.data[packet.offset];
  packet.offset += sizeof(float32);
}

void packet_extract(Packet& packet, Vec3& data) {
  assert_(packet.offset + sizeof(data) > packet.length, "Tried to extract more then the buffer allows!");
  data = *(Vec3*)&packet.data[packet.offset];
  packet.offset += sizeof(Vec3);
}

void packet_extract(Packet& packet, char* data) {
  uint32 length;
  packet_extract(packet, length);

  assert_(packet.offset + length > packet.length, "Tried to extract more then the buffer allows!");

  strcpy(data, (const char*)&packet.data[packet.offset]);
  packet.offset += length;
}
