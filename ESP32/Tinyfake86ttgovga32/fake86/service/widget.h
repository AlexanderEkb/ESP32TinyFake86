#ifndef _SERVICE_WIDGET_H
#define _SERVICE_WIDGET_H

#include "service/list.h"
#include <stdint.h>

typedef struct rect_t
{
  uint32_t left;
  uint32_t top;
  uint32_t width;
  uint32_t height;
  rect_t() : left(0), top(0), width(0), height(0){};
  rect_t(uint32_t left, uint32_t top, uint32_t width, uint32_t height) : left(left), top(top), width(width), height(height){};
  rect_t &operator=(rect_t &rvalue)
  {
    this->left = rvalue.left;
    this->top = rvalue.top;
    this->width = rvalue.width;
    this->height = rvalue.height;
    return *this;
  }
} rect_t;

class Widget_t : public SimpleListItem_t
{
  public:
    rect_t area;
    virtual void repaint();
    virtual void onKey(uint8_t scancode) = 0;
    virtual void onSelected() {repaint();};
    virtual void onDeselected() {repaint();};
};

#endif /*_SERVICE_WIDGET_H */