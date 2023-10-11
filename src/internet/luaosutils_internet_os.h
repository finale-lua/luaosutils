//
//  luaosutils_os.h
//  luaosutils
//
//  Created by Robert Patterson on 9/9/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_os_h
#define luaosutils_os_h

#include <memory>
#include <map>
#include <mutex>

namespace luaosutils
{

using lua_callback = std::function<void (bool, const std::string&)>;
using HeadersMap = std::map<std::string, std::string>;

static const int kHTTPStatusCodeOK = 200;

#if OPERATING_SYSTEM == WINDOWS
#include <wininet.h>

enum class win_request_state
{
   SEND,
   ALLOCATE,
   READ_CHUNK,
   CHUNK_COMPLETE,
   TERMINATE,
   COMPLETE
};

struct win_request_context
{
   win_request_state state;
   lua_callback callbackFunction;
   HINTERNET hInternet{};
   HINTERNET hConnect{};
   HINTERNET hRequest{};
   HANDLE hEvent{};
   DWORD statusCode{};
   DWORD readErrorCode{};
   LONG bufferReserve{};
   std::string buffer{};
   std::string postData{};
   std::vector<CHAR> readBuf{};
   DWORD numBytesRead{};
   
   win_request_context(lua_callback callback) :
               state(win_request_state::SEND),
               callbackFunction(callback)
   {
      hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   }
   
   ~win_request_context()
   {
      this->set_timer_id(0);
      if (hEvent) CloseHandle(hEvent);
      if (hRequest) InternetCloseHandle(hRequest);
      if (hConnect) InternetCloseHandle(hConnect);
      if (hInternet) InternetCloseHandle(hInternet);
   }

   static win_request_context* get_context_from_timer(UINT_PTR timerID)
   {
      auto it = get_timer_map().find(timerID);
      if (it == get_timer_map().end()) return nullptr;
      return it->second;
   }

   UINT_PTR get_timer_id() const { return timerID; }

   bool set_timer_id(UINT_PTR id)
   {
      if (timerID)
      {
         ::KillTimer(NULL, timerID);
         get_timer_mutex().lock();
         get_timer_map().erase(timerID);
         get_timer_mutex().unlock();
      }
      if (id)
      {
         get_timer_mutex().lock();
         get_timer_map().emplace(id, this);
         get_timer_mutex().unlock();
      }
      this->timerID = id;
      return (id != 0);
   }

private:
   UINT_PTR timerID{};
   
   static std::mutex& get_timer_mutex()
   {
      static std::mutex timerMapMutex;
      return timerMapMutex;
   }
   
   static std::map<UINT_PTR, win_request_context*>& get_timer_map()
   {
      static std::map<UINT_PTR, win_request_context*> timerMap;
      return timerMap;
   }
};
using OSSESSION = win_request_context;
#endif //OPERATING_SYSTEM == WINDOWS

#if OPERATING_SYSTEM == MAC_OS
void cancel_session(void* sessionTask);

struct mac_request_context
{
   lua_callback callbackFunction;
   void* sessionTask{}; // NSURLSessionDataTask* on Obj-C
   int statusCode{};
   std::string buffer{};
   bool success{};
   
   mac_request_context(lua_callback callback) : callbackFunction(callback)
   {
      _id = get_new_id();
      get_id_mutex().lock();
      get_id_map().emplace(_id, this);
      get_id_mutex().unlock();
   }

   ~mac_request_context()
   {
      if (sessionTask)
         cancel_session(sessionTask);
      get_id_mutex().lock();
      get_id_map().erase(_id);
      get_id_mutex().unlock();
   }
   
   void complete_request()
   {
      callbackFunction(success, buffer);
      cancel_session(sessionTask);
      sessionTask = nullptr;
   }

   static mac_request_context* get_context_from_id(size_t val)
   {
      auto it = get_id_map().find(val);
      if (it == get_id_map().end()) return nullptr;
      return it->second;
   }

   size_t get_id() const { return _id; }
   //void set_id(size_t val) { _id = val; }
   
private:
   size_t _id{};
   
   static std::mutex& get_id_mutex()
   {
      static std::mutex idMutex;
      return idMutex;
   }
   
   static std::map<size_t, mac_request_context*>& get_id_map()
   {
      static std::map<size_t, mac_request_context*> idMap;
      return idMap;
   }

   static size_t get_new_id() // this should always be used to calculate the id
   {
      static std::mutex mtx;
      static size_t _highestId = 0;
      std::lock_guard<std::mutex> lock(mtx);
      return ++_highestId;
   }
};
using OSSESSION = mac_request_context;

#endif //OPERATING_SYSTEM == MAC_OS

using OSSESSION_ptr = std::unique_ptr<OSSESSION>;

OSSESSION_ptr https_request(const std::string& requestType, const std::string &urlString, const std::string& postData,
                            const HeadersMap& headers, double timeout, lua_callback callback);

void error_message_box(const std::string &msg);

std::string server_name(const std::string &url);

}

#endif /* luaosutils_os_h */
