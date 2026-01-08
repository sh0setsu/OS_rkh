#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <utility>

template<class T>
class BufferedChannel {
public:
   explicit BufferedChannel(int size) : buffer_size_(size), closed_(false) {}

   ~BufferedChannel() {
      Close();
   }

   void Send(T value) {
      std::unique_lock<std::mutex> lock(mutex_);
   
      if (closed_) {
         throw std::runtime_error("Channel is closed");
      }
      
      cv_send_.wait(lock, [this]() {
         return queue_.size() < static_cast<size_t>(buffer_size_) || closed_;
      });
      
      if (closed_) {
         throw std::runtime_error("Channel is closed");
      }
      
      queue_.push(std::move(value));
      cv_recv_.notify_one();
   }
   
   std::pair<T, bool> Recv() {
      std::unique_lock<std::mutex> lock(mutex_);
   
      cv_recv_.wait(lock, [this]() {
         return !queue_.empty() || closed_;
      });
      
      if (queue_.empty()) {
         return {T{}, false};
      }
      
      T value = std::move(queue_.front());
      queue_.pop();
      cv_send_.notify_one();
      
      return {std::move(value), true};
   }
   
   void Close() {
      std::unique_lock<std::mutex> lock(mutex_);
      closed_ = true;
   
      cv_send_.notify_all();
      cv_recv_.notify_all();
   }

private:
   int buffer_size_;
   std::queue<T> queue_;
   std::mutex mutex_;
   std::condition_variable cv_send_;
   std::condition_variable cv_recv_;
   bool closed_;
};

#endif // BUFFERED_CHANNEL_H_
