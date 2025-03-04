#include "widget.h"

widget_t::widget_t()
{
  area = {0, 0, 0, 0};
  parent = nullptr;
  isFocused = false;
  children.clear();
}

widget_t::widget_t(rect_t r)
{
  area = r;
  parent = nullptr;
  isFocused = false;
  children.clear();
}

void widget_t::add(widget_t * c)
{
  children.push_front(c);
  c->parent = this;
}

void widget_t::remove(widget_t * c)
{
  children.remove(c);
  c->parent = nullptr;
}

void widget_t::repaint()
{
  for ( auto c = children.begin(); (c != children.end()); ++c)
  {
    widget_t * w = *c;
    w->repaint();
  }
}

void widget_t::toGlobal(rect_t & r)
{
  r.left += area.left;
  r.top += area.top;
  if(parent != nullptr)
  {
    parent->toGlobal(r);
  }
}

WidgetState_t widget_t::state()
{
  return WIDGET_STATE_ACTIVE;
}

bool widget_t::dispatch(Msg_t * msg)
{
  if(onMessage(msg)) {
    return true;
  } else {
    for ( auto c = children.begin(); c != children.end(); ++c)
    {
      widget_t * w = *c;
      if(w->onMessage(msg)) return true;
    }
    return false;
  }
}

bool widget_t::onMessage(Msg_t * msg)
{
  (void)msg;
  return false;
}

bool widget_t::onKey(uint8_t scancode)
{
  bool handled = false;
  for ( auto c = children.begin(); (c != children.end()) && !handled; ++c)
  {
    widget_t * w = *c;
    if(w->isFocused)
      handled = w->onKey(scancode);
  }
  return handled;
};
bool widget_t::onKeyPreview(uint8_t scancode)
{
  return false;
}

void widget_t::setFocus()
{
  widget_t * r = root();
  r->defocusChildrenRecursively();
  widget_t * w = this;
  while(w != nullptr)
  {
    w->isFocused = true;
    w = w->parent;
  }
}

widget_t * widget_t::root()
{
  widget_t * w = this;
  while(w->parent != nullptr)
    w = w->parent;
  return w;
}

void widget_t::defocusChildrenRecursively()
{
  for(auto c : children)
  {
    c->isFocused = false;
    c->defocusChildrenRecursively();
  }
}
