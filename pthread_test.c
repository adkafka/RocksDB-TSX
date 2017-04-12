#include <pthread.h>
#include <stdio.h>


/* this function is run by the second thread */
void *inc_x(void *x_void_ptr)
{

    /* increment x to 100 */
    int *x_ptr = (int *)x_void_ptr;
    while(++(*x_ptr) < 100);

    printf("x increment finished\n");

    /* the function must return something - NULL will do */
    return NULL;

}

pthread_mutex_t mutex1;
int main()
{
    int z = pthread_mutex_init(&mutex1,NULL);
    pthread_mutex_lock(&mutex1);
    pthread_mutex_unlock(&mutex1);
    return 0;
}
