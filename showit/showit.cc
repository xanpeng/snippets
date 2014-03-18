#include <iostream>
#include <vector>
#include <iterator>
#include <cstdlib>

#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionProcessor.h>
#include <Poco/Util/OptionException.h>

#include <muduo/base/Logging.h>

using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionProcessor; 


class OptionUtil {
public:
    void ParseOptions(int argc, const char** args) {
        OptionSet options;
        DefineOptions(options);
        OptionProcessor parser(options);

        std::vector<std::string> argv(args + 1, args + argc);
        for (std::vector<std::string>::iterator it = argv.begin(); it != argv.end(); ) {
            std::string name, value;
            if (parser.process(*it, name, value)) {
                HandleOptions(name, value);
                it = argv.erase(it);
            }
            else ++it;
        }
    }

    bool show_detail() const { return show_detail_; }
    int process_id() const { return pid_; }

private:
    void DefineOptions(OptionSet& options) {
        options.addOption(Option("detail", "d", "show detail", false));
        options.addOption(Option("process", "p", "specify process id", true).argument("int"));
    }

    void HandleOptions(std::string& name, std::string& value) {
        if (!name.compare("detail")) show_detail_ = true;
        else if (!name.compare("process")) pid_ = ::atoi(value.c_str());
    }

    bool show_detail_;
    int pid_;
};

class Process {
public:
    explicit Process(int pid) : pid_(pid) {}
    
    //
    // Show parent id, command string, etc.
    //
    void ShowDetail() {
        LOG_INFO << "Not supported yet.";
    }

    void ShowLog() {
    }

private:
    void GainPermission() {

    }

    int pid_;
};

// 
// Usage: ./showit [options] -p $process-id
// options:
//      -d,--detail: show only statistic detail of process
// 
int main(int argc, const char** argv) {
    OptionUtil util;
    util.ParseOptions(argc, argv);
    Process process(util.process_id());
    if (util.show_detail())
        process.ShowDetail();
    else
        process.ShowLog();

    return 0;
}

