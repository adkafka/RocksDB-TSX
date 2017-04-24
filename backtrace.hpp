#define UNW_LOCAL_ONLY

#include <array>
#include <cstdio>
#include <cstdlib> 
#include <iostream> 
#include <execinfo.h> //backtrace
#include <fstream> //file writing
#include <memory> //shared_ptr
#include <sstream>
#include <string>

#include <junction/ConcurrentMap_Grampa.h>


#ifndef EXEC_PATH
//#define EXEC_PATH "pthread_test"
#define EXEC_PATH "rocksdb/db_bench"
#endif

typedef junction::ConcurrentMap_Grampa<turf::u64, char*> ConcMap;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    /* Get rid of LD_PRELOAD */
    unsetenv("LD_PRELOAD");
    FILE* pipe = popen(cmd,"r");
    if (!pipe){
        perror("popen() failed!");
        exit(1);
    }
    while (!feof(pipe)) {
        if (fgets(buffer.data(), 128, pipe) != NULL){
            result += buffer.data();
        }
    }
    pclose(pipe);
    return result;
}

void my_backtrace(std::ofstream *ofs, ConcMap* cache){
    void *trace[16];
    int i, trace_size = 0;


    trace_size = backtrace(trace, 16);
    // Ignore first two frames (interpose func and this bt)
    for (i=2; i<trace_size; ++i) {
        turf::u64 instruction = reinterpret_cast<turf::u64>(trace[i]);

        ConcMap::Mutator mut = cache->insertOrFind(instruction);
        char* buf = mut.getValue();
        if(!buf){
            std::ostringstream cmd,new_val;
            std::stringstream out;
            std::string output,func,loc;

            buf = new char[256];

            cmd.str("");
            new_val.str("");
            cmd << "LD_PRELOAD=\"\" addr2line -C -e " << EXEC_PATH << " -f -i ";
            cmd << std::hex << trace[i];
            output = exec(cmd.str().c_str());

            out = std::stringstream(output);
            std::getline(out,func,'\n');
            std::getline(out,loc,'\n');

            new_val << "(" << i-1 << ") ";
            new_val << loc << " [" << func << "]" << std::endl;

            strncpy(buf,new_val.str().c_str(),255);
            buf[255]='\0';

            mut.exchangeValue(buf);
        }
        (*ofs) << buf;
    }
}
