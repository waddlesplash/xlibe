#ifndef EVENT_H
#define EVENT_H

union _XEvent;
typedef union _XEvent XEvent;

#include <list>
#include <kernel/OS.h>

class Events {
private:
	sem_id counter_;
	sem_id lock_;
	sem_id wait_;
	std::list<XEvent*> list_;
	thread_id waiting_thread_;

	Events();
	void wait_for_coming();
	bool is_match(long mask, long event) const;

public:
	static Events& instance();
	void add(XEvent* event);
	void wait_for_next(XEvent* event);
	void wait_event(XEvent* event, long event_mask);
};

#endif
