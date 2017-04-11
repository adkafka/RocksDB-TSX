#define UNW_LOCAL_ONLY

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <execinfo.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <libunwind.h>

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
    char **messages = (char **)NULL;
    int i, trace_size = 0;
    std::ostringstream cmd;
    std::stringstream out;
    std::string output,func,loc;


    trace_size = backtrace(trace, 16);
    messages = backtrace_symbols(trace, trace_size);
    for (i=0; i<trace_size; ++i)
    {
        /* find first occurence of '(' or ' ' in message[i] and assume
         *      * everything before that is the file name. (Don't go beyond 0 though
         *           * (string terminator)*/
        size_t p = 0;
        while(messages[i][p] != '(' && messages[i][p] != ' '
                && messages[i][p] != 0)
            ++p;

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

void backtrace_bad() {
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    unsigned cur_frame = 0;
    std::ostringstream cmd;
    std::stringstream out;
    std::string output,func,loc;

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0) {
        /* Get pc register value */
        unw_word_t pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }

        /* Use 'addr2line' to get the function name, file, and line number */
        cmd.str("");
        cmd << "addr2line -C -e " << "pthread_test " << "-f -i 0x";
        cmd << std::hex << pc;

        output = exec(cmd.str().c_str());
        out = std::stringstream(output);
        std::getline(out,func,'\n');
        std::getline(out,loc,'\n');

        std::cout << "(" << cur_frame << ")\n";
        std::cout << loc << " [" << func << "]" << std::endl;

        cur_frame++;
    }
}