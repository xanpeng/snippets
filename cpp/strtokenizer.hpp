//
// A naive char-string tokenizer and split tool (by tokenizer)
//
i#ifndef STR_TOKENIZER_H_
#define STR_TOKENIZER_H_

#include "util.h"
#include <algorithm>
#include <string>
#include <iterator>
#include <iostream>
#include <vector>

// TODO group delimiter, with multiple chars

namespace libcppstr {

class Tokenizer : private NonCopyable {
public:
  typedef const char* CharPtr;
  typedef std::pair<CharPtr, CharPtr> RangeType;

private:
  class TokenizerIterator : public std::iterator<std::forward_iterator_tag, RangeType> {
  public:
    explicit inline TokenizerIterator(const CharPtr begin, const CharPtr end, const std::string& delimiter)
      : end_(end),
      range_(begin, begin),
      current_token_(end, end),
      delimiter_(delimiter),
      first_token_done_(false),
      last_token_done_(false),
      group_delimiter_(false),
      compress_delimiter_(true) {
        if (end != begin) {
          set_delimiter_table();
        }
      }

    inline bool operator!=(const TokenizerIterator& iter) const {
      return (range_ != iter.range_) || (end_ != iter.end_);
    }

    inline RangeType operator*() const {
      return current_token_;
    }

    inline TokenizerIterator operator++() {
      // first ++, then print, so we should give the last 
      // token a chance to print
      if (last_token_done_) {
        range_.first = range_.second;
        return (*this);
      }

      // iterate the string, increase the cursor
      while (!ReachEnd()) {
        if (!delimiter_table_[*(range_.second)]) {
          ++range_.second;
          if (ReachEnd()) {
            current_token_ = range_;
          }
          continue;
        }

        if (!compress_delimiter_) {
          current_token_ = range_;
          ++range_.second;
          first_token_done_ = true;
          if (!ReachEnd()) {
            range_.first = range_.second;
            return (*this);
          }
        }

        if (compress_delimiter_) {
          current_token_ = range_;
          ++range_.second;
          while (!ReachEnd() && delimiter_table_[*(range_.second)]) {
            ++range_.second;
          }

          if (!ReachEnd()) {
            range_.first = range_.second;
            if (!first_token_done_) {
              first_token_done_ = true;
              continue;
            }
            return (*this);
          }
        }
      } // end-of-while

      // reach the end
      last_token_done_ = true;
      if (!first_token_done_) {
        range_.first = range_.second;
      }
      return (*this);
    }

    inline void set_group_delimiter(bool flag) {
      group_delimiter_ = flag;
    }
    inline void set_compress_delimiter(bool flag) {
      compress_delimiter_ = flag;
    }

  private:
    inline bool ReachEnd() {
      return range_.second == end_;
    }

    inline void set_delimiter_table() {
      std::fill_n(delimiter_table_, kTableSize, false);
      for (unsigned int i = 0; i < delimiter_.length(); ++i) {
        delimiter_table_[delimiter_[i]] = true;
      }
    }

    CharPtr end_;
    RangeType range_;
    RangeType current_token_;
    bool first_token_done_;
    bool last_token_done_;

    // use the whole string as a delimiter or not
    // only allow one group delimiter
    // default: NO
    bool group_delimiter_;
    // Don't print empty, ex.ignore empty in string "||"
    // default: YES
    bool compress_delimiter_;
    const std::string& delimiter_;

    // printable ASCII table
    // Normally we only use some of these ASCII characters as delimiter
    static const std::size_t kTableSize = 128;
    bool delimiter_table_[kTableSize];
  }; // class TokenizerIterator

public:
  typedef TokenizerIterator iterator;
  Tokenizer(const std::string& s, const std::string& delimiter)
    : begin_(s.data()),
    end_(s.data() + s.size()),
    begin_iter_(begin_, end_, delimiter),
    end_iter_(end_, end_, delimiter) {}

  inline void set_compress_delimiter(bool flag) {
    begin_iter_.set_compress_delimiter(flag);
  }

  inline const iterator begin() {
    ++begin_iter_;
    return begin_iter_;
  }

  inline const iterator end() const {
    return end_iter_;
  }

private:
  CharPtr begin_;
  CharPtr end_;
  iterator begin_iter_;
  iterator end_iter_;
}; // class Tokenizer

//
// split
//

class TokenizerOutputIterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
public:
  TokenizerOutputIterator(std::vector<std::string>& container) : container_(container) {}

  TokenizerOutputIterator& operator=(Tokenizer::RangeType range) {
    container_.push_back(std::string(range.first, range.second));
  }

  TokenizerOutputIterator& operator*() {
    return *this;
  }

  TokenizerOutputIterator& operator++() {
    return *this;
  }

  TokenizerOutputIterator& operator++(int) {
    return *this;
  }

private:
  std::vector<std::string>& container_;
};

static inline void split(const std::string& line,
    const std::string& delimiter,
    std::vector<std::string>& vec) {
  Tokenizer tokenizer(line, delimiter);
  std::copy(tokenizer.begin(),
      tokenizer.end(),
      TokenizerOutputIterator(vec));
}

} // namespace libcppstr

namespace {

static inline std::ostream& operator<<(std::ostream& os, libcppstr::Tokenizer::iterator range) {
  os << std::string((*range).first, (*range).second);
  return os;
}

} // namespace

void example_split() {
  std::cout << "example_split" << std::endl;
  std::string sarray[] = { 
    "|.;{",
    "abc.|123{|xyz|;789",
    "..abc|123|xyz|.789",
    "|.abc|123|xyz|;789{",
    ".;abc|123|xyz{|789{;",
  };

  std::vector<std::string> vec;
  for (std::string s : sarray) {
    std::cout << "To split: " << s << std::endl;

    vec.clear();
    libcppstr::split(s, "|.;{", vec);
    for (std::string token : vec) {
      std::cout << "\t[" << token << "]";
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}

//
// Worries:
//
// Not compliant with STL standard.
// Not elegant.
// May have defects (like maybe construct redundant objects implicitly).
//

#endif
