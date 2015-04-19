//
// http://leveldb.googlecode.com/svn/trunk/doc/index.html
// https://github.com/dazfuller/LevelDB-Sample/blob/master/sample.cpp
//
#include <iostream>
#include <sstream>
#include <string>

#include <leveldb/db.h>

int main() {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;

  leveldb::Status status = leveldb::DB::Open(options, "./testdb", &db);
  if (false == status.ok()) {
    std::cerr << "Unable to open/create test database './testdb'" << std::endl;
    std::cerr << status.ToString() << std::endl;
    return -1;
  }

  leveldb::WriteOptions write_options;
  for (unsigned int i = 0; i < 256; ++i) {
    std::ostringstream key_stream;
    key_stream << "Key" << i;
    
    std::ostringstream value_stream;
    value_stream << "Test data value: " << i;

    db->Put(write_options, key_stream.str(), value_stream.str());
  }

  leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::cout << it->key().ToString() << " : " << it->value().ToString() << std::endl;
  }

  if (false == it->status().ok()) {
    std::cerr << "An error was found during the scan" << std::endl;
    std::cerr << it->status().ToString() << std::endl;
  }

  delete it;
  delete db;
  
  return 0;
}
