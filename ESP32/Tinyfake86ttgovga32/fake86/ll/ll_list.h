#ifndef _LL_LIST_H_
#define _LL_LIST_H_

namespace ll
{
  class listEntry
  {
    public:
      listEntry * _next;
      listEntry * _prev;
      void * data;
      listEntry()
      {
        _next = nullptr;
        _prev = nullptr;
        data = nullptr;
      };
      virtual bool greaterThan(listEntry * rvalue) = 0;
  };

  class list
  {
    public:
      list()
      {
        _root = nullptr;
      }

      virtual void add(listEntry * entry)
      {
        if(isEmpty())
        {
          _root = entry;
          _root->_prev = _root;
          _root->_next = _root;
        }
        else
        {
          entry->_prev = _root->_prev;
          entry->_next = _root;
          entry->_prev->_next = entry;
          entry->_next->_prev = entry;
        }
      }

      virtual void addToHead(listEntry * entry)
      {
        if(isEmpty())
          add(entry);
        else
        {
          entry->_prev = _root;
          entry->_next = _root->_next;
          _root->_next->_prev = entry;
          _root->_next = entry;
          _root = entry;
        }
      }
      virtual bool remove(listEntry * entry)
      {
        if(isEmpty())
          return false;
        if(entry == nullptr)
          return false;
        if(entry->_prev == nullptr)
          return false;
        if(entry->_next == nullptr)
          return false;
        entry->_prev->_next = entry->_next;
        entry->_next->_prev = entry->_prev;

        if(entry == _root)
          _root = (entry->_next == entry)?nullptr:entry->_next;

        entry->_next = nullptr;
        entry->_prev = nullptr;
      }
      bool isEmpty() {return _root == nullptr;};
    protected:
      listEntry * _root;
  };

  class sortedList : public list
  {
    public:
      virtual void add(listEntry * entry) override
      {
        if(isEmpty())
        {
          _root = entry;
          _root->_prev = _root;
          _root->_next = _root;
        }
        else
        {
          // find a proper place
          listEntry * e = _root;
          while(entry->greaterThan(e->_next))
            if(e->_next == _root)
              break;
            else
              e = e->_next;
          // e->_next >= entry
          entry->_prev = e;
          entry->_next = e->_next;
          entry->_prev->_next = entry;
          entry->_next->_prev = entry;
        }
      }
  };
}

#endif /* _LL_LIST_H_ */