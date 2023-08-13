#ifndef STM32KBD_H
#define STM32KBD_H

#include "keyboard.h"

class stm32keyboard : public KeyboardDriver {
  public:
    stm32keyboard(){};
    virtual void Init();
    virtual void Reset();
    virtual void Exec();
};

#endif /* STM32KBD_H */