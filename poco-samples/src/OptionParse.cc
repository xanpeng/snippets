// g++ OptionParse.cc -o OptionParse -lPocoUtil
// Run:
//  ./OptionParse -I /usr -L /lib -o /tmp -v,   
//  ./OptionParse --inc /usr --libr /lib -o /tmp -v

#include <iostream>
#include <vector>
#include <iterator>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionProcessor.h>
#include <Poco/Util/OptionException.h>


using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionProcessor; 


class OptionUtil {
public:
    void DefineOptions() {
        options_.addOption(
                Option("include-dir", "I", "specify a search path for locating header files")
                .required(false).repeatable(true).argument("path"));
        options_.addOption(
                Option("library-dir", "L", "specify a search path for locating library files")
                .required(false).repeatable(true).argument("path"));
        options_.addOption(
                Option("output", "o", "specify the output file", true)
                .argument("file", true));
        options_.addOption(
                Option("verbose", "v", "enable verbose mode")
                .required(false).repeatable(false));

        options_.addOption(
                Option("optimize", "O", "enable optimization")
                .required(false).repeatable(false).argument("level", false).group("mode"));

        options_.addOption(
                Option("debug", "g", "generate debug information")
                .required(false).repeatable(false).group("mode"));

        options_.addOption(
                Option("info", "i", "print information")
                .required(false).repeatable(false));
    }

    void AcceptArguments(int argc, char* argv[]) {
        arg_vector_.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            std::string arg(argv[i]);
            arg_vector_.push_back(arg);
        }
    }

    void ParseOptions(OptionProcessor& parser) {
        arg_vector_.erase(arg_vector_.begin());
        std::vector<std::string>::iterator it = arg_vector_.begin();
        while (it != arg_vector_.end()) {
            std::string name;
            std::string value;
            if (parser.process(*it, name, value)) {
                if (!name.empty()) {
                    // This is where to handle options
                    std::cout << "Option name: " << name << ", value: " << value << std::endl;
                }
                it = arg_vector_.erase(it);
            }
            else ++it;
        }
    }

    OptionSet& GetOptionSet() { return options_; }

private:
    std::vector<std::string> arg_vector_;
    OptionSet options_;
};

int main(int argc, char* argv[]) {
    OptionUtil util;
    util.DefineOptions();
    util.AcceptArguments(argc, argv);
    OptionProcessor parser(util.GetOptionSet());
    util.ParseOptions(parser); 

    return 0;
}
