#ifndef DISTRIBUTED_COUNTER_1_H
#  define DISTRIBUTED_COUNTER_1_H
// Implement a distributed counter with a thread local count in
// conformance with cache-conscious programming best practices in
// the lecture.
//
// Most of the complexity here is in managing the thread local counts
// as threads are created or destroyed. Study how we use various
// multithreading techniques to accurately and safely track these.

#include<mutex>
#include<shared_mutex>
#include<map>
#include<numeric>

namespace mpcs {
class DistributedCounter1 {
public:
  typedef long long value_type;
private:
  value_type count;
  std::shared_mutex mutable mtx;

public:
	DistributedCounter1() : count(0) {}
  void operator++() {
	  std::unique_lock lock(mtx);
	  ++count;
  }
  void operator++(int) {
	  std::unique_lock lock(mtx);
	  count++;
  }

  value_type get() const {
	  std::shared_lock lock(mtx);
	  return count;
  }
};
}
#endif