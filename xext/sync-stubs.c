#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

int
XSyncQueryExtension(Display* dpy,
	int* event_base_return, int* error_base_return)
{
	return 0;
}

int
XSyncInitialize(Display* dpy,
	int* major_version_return, int* minor_version_return)
{
	return 0;
}

XSyncSystemCounter*
XSyncListSystemCounters(Display* dpy,
	int* n_counters_return)
{
	*n_counters_return = 0;
	return NULL;
}

void
XSyncFreeSystemCounterList(XSyncSystemCounter* list)
{
}

XSyncCounter
XSyncCreateCounter(Display* dpy, XSyncValue initial_value)
{
	return None;
}

Status
XSyncSetCounter(Display* dpy, XSyncCounter counter, XSyncValue value)
{
	return BadImplementation;
}

Status
XSyncChangeCounter(Display* dpy, XSyncCounter counter, XSyncValue value)
{
	return BadImplementation;
}

Status
XSyncDestroyCounter(Display* dpy, XSyncCounter counter)
{
	return BadImplementation;
}

Status
XSyncQueryCounter(Display* dpy, XSyncCounter counter,
	XSyncValue* value_return)
{
	return BadImplementation;
}

Status
XSyncAwait(Display* dpy, XSyncWaitCondition* wait_list, int n_conditions)
{
	return BadImplementation;
}

XSyncAlarm
XSyncCreateAlarm(Display* dpy, unsigned long values_mask, XSyncAlarmAttributes* values)
{
	return None;
}

Status
XSyncDestroyAlarm(Display* dpy, XSyncAlarm alarm)
{
	return BadImplementation;
}

Status
XSyncQueryAlarm(Display* dpy, XSyncAlarm alarm,
	XSyncAlarmAttributes* values_return)
{
	return BadImplementation;
}

Status
XSyncChangeAlarm(Display* dpy, XSyncAlarm alarm, unsigned long values_mask, XSyncAlarmAttributes* values)
{
	return None;
}

Status
XSyncSetPriority(Display* dpy, XID client_resource_id, int priority)
{
	return BadImplementation;
}

Status
XSyncGetPriority(Display* dpy, XID client_resource_id, int* return_priority)
{
	return BadImplementation;
}

XSyncFence
XSyncCreateFence(Display* dpy, Drawable d, Bool initially_triggered)
{
	return None;
}

Bool
XSyncTriggerFence(Display* dpy, XSyncFence fence)
{
	return False;
}

Bool
XSyncResetFence(Display* dpy, XSyncFence fence)
{
	return False;
}

Bool
XSyncDestroyFence(Display* dpy, XSyncFence fence)
{
	return False;
}

Bool
XSyncQueryFence(Display* dpy, XSyncFence fence, Bool* triggered)
{
	return False;
}

Bool
XSyncAwaitFence(Display* dpy, const XSyncFence* fence_list, int n_fences)
{
	return False;
}
