#include "temp_files.h"
#include <list>
#include <string>
#include <unistd.h>
#include <thread>         
#include <mutex>    

std::mutex mtx; 

std::list<std::string> temps;

void TempFiles::cleanup() {
  std::unique_lock<std::mutex> lck (mtx,std::defer_lock);
  //enter critical section
  lck.lock();

  for(auto temp: temps) {
    unlink(temp.c_str());
  }
  temps.empty();

  //leave critical section
  lck.unlock();
}

void TempFiles::onNewTempFile(const char* name) {
  temps.push_back(name);
}