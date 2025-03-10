#ifndef DISTRIBUTED_COUNTER_2_H
#  define DISTRIBUTED_COUNTER_2_H
// Create multiple "sub-counters" so that different threads are unlikely
// to use the same sub-counter, reducing contention compared to DistributedCounter1
#include<atomic>
#include<mutex>
#include<shared_mutex>
#include<map>
#include<numeric>
#include<utility>
#include<vector>
#include<thread>
namespace mpcs {
class DistributedCounter2 {
  typedef size_t value_type;

  struct bucket {
    std::shared_mutex sm;
    value_type count;
  };
  static size_t const buckets{ 128 };
  std::vector<bucket> counts{ buckets };
public:
  
  void operator++() {
	size_t index = std::hash<std::thread::id>()(std::this_thread::get_id()) % buckets;
	std::unique_lock ul(counts[index].sm);
    counts[index].count++;
  }
  void operator++(int) {
	  ++*this;
  }

  value_type get() {
    return std::accumulate(counts.begin(), counts.end(), (value_type)0, 
		[](auto acc, auto &x) { std::shared_lock sl(x.sm); return acc + x.count; });
  }
};
}
#endif