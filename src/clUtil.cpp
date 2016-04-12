#include "clUtil.h"
#include "stdafx.h"
#include <stdio.h>
#include <cassert>
#include "stdafx.h"
void check_error(cl_int value, cl_int reference, void(*clean_up)(int), const char *file_name, const int line_num)
{
	if(reference != value)
	{
		printf("\n !!! Error # %i at line %i, int file %s !!! \n", value, line_num, file_name);

		assert(0);
	}
}