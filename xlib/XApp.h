#ifndef XAPP_H
#define XAPP_H

#include <Application.h>

class XApp : public BApplication {
private:
  void reply_screen(BMessage* message);
public:
  void MessageReceived(BMessage* message);
  //bool QuitRequested();
  void ReadyToRun();
  XApp(const char* signature);
};

#endif
