#include "XApp.h"
#include <kernel/OS.h>

extern thread_id main_thread;

XApp::XApp(const char* signature) : BApplication(signature) {
}

void XApp::MessageReceived(BMessage *message) {
	switch(message->what) {
	default:
		BApplication::MessageReceived(message);
	}
}

/*bool XApp::QuitRequested() {
  return (CountWindows() == 0);
}*/

void XApp::ReadyToRun() {
	resume_thread(main_thread);
}
