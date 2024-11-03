#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using std::cout;
using std::thread;
using std::vector;
int main() {
  std::mutex mtx;
  auto func = [&mtx](int n) {
    std::lock_guard lg(mtx);
    cout << "hi : " << n << "\n";
  };
  vector<thread> threads;
  for (int i = 0; i < 10; i++) {
    threads.emplace_back(func, i);
  }
  for (auto& t : threads) {
    t.join();
  }

}
