#include <stdio.h>
#include <Windows.h>
#include "fmt.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("usage: %s FileName\n", argv[0]);
		return 1;
	}
	Fat12Image image(argv[1]);
	image.ProcessAllFiles(OFF_RootDir);
}

