#pragma once

class Context {
  public:
    virtual int id();
    virtual void remove();
    virtual void pauseRead();
    virtual void pauseWrite();
    virtual bool isValid();
};

class Handler {
  void handleRead(Context ctx);
  void handleClosed();
  void handleInit();
  void handleAccept();
};

class EventLoop {
  public:
    EventLoop();
    virtual ~EventLoop();

    void add(Handler *handler);
    void remove(Handler *handler);
    void start(); // start the event loop in the curent thread
    void stop(); // stop the event loop, can be called from the different thread, which has access to the server
};

class Server : public Handler { 
  public:
    virtual void handleAccept(Context *ctx);
};
