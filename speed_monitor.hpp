#ifndef GAISWT_SPEED_MONITOR_HPP_
#define GAISWT_SPEED_MONITOR_HPP_

#include <chrono>
#include <boost/cstdint.hpp>
#include <iostream>
struct speed_monitor 
{
  unsigned int average_speed()
  {
    std::cout << total_amount_ << "/" << total_elapsed_ << "\n";
    return total_amount_ / total_elapsed_;
  }

protected:

  void start_monitor()
  {
    start_ = std::chrono::system_clock::now();
    total_amount_ = total_elapsed_ = 0;
  }

  void update_monitor(boost::uint32_t amount)
  { 
    using namespace std::chrono;

    total_elapsed_ += 
      duration_cast<seconds>(system_clock::now() - start_).count();

    start_ = std::chrono::system_clock::now();
    total_amount_ += amount;
  }

  void stop_monitor()
  {
    using namespace std::chrono;

    total_elapsed_ +=
      duration_cast<seconds>(system_clock::now() - start_).count();
  }

private:

  std::chrono::time_point<std::chrono::system_clock> start_;
  boost::uint32_t total_amount_;
  unsigned int total_elapsed_;
};

#endif // header guard
