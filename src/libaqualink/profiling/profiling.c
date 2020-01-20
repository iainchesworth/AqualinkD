#include "profiling.h"

#include "aqualink.h"

#if defined (VTUNE_SUPPORT_ENABLED)

#include <ittnotify.h>

// Real Functions

AQ_Perf_Domain_Ptr profiling_create_domain(const char name[])
{
	return __itt_domain_create(name);
}

AQ_Perf_String_Handle_Ptr profiling_create_string_handle(const char name[])
{
	return __itt_string_handle_create(name);
}

void profiling_set_thread_name(const char name[])
{
	__itt_thread_set_name(name);
}

void profiling_begin_profile_task(const AQ_Perf_Domain_Ptr domain, AQ_Perf_Id task_id, AQ_Perf_Id parent_id, AQ_Perf_String_Handle_Ptr name)
{
	__itt_task_begin(domain, task_id, parent_id, name);
}

void profiling_end_profile_task(const AQ_Perf_Domain_Ptr domain)
{
	__itt_task_end(domain);
}


#else // defined (VTUNE_SUPPORT_ENABLED)

AQ_Perf_Domain_Ptr profiling_create_domain(const char name[])
{
	UNREFERENCED_PARAMETER(name);
	return 0;
}

AQ_Perf_String_Handle_Ptr profiling_create_string_handle(const char name[])
{
	UNREFERENCED_PARAMETER(name);
	return 0;
}

void profiling_set_thread_name(const char name[])
{
	UNREFERENCED_PARAMETER(name);

	// NO OP
}

void profiling_begin_profile_task(const AQ_Perf_Domain_Ptr domain, AQ_Perf_Id task_id, AQ_Perf_Id parent_id, AQ_Perf_String_Handle_Ptr name)
{
	UNREFERENCED_PARAMETER(domain);
	UNREFERENCED_PARAMETER(task_id);
	UNREFERENCED_PARAMETER(parent_id);
	UNREFERENCED_PARAMETER(name);

	// NO OP
}

void profiling_end_profile_task(const AQ_Perf_Domain_Ptr domain)
{
	UNREFERENCED_PARAMETER(domain);

	// NO OP
}

#endif // defined (VTUNE_SUPPORT_ENABLED)