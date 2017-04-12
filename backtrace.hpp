#define UNW_LOCAL_ONLY

#include <array>
#include <cstdio>
#include <cstdlib> 
#include <execinfo.h> //backtrace
#include <fstream> //file writing
#include <memory> //shared_ptr
#include <sstream>
#include <string>

#ifndef EXEC_PATH
#define EXEC_PATH "rocksdb/db_bench"
#endif

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL){
            result += buffer.data();
        }
    }
    return result;
}

void backtrace(std::ofstream *ofs){
    void *trace[16];
    int i, trace_size = 0;
    std::ostringstream cmd;
    std::stringstream out;
    std::string output,func,loc;


    trace_size = backtrace(trace, 16);
    // Ignore first two frames (interpose func and this bt)
    for (i=2; i<trace_size; ++i) {
        cmd.str("");
        cmd << "LD_PRELOAD=\"\" addr2line -C -e " << EXEC_PATH << " -f -i ";
        cmd << std::hex << trace[i];

        //output = exec(cmd.str().c_str());
        out = std::stringstream(output);
        std::getline(out,func,'\n');
        std::getline(out,loc,'\n');

        (*ofs) << "(" << i-1 << ") ";
        (*ofs) << cmd.str() << "\n";
        //(*ofs) << loc << " [" << func << "]" << std::endl;
    }
}
