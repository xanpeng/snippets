//
// g++ --std=c++11 compact.cc -o compact -lboost_filesystem -lboost_system -lrocksdb -lpthread
//
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace rocksdb;


void CompactFiles(DB *db, string db_path)
{
  vector<string> input_file_names;
  boost::filesystem::directory_iterator end_it;
  for (boost::filesystem::directory_iterator it(db_path); it != end_it; ++it) {
    if (!boost::filesystem::is_regular_file(it->status()))
      continue;

    if (it->path().extension() != ".sst")
      continue;
    input_file_names.push_back(it->path().filename().string());
  }

  // for (string fn : input_file_names)
  //  cout << fn << endl;

  int output_level;
  CompactionOptions copts;
  Status s = db->CompactFiles(copts, input_file_names, output_level);
}

void CompactRange(DB* db, ColumnFamilyHandle* cf)
{
  CompactRangeOptions cro;
  cout << __func__ << " begin...\n";
  db->CompactRange(cro, cf, nullptr, nullptr);
  cout << __func__ << " end...\n";
  db->DropColumnFamily(cf);
  delete cf;
}

int main()
{
  string db_path = "/path/to/db";
  DB* db;

  Options opts;
  opts.create_if_missing = true;
  opts.create_missing_column_families = true;

  vector<ColumnFamilyDescriptor> cfd_vec;
  cfd_vec.push_back(ColumnFamilyDescriptor(kDefaultColumnFamilyName, opts));
  cfd_vec.push_back(ColumnFamilyDescriptor("dbLog", opts));

  vector<ColumnFamilyHandle*> cfs;
  Status s = rocksdb::DB::Open(opts, db_path, cfd_vec, &cfs, &db);
  if (!s.ok())
    cout << s.ToString() << endl;


  CompactRange(db, cfs[1]);

  return 0;
}
