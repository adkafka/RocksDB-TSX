#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>

void backtrace() {
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    // This should point to the instruction pointer
    std::printf("0x%lx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      std::printf(" (%s+0x%lx)\n", nameptr, offset);
      std::free(demangled);
    } else {
      std::printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
}

void backtrace_fd(){
    void *array[10];
    size_t size;

    size = backtrace(array,10);

    backtrace_symbols_fd(array,size,1);
    for(int i=0; i<10; i++){
        printf("%p\n",array[i]);
    }
}




namespace ns {

template <typename T, typename U>
void foo(T t, U u) {
  backtrace(); // <-------- backtrace here!
}

}  // namespace ns

template <typename T>
struct Klass {
  T t;
  void bar() {
    ns::foo(t, true);
  }
};

int main(int argc, char** argv) {
  Klass<double> k;
  k.bar();

  return 0;
}
