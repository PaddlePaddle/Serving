#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "yperfbench.h"
namespace pb
{

int argc;
char ** argv;

int output_counter=0;
output outputs[40];
pthread_mutex_t perfmutex = PTHREAD_MUTEX_INITIALIZER;
void print_result()
{	
	for(int i=0; i<output_counter; i++)
	{
		printf("%s:%s ", outputs[i].name, outputs[i].value);
	}
}

void init_opt(int  _argc, char ** _argv)
{
	argc = _argc;
	argv = _argv;
	atexit(print_result);
}


void put_result(char * name, char * value)
{
	pthread_mutex_lock(&perfmutex);
	int p = output_counter++;
	pthread_mutex_unlock(&perfmutex);
	strncpy(outputs[p].name, name, sizeof(outputs[0].name)-1);
	outputs[p].name[sizeof(outputs[0].name)-1] = 0;
	strncpy(outputs[p].value, value, sizeof(outputs[0].value)-1);
	outputs[p].value[sizeof(outputs[0].value)-1] = 0;
}
void put_result(char * name, double value)
{
	char buf[__YPERFPENCH_VALUE_LENGTH];
	snprintf(buf, sizeof(buf), "%.3f", value);
	put_result(name, buf); 
}

void put_result(char * name, int value)
{
	char buf[__YPERFPENCH_VALUE_LENGTH];
	snprintf(buf, sizeof(buf), "%d", value);
	put_result(name, buf); 
}
#include <unistd.h>
#include <sys/time.h>
timer::timer(char * _name)
{
	strncpy(name, _name, sizeof(name)-1);
	name[sizeof(name)-1]=0;
	gettimeofday(&start, NULL);
}
timer::timer()
{
	strcpy(name, "timer");
	gettimeofday(&start, NULL);
}
void timer::check()
{
	timeval end;
	gettimeofday(&end, NULL);
	put_result(name, 1000 * (end.tv_sec - start.tv_sec) + 0.001 * (end.tv_usec -start.tv_usec));
}


void run_threads(void *fun(void*), int num)
{
	if(num == 1)
	{
		fun(0);
		return;
	}
	pthread_t *threads = (pthread_t*) calloc(num, sizeof(threads));
	for(int i=0; i<num; i++)
	{
		pthread_create(&threads[i],NULL,fun,NULL);
	}
    for(int t=0; t<num; t++)
    {
		pthread_join(threads[t], NULL);
	}
}


}
