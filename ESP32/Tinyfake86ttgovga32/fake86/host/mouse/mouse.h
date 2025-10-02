#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <stdint.h>

typedef struct MouseEvent_t
{
  int32_t dx;
  int32_t dy;
  int32_t btn;
} MouseEvent_t;

class Mouse_t
{
  public:
    virtual ~Mouse_t() {};
    virtual void init() = 0;
    virtual bool poll(MouseEvent_t * e) = 0;
};

#endif /* __MOUSE_H__ */