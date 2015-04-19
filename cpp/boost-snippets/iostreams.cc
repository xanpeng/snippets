//
// g++ main.cc -o main [-lboost_iostreams] 
//
// about this sample file:
// 1) two ways to define your source
// 2) two ways to define your sink
// 3) use container as source / sink
// 4) two ways to define device
// 5) use container as device
// 6) output filter
//

#include <boost/iostreams/categories.hpp> // source_tag, sink_tag, seekable_device_tag
#include <boost/iostreams/concepts.hpp>   // source, sink, seekable_device
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/iostreams/stream.hpp>     // stream
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/range/iterator_range.hpp>

#include <ios>        // ios_base::beg
#include <iosfwd>     // streamsize, seekdir
#include <algorithm>  // copy, min
#include <iterator>   // back_inserter
#include <string>
#include <cassert>

namespace io = boost::iostreams;

class MySource1 {
public:
  typedef char char_type;
  typedef io::source_tag category;  // indicate only support read()
  
  std::streamsize read(char* s, std::streamsize n) {
    // TODO
    (void) s; (void) n; return -1;
  }
};

class MySource2 : public io::source {
public:
  std::streamsize read(char* s, std::streamsize n);
};

template<typename Container>
class ContainerSource {
public:
  typedef typename Container::value_type char_type;
  typedef io::source_tag category;
  ContainerSource(Container& container) : container_(container), pos_(0) {}
  
  std::streamsize read(char_type* s, std::streamsize n) {
    std::streamsize amt = static_cast<std::streamsize>(container_.size()- pos_);
    std::streamsize result = std::min(n, amt);
    if (result != 0) {
      std::copy(container_.begin() + pos_, container_.begin() + pos_ + result, s);
      pos_ += result;
      return result;
    }
    else {
      return -1;  // EOF
    }
  }

  Container& container() { return container_; }

private:
  typedef typename Container::size_type size_type;
  Container& container_;
  size_type pos_;
};

class MySink1 {
public:
  typedef char char_type;
  typedef io::sink_tag category;
  std::streamsize write(const char* s, std::streamsize n) {
    // TODO
    (void)s; (void)n; return -1;
  }
};

class MySink2 {
public:
  std::streamsize write(const char* s, std::streamsize n);
};

template<typename Container>
class ContainerSink {
public:
  typedef typename Container::value_type char_type;
  typedef io::sink_tag category;
  ContainerSink(Container& container) : container_(container) {}

  std::streamsize write(const char_type* s, std::streamsize n) {
    container_.insert(container_.end(), s, s + n);
    return n;
  }
  Container& container() { return container_; }
private:
  Container& container_;
};

class MyDevice1 {
public:
  typedef char char_type;
  typedef io::seekable_device_tag category;

  std::streamsize read(char* s, std::streamsize n) {
    // TODO
    (void)s; (void)n; return -1;
  }
  std::streamsize write(const char* s, std::streamsize n) {
    // TODO
    (void)s; (void)n; return -1;
  }
  io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way) {
    // TODO
    (void) off; (void) way; return -1;
  }
};

/* no io::seekable_device?
class MyDevice2 : public io::seekable_device {
public:
  std::streamsize read(char* s, std::streamsize n);
  std::streamsize write(const char* s, std::streamsize n);
  io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);
};
*/

template<typename Container>
class ContainerDevice {
public:
  typedef typename Container::value_type char_type;
  typedef io::seekable_device_tag category;
  ContainerDevice(Container& container) : container_(container), pos_(0) {}

  std::streamsize read(char_type* s, std::streamsize n) {
    std::streamsize amt = static_cast<std::streamsize>(container_.size() - pos_);
    std::streamsize result = std::min(n, amt);
    if (result != 0) {
      std::copy(container_.begin() + pos_, container_.begin() + pos_ + result, s);
      pos_ += result;
      return result;
    }
    else {
      return -1;
    }
  }

  std::streamsize write(const char_type* s, std::streamsize n) {
    std::streamsize result = 0;
    if (pos_ != container_.size()) {
      std::streamsize amt = static_cast<std::streamsize>(container_.size() - pos_);
      result = std::min(n, amt);
      std::copy(s, s+result, container_.begin()+pos_);
      pos_ += result;
    }
    if (result < n) {
      container_.insert(container_.end(), s+result, s+n);
      pos_ = container_.size();
    }
    return n;
  }

  io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way) {
    io::stream_offset next;
    if (way == std::ios_base::beg) {
      next = off;
    } else if (way == std::ios_base::cur) {
      next = pos_ + off;
    } else if (way == std::ios_base::end) {
      next = container_.size() + off - 1;
    } else {
      throw std::ios_base::failure("bad seek direction");
    }

    // Check for errors
    if (static_cast<int>(next) < 0 || static_cast<size_type>(next) >= container_.size())
      throw std::ios_base::failure("bad seek offset");

    pos_ = next;
    return pos_;
  }

  Container& container() { return container_; }

private:
  typedef typename Container::size_type size_type;
  Container& container_;
  size_type pos_;
};

int main() {
  using namespace std;

  // source
  typedef ContainerSource<string> StringSource;

  string input = "Hello world";
  string output;
  io::stream<StringSource> in(input);
  getline(in, output);
  assert(input == output);

  io::filtering_istream fin(boost::make_iterator_range(input));
  getline(fin, output);
  assert(input == output);

  // sink
  typedef ContainerSink<string> StringSink;
  string result1;
  io::stream<StringSink> out1(result1);
  out1 << "hello world";
  out1.flush();
  assert(result1 == "hello world");

  string result2;
  io::filtering_ostream out2(back_inserter(result2));
  out2 << "hello world";
  out2.flush();
  assert(result2 == "hello world");

  // device
  typedef ContainerDevice<string> StringDevice;
  string one, two;

  io::stream<StringDevice> io(one);
  io << "hello world";
  io.flush();
  io.seekg(0, BOOST_IOS::beg);  // seek to the beginning
  getline(io, two);
  assert(one == "hello world");
  assert(two == "hello world");

  // output filter
}
