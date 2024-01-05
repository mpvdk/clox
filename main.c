#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[])
{
	initVM();	

	freeVM();
	return 0;
}
