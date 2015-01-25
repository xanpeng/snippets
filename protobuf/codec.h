#ifndef PROTOBUF_CODEC_H
#define PROTOBUF_CODEC_H

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <zlib.h>  // adler32
#include <string>
#include <arpa/inet.h>  // htonl, ntohl
#include <stdint.h>
#include <stdio.h>

const int kHeaderLen = sizeof(int32_t);
///
/// Encode protobuf Message to transport format defined above
/// returns a std::string.
///
/// returns a empty string if message.AppendToString() fails.
///
inline std::string encode(const google::protobuf::Message& message)
{
  std::string result;

  result.resize(kHeaderLen);

  const std::string& typeName = message.GetTypeName();
  int32_t nameLen = static_cast<int32_t>(typeName.size()+1);
  int32_t be32 = ::htonl(nameLen);
  result.append(reinterpret_cast<char*>(&be32), sizeof be32);
  printf("encode namelen: (%zd, %zd)\n", sizeof be32, result.size());

  result.append(typeName.c_str(), nameLen);
  printf("encode name: (%zd, %zd)\n", typeName.size() + 1, result.size());

  std::string msg_only;
  message.AppendToString(&msg_only);
  bool succeed = message.AppendToString(&result);
  printf("encode body: (%zd, %zd)\n", msg_only.size(), result.size());

  if (!succeed)
  {
    result.clear();
  }
  int32_t len = ::htonl(result.size() - kHeaderLen);
  std::copy(reinterpret_cast<char*>(&len),
      reinterpret_cast<char*>(&len) + sizeof len,
      result.begin());

  return result;
}

inline google::protobuf::Message* createMessage(const std::string& type_name)
{
  google::protobuf::Message* message = NULL;
  const google::protobuf::Descriptor* descriptor =
    google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
  if (descriptor)
  {
    const google::protobuf::Message* prototype =
      google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
    if (prototype)
    {
      message = prototype->New();
    }
  }
  return message;
}

inline int32_t asInt32(const char* buf)
{
  int32_t be32 = 0;
  ::memcpy(&be32, buf, sizeof(be32));
  return ::ntohl(be32);
}

///
/// Decode protobuf Message from transport format defined above.
/// returns a Message*
///
/// returns NULL if fails.
///
inline google::protobuf::Message* decode(const std::string& buf)
{
  google::protobuf::Message* result = NULL;

  int32_t len = static_cast<int32_t>(buf.size());
  int32_t nameLen = asInt32(buf.c_str());
  if (nameLen < 2 || nameLen > len - kHeaderLen)
  {
    fprintf(stderr, "ERROR: invalid type name\n");
    exit(-1);
  }
  std::string typeName(buf.begin() + kHeaderLen, buf.begin() + kHeaderLen + nameLen - 1);

  google::protobuf::Message* message = createMessage(typeName);
  if (!message)
  {
    fprintf(stderr, "ERROR: construct google::protobuf::Message failed\n");
    exit(-1);
  }

  const char* data = buf.c_str() + kHeaderLen + nameLen;
  int32_t dataLen = len - nameLen - kHeaderLen;
  if (message->ParseFromArray(data, dataLen))
  {
    result = message;
  }
  else
  {
    fprintf(stderr, "ERROR: parse message failed\n");
    delete message;
    exit(-1);
  }

  return result;
}

#endif  // PROTOBUF_CODEC_H
