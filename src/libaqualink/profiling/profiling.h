#ifndef AQ_PROFILING_H_
#define AQ_PROFILING_H_

#if defined (VTUNE_SUPPORT_ENABLED)

#include <ittnotify.h>

typedef __itt_domain* AQ_Perf_Domain_Ptr;
typedef __itt_string_handle* AQ_Perf_String_Handle_Ptr;
typedef __itt_id AQ_Perf_Id;

#define PROFILING_NULL __itt_null

#else

typedef void* AQ_Perf_Domain_Ptr;
typedef void* AQ_Perf_String_Handle_Ptr;
typedef int AQ_Perf_Id;

#define PROFILING_NULL 0

#endif

// Functions
AQ_Perf_Domain_Ptr profiling_create_domain(const char name[]);
AQ_Perf_String_Handle_Ptr profiling_create_string_handle(const char name[]);
void profiling_set_thread_name(const char name[]);
void profiling_begin_profile_task(const AQ_Perf_Domain_Ptr domain, AQ_Perf_Id task_id, AQ_Perf_Id parent_id, AQ_Perf_String_Handle_Ptr name);
void profiling_end_profile_task(const AQ_Perf_Domain_Ptr domain);

#endif // AQ_PROFILING_H_
