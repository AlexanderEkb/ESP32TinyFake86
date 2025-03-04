#ifndef _SERVICE_WIDGET_H
#define _SERVICE_WIDGET_H

#include <stdint.h>
#include <list>

static const uint32_t ACTUAL_FONT_WIDTH = 8; //SERVICE_FONT_WIDTH + 1;
static const uint32_t ACTUAL_FONT_HEIGHT = 8; //SERVICE_FONT_HEIGHT + 1;

typedef enum {
  WIDGET_STATE_ACTIVE,
  WIDGET_STATE_CONFIRMED,
  WIDGET_STATE_CANCELLED
} WidgetState_t;

typedef struct rect_t
{
  public:
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
    rect_t() : left(0), top(0), width(0), height(0){};
    rect_t(uint32_t left, uint32_t top, uint32_t width, uint32_t height) : left(left), top(top), width(width), height(height){};
    rect_t &operator=(rect_t rvalue)
    {
      this->left = rvalue.left;
      this->top = rvalue.top;
      this->width = rvalue.width;
      this->height = rvalue.height;
      return *this;
    }
} rect_t;

typedef enum {
  EVENT_KEYPRESS,
  __EVENT_COUNT
} Event_t ;

class Msg_t {
  public:
    Event_t event;
    int32_t param1;
    int32_t param2;
  private:
};

class widget_t
{
  public:
    rect_t area;

    widget_t();
    widget_t(rect_t r);
    void add(widget_t * c);
    void remove(widget_t * c);
    virtual void repaint();
    void toGlobal(rect_t & r);
    virtual WidgetState_t state();
    bool dispatch(Msg_t * msg);
    virtual bool onKey(uint8_t scancode);
    virtual bool onKeyPreview(uint8_t scancode);
    void setFocus();
  protected:
    widget_t * parent;
    std::list<widget_t *> children;
    bool isFocused;
    widget_t * root();
    void defocusChildrenRecursively();
    virtual bool onMessage(Msg_t * msg);
};

#endif /*_SERVICE_WIDGET_H */