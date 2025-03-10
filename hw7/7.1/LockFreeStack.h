#ifndef LOCK_FREE_STACK_H
#  define LOCK_FREE_STACK_H
#include<atomic>
#include<memory>
using std::atomic;

namespace mpcs51044 {
// Linked list of integers
struct StackItem {
  StackItem(int val) : next(0), value(val) {}
  StackItem *next; // Next item, 0 if this is last
  int value;
};

struct StackHead {
  StackItem *link;      // First item, 0 if list empty
  unsigned count;      // How many times the list has changed (see lecture notes)
};

struct Stack {
  Stack();
  int pop();
  void push(int);
private:
  atomic<StackHead> head;
};

Stack::Stack()
{
  StackHead init;
  init.link = nullptr;
  init.count = 0;
  head.store(init);
}

// Pop value off list
int
Stack::pop()
{
    // What the head will be if nothing messed with it
    StackHead expected = head.load();
    StackHead newHead;
    bool succeeded = false;
    while(!succeeded) {
        if(expected.link == 0) {
            return 0; // List is empty
        }
        // What the head will be after the pop:
        newHead.link = expected.link->next;
        newHead.count = expected.count + 1;
        // Even if the compare_exchange fails, it updates expected.
        succeeded = head.compare_exchange_weak(expected, newHead);
    }
    int value = expected.link->value;
    delete expected.link;
    return value;
}

// Push an item onto the list with the given head
void
Stack::push(int val)
{
    // Create new stack item
    StackItem *newItem = new StackItem(val);
    
    // What the head currently is
    StackHead expected = head.load();
    StackHead newHead;
    bool succeeded = false;
    
    while(!succeeded) {
        // Set up the new item's next pointer to point to current head
        newItem->next = expected.link;
        
        // Prepare the new head that will point to our new item
        newHead.link = newItem;
        newHead.count = expected.count + 1;
        
        // Try to update the head atomically
        // If it fails, expected will be updated with the new head value
        succeeded = head.compare_exchange_weak(expected, newHead);
    }
}
}
#endif