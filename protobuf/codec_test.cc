#include "codec.h"
#include "reqresp.pb.h"
#include <stdio.h>

void print(const std::string& buf)
{
  printf("encoded to %zd bytes\n", buf.size());
  for (size_t i = 0; i < buf.size(); ++i)
  {
    printf("%2zd:  0x%02x  %c\n",
           i,
           (unsigned char)buf[i],
           isgraph(buf[i]) ? buf[i] : ' ');
  }
}

void testReq()
{
  reqresp::CreateReq req;
  req.set_magic(77);
  req.set_op(88);
  req.set_opsn(99);
  req.set_volume_id(1);
  req.set_chunk_size(128);

  std::string transport = encode(req);
  print(transport);

  int32_t be32 = 0;
  std::copy(transport.begin(), transport.begin() + sizeof be32, reinterpret_cast<char*>(&be32));
  int32_t len = ::ntohl(be32);
  assert(len == transport.size() - sizeof(be32));

  // network library decodes length header and get the body of message
  std::string buf = transport.substr(sizeof(int32_t));
  assert(len == buf.size());

  reqresp::CreateReq* dreq = dynamic_cast<reqresp::CreateReq*>(decode(buf));
  assert(dreq != NULL);
  dreq->PrintDebugString();
  assert(dreq->DebugString() == req.DebugString());
  delete dreq;
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testReq();
  puts("");
  puts("All pass!!!");

  google::protobuf::ShutdownProtobufLibrary();
}

