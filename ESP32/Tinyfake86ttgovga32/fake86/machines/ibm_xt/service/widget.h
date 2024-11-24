#ifndef _SERVICE_WIDGET_H
#define _SERVICE_WIDGET_H

#include <stdint.h>
#include <list>

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

class widget_t
{
  public:
    widget_t()
    {
      area = {0, 0, 0, 0};
      parent = nullptr;
      focused = false;
      children.clear();
    }
    widget_t(rect_t r)
    {
      area = r;
      parent = nullptr;
      focused = false;
      children.clear();
    }
    rect_t area;
    void add(widget_t * c)
    {
      children.push_back(c);
      c->parent = this;
    }
    virtual void repaint() {};
    virtual bool onKey(uint8_t scancode)
    {
      bool handled = false;
      for ( auto c = children.begin(); (c != children.end()) && !handled; ++c)
      {
        widget_t * w = *c;
        if(w->focused)
          handled = w->onKey(scancode);
      }
      return handled;
    };
    virtual bool onKeyPreview(uint8_t scancode) {return false;};
    virtual void onFocusChanged(bool focused) { repaint(); };
    void setFocus()
    {
      widget_t * r = root();
      r->_defocusChildren();
      widget_t * w = this;
      while(w != nullptr)
      {
        w->_setFocus(true);
        w = w->parent;
      }
    }
  protected:
    widget_t * parent;
    std::list<widget_t *> children;
    bool focused;
    widget_t * root()
    {
      widget_t * w = this;
      while(w->parent != nullptr)
        w = w->parent;
      return w;
    }
    void _defocusChildren()
    {
      for(auto c : children)
      {
        c->_setFocus(false);
        c->_defocusChildren();
      }
    }
    void _setFocus(bool f)
    {
      if(f != focused)
        repaint();
      focused = f;
    }
};

#endif /*_SERVICE_WIDGET_H */