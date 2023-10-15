#ifndef _PORTS_H
#define _PORTS_H

#include <stdint.h>

typedef uint8_t(* portReader_t)(uint32_t address);
typedef void(* portWriter_t)(uint32_t address, uint8_t value);

// class node
// {
//   public:
//   node()
//   {
//     left = nullptr;
//     right = nullptr;
//   }
//   node *left;
//   node *right;
//   uint32_t address;

//   portReader_t reader;
//   portWriter_t writer;
//   uint8_t value;
// };

// class tree
// {
//   public:
//   static tree &getInstance()
//   {
//     return instance;
//   }

//   void insert(node *newNode)
//   {
//     node *pNode = root;
//     bool exit = false;
//     while (!exit)
//     {
//       if (pNode == nullptr)
//       {
//         pNode = newNode;
//         exit = true;
//       }
//       else
//       {
//         if (newNode->address > pNode->address)
//           pNode = pNode->right;
//         else
//           pNode = pNode->left;
//       }
//     }
//   }
//   node *get(uint32_t address)
//   {
//     node *pNode = root;
//     while (true)
//     {
//       if (pNode == nullptr)
//         return nullptr;
//       else if (pNode->address == address)
//         return pNode;
//       else
//       {
//         if (address > pNode->address)
//           pNode = pNode->right;
//         else
//           pNode = pNode->left;
//       }
//     }
//   }

//   private:
//   node *root;
//   static tree instance;
//   tree()
//   {
//     root = nullptr;
//   }
// };

void set_port_write_redirector(unsigned short int startport, unsigned short int endport, void *callback);
void set_port_read_redirector(unsigned short int startport, unsigned short int endport, void *callback);

void portWriteTiny(uint32_t numPort, unsigned char aValue);
unsigned char portReadTiny(unsigned short int numPort);
#endif
