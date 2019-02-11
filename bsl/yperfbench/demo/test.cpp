#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yperfbench.h"

int main(int argc, char ** argv)
{
	//printf("12\n");
	pb::init_opt(argc, argv);
	//printf("%d\n", pb::getopt<int>("para") + pb::getopt<int>("p"));
	pb::timer ti("yufan");
	pb::put_result("sum", pb::getopt<int>("para") + pb::getopt<int>("p"));
	printf("sdfishflsifhsldhfslkfh\n");
	ti.check();
	return 0;
}
