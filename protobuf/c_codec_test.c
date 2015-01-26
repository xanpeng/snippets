#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "reqresp.pb-c.h"

void deep_print(char *buf, int buf_len)
{
  printf("encoded to %d bytes\n", buf_len);
  for (int i = 0; i < buf_len; ++i)
  {
    printf("%2d:  0x%02x  %c\n",
        i,
        (unsigned char)buf[i],
        isgraph(buf[i]) ? buf[i] : ' ');
  }
}

void *encode(const ProtobufCMessage *msg, int* buf_len)
{
  const char *type_name = msg->descriptor->name;
  int32_t name_len = strlen(type_name) + 1;
  int32_t int_size = sizeof(name_len);
  int32_t body_len = protobuf_c_message_get_packed_size(msg);
  int32_t total_len = int_size + int_size + name_len + body_len;

  // alloc buf
  void *buf = malloc(total_len);
  if (buf == NULL)
  {
    fprintf(stderr, "ERROR: malloc failed\n");
    exit(-1);
  }

  // insert header len
  int32_t record_msg_len = total_len - int_size;
  int32_t be_record_msg_len = htonl(record_msg_len);
  memcpy(buf, (const char*)(&be_record_msg_len), int_size);

  // insert name_len
  int32_t be_name_len = htonl(name_len);
  memcpy(buf + int_size, (const char*)(&be_name_len), int_size);
  int msg_len = int_size;
  msg_len += int_size;
  printf("encode namelen: (%d, %d)\n", int_size, msg_len);

  // insert type_name
  memcpy(buf + int_size * 2, type_name, name_len);
  msg_len += name_len;
  printf("encode name: (%d, %d)\n", name_len, msg_len);

  // insert serialized msg
  // reqresp__create_req__pack(msg, buf + int_size * 2 + name_len);
  protobuf_c_message_pack(msg, buf+int_size*2+name_len);
  msg_len += body_len;
  printf("encode body: (%d, %d)\n", body_len, msg_len);

  // return encoded data
  *buf_len = total_len;
  return buf;
}

ProtobufCMessage* decode(void *buf, int32_t buf_len, 
    const ProtobufCMessageDescriptor* descriptor)
{
  ProtobufCMessage *result = NULL;
  int32_t be32 = 0;
  memcpy(&be32, buf, sizeof(int32_t));
  int32_t name_len = ntohl(be32);
  if (name_len < 2 || name_len > buf_len - sizeof(int32_t))
  {
    fprintf(stderr, "ERROR: invalid type name\n");
    exit(-1);
  }

  // get type_name
  char *type_name = (char*)malloc(name_len);
  memcpy(type_name, buf + sizeof(int32_t), name_len-1);

  result = protobuf_c_message_unpack(descriptor, NULL,
      buf_len - name_len - sizeof(int32_t), 
      buf + name_len + sizeof(int32_t));
  return result;
}

void testReq()
{
  Reqresp__CreateReq msg = REQRESP__CREATE_REQ__INIT;
  msg.magic = 77;
  msg.op = 88;
  msg.opsn = 99;
  msg.volume_id = 1;
  msg.chunk_size = 128;

  // int32_t body_len = reqresp__create_req__get_packed_size(msg);
  int total_len;
  void *encoded = encode((const ProtobufCMessage*)&msg, &total_len);

  deep_print(encoded, total_len);

  int32_t be32 = 0;
  memcpy(&be32, encoded, sizeof(be32));
  int32_t len = ntohl(be32);
  assert(len == total_len - sizeof(be32));

  Reqresp__CreateReq *dmsg = (Reqresp__CreateReq*) decode(encoded + sizeof(be32),
      total_len - sizeof(be32),
      &reqresp__create_req__descriptor);
  assert(dmsg != NULL);
  printf("decoded op: %d\n", dmsg->op);
}

// protobuf-c don't support find message by type_name,
// so we manually record "typename<-> descriptor" here?
void register_message(const ProtobufCMessage *msg)
{
  // TODO
}

int main()
{
  testReq();
  puts("");
  puts("All pass!");
  return 0;
}
