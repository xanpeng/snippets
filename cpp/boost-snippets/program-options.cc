//
// g++ program-options.cc -o program-options -lboost_program_options
//

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  try {
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "show this help message")
      ("compression,c", po::value<int>()->required(), "set compression level");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }
    if (vm.count("compression")) {
      std::cout << "compression level was " << vm["compression"].as<int>() << std::endl;
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
