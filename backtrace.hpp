#define UNW_LOCAL_ONLY

#include <array>
#include <cstdio>
#include <cstdlib> 
#include <execinfo.h> //backtrace
#include <iostream> //cout...
#include <memory> //shared_ptr
#include <sstream>
#include <string>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return result;
}

void backtrace(){
    void *trace[16];
    int i, trace_size = 0;
    std::ostringstream cmd;
    std::stringstream out;
    std::string output,func,loc;


    trace_size = backtrace(trace, 16);
    for (i=0; i<trace_size; ++i) {
        cmd.str("");
        cmd << "LD_PRELOAD=\"\" addr2line -C -e " << "pthread_test " << "-f -i ";
        cmd << std::hex << trace[i];

        output = exec(cmd.str().c_str());
        out = std::stringstream(output);
        std::getline(out,func,'\n');
        std::getline(out,loc,'\n');

        std::cout << "(" << i << ")\n";
        std::cout << loc << " [" << func << "]" << std::endl;
    }
}
