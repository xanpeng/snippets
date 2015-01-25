import google.protobuf
import google.protobuf.message_factory
import google.protobuf.descriptor
import google.protobuf.descriptor_pool
import google.protobuf.text_format
import socket
import struct
import curses.ascii
import reqresp_pb2


def encode(msg):
    buf_str = ''

    full_type_name = msg.DESCRIPTOR.full_name
    name_len = socket.htonl(len(full_type_name) + 1)
    buf_str += struct.pack('<I', name_len)
    print 'encode name_len: (%d, %d)' % (4, len(buf_str)+4)

    buf_str += full_type_name
    buf_str += struct.pack('<B', 0)
    print 'encode name: (%d, %d)' % (len(full_type_name)+1, len(buf_str)+4)

    buf_str += msg.SerializeToString()
    print 'encode body: (%d, %d)' % (len(msg.SerializeToString()), len(buf_str)+4)

    data_len = socket.htonl(len(buf_str))
    buf_str = ''.join([struct.pack('<I', data_len), buf_str])

    return buf_str


def generate_pool():
    pool = google.protobuf.descriptor_pool.DescriptorPool()
    obj_fd = google.protobuf.descriptor_pb2.FileDescriptorProto.FromString(reqresp_pb2.DESCRIPTOR.serialized_pb)
    pool.Add(obj_fd)
    return pool


def create_message(full_type_name):
    pool = generate_pool()
    message_descriptor = pool.FindMessageTypeByName(full_type_name)
    if not message_descriptor:
        print('ERROR create descriptor')
        return None
    prototype = google.protobuf.message_factory.MessageFactory().GetPrototype(message_descriptor)
    if not prototype:
        print('ERROR create prototype')
        return None
    message = prototype()
    return message


def decode(buf):
    (name_len,) = struct.unpack_from('>I', buf[:4])
    if name_len < 2 or name_len > len(buf) - 4:
        print('ERROR: invalid type name')
        return None

    full_type_name = buf[4:4+name_len-1]
    message_instance = create_message(full_type_name)
    if not message_instance:
        print('ERROR create protobuf message')
        return None
    message_instance.ParseFromString(buf[4+name_len:])
    return message_instance


def deep_print(buf):
    print('encoded to %d bytes' % len(buf))
    counter = 0
    for c in buf:
        printable = ' '
        if curses.ascii.isgraph(c):
            printable = c
        print('%2d:  0x%02x  %c' % (counter, ord(c), printable))
        counter += 1


def test_req():
    req = reqresp_pb2.CreateReq()
    req.magic = 77
    req.op = 88
    req.opsn = 99
    req.volume_id = 1
    req.chunk_size = 128

    transport = encode(req)
    deep_print(transport)

    # decode
    # data_len = socket.ntohl(int(transport[:4]))
    (data_len,) = struct.unpack_from('>I', transport[:4])
    print data_len, len(transport)
    assert data_len == len(transport) - 4

    dreq = decode(transport[4:])
    assert dreq
    print google.protobuf.text_format.MessageToString(dreq)


def main():
    test_req()


if __name__ == '__main__':
    main()