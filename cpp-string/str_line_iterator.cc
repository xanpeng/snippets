//
// Sample code to practice iterator. iterate a std::string, tokenize and print it with delimiter '\n'
//
#include <iostream>
#include <string>
#include <iterator>

namespace {

class LinedString {
public:
  typedef const std::string::value_type* ConstCharPtr;

private:
  class LinedStringIterator {
  protected:
    typedef std::pair<ConstCharPtr, ConstCharPtr> range_type;

  public:
    explicit inline LinedStringIterator(const ConstCharPtr begin, const ConstCharPtr end)
      : end_(end),
      range_(begin, begin),
      current_token_(end, end),
      last_token_done_(false) {
        if (end != begin) {
          this->operator++();
        }
      }

    inline LinedStringIterator& operator++() {
      if (last_token_done_) {
        range_.first = range_.second;
        return (*this);
      }
      else if (end_ != range_.second) {
        range_.first = range_.second;
      }

      while (end_ != range_.second) {
        if ('\n' == *(range_.second)) {
          current_token_ = range_;
          ++range_.second;
          return (*this);
        }
        else {
          ++range_.second;
        }
      }

      if (range_.first != range_.second) {
        current_token_.second = range_.second;
        if (!last_token_done_) {
          if (*(range_.second - 1) == '\n')
            current_token_.first = range_.second;
          else
            current_token_.first = range_.first;
          last_token_done_ = true;
        }
        else {
          range_.first = range_.second;
        }
      }

      return (*this);
    }

    inline range_type operator*() const {
      return current_token_;
    }

    inline bool operator!=(const LinedStringIterator& iter) {
      return (range_ != iter.range_) || (end_ != iter.end_);
    }

  private:
    ConstCharPtr end_;
    range_type range_;
    range_type current_token_;
    bool last_token_done_;
  }; // class LinedStringIterator


public:
  typedef LinedStringIterator iterator;

  inline LinedString(const std::string& s)
    : begin_(s.data()),
    end_(s.data() + s.size()),
    begin_iter_(begin_, end_),
    end_iter_(end_, end_) {}

  inline const LinedStringIterator& begin() const {
    return begin_iter_;
  }

  inline const LinedStringIterator& end() const {
    return end_iter_;
  }

private:
  ConstCharPtr begin_;
  ConstCharPtr end_;
  LinedStringIterator begin_iter_;
  LinedStringIterator end_iter_;
}; // class LinedString

static inline std::ostream& operator<<(std::ostream& os,
    const LinedString::iterator& range) {
  os << std::string((*range).first, (*range).second);
  return os;
}

} // namespace

int main() {
  std::string many_lines = "hello\nworld\nxan\nxanpeng\nyasha\nxyasha\nzyasha\nyyasha\n\nhello";
  LinedString lined_str(many_lines);
  LinedString::iterator iter = lined_str.begin();
  while (iter != lined_str.end()) {
    std::cout << iter << "\n";
    ++iter;
  }

  return 0;
}
