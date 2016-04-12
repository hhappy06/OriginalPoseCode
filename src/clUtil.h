#ifndef CLUTIL_H
#define CLUTIL_H
#include <CL/cl.h>
#ifdef __cplusplus
extern "C" {
#endif

#define clCHECKERROR(a, b) check_error(a, b, 0, __FILE__, __LINE__)
void check_error(cl_int value, cl_int reference, void(*clean_up)(int), const char *file_name, const int line_num);
#ifdef __cplusplus
}
#endif

#endif