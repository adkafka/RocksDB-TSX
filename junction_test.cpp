#include <junction/Core.h>
#include <turf/Heap.h>
#include <junction/extra/MapAdapter.h>
#include <iostream>

using namespace turf::intTypes;

int main() {
    junction::extra::MapAdapter adapter(1);
    junction::extra::MapAdapter::ThreadContext context(adapter, 0);
    junction::extra::MapAdapter::Map map(65536);

    context.registerThread();

    ureg population = 0;
    for (ureg i = 0; i < 100; i++) {
        for (; population < i * 5000; population++)
            map.assign(population + 1, (void*) ((population << 2) | 3));
    }

    context.update();
    context.unregisterThread();

    return 0;
}
