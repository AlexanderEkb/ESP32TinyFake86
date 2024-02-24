#ifndef _SERVICE_LIST_H
#define _SERVICE_LIST_H

class SimpleListItem_t
{
  public:
    SimpleListItem_t * next;
    void * owner;
    SimpleListItem_t() : next(nullptr), owner(nullptr) {};
};

class SimpleList_t
{
  public:
    SimpleList_t() : root(nullptr), selection(nullptr) {};
    void add(SimpleListItem_t * item)
    {
      if(root == nullptr)
      {
        root = item;
        selection = item;
      }
      else
      {
        SimpleListItem_t * i = root;
        while(i->next != root)
          i = i->next;
      }
      item->next = root;
      item->owner = reinterpret_cast<void *>(this);
    }
    virtual void next()
    {
      if(selection != nullptr)
        selection = selection->next;
    }
    SimpleListItem_t * getSelection()
    {
      return selection;
    }
  private:
    SimpleListItem_t * root;
    SimpleListItem_t * selection;
};

#endif /* _SERVICE_LIST_H */