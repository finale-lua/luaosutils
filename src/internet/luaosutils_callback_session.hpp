//
//  luaosutils::callback_session.hpp
//  luaosutils
//
//  Created by Robert Patterson on 9/11/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_callback_session_hpp
#define luaosutils_callback_session_hpp

#include <map>
#include <mutex>

#include "luaosutils.hpp"
#include "internet/luaosutils_internet_os.h"

namespace luaosutils
{

constexpr const char (&kSessionMetatableKey)[] = "luaosutils_callback_session";

/** \brief This class is used to guarantee that a Lua state is still active when a callback occurs.
 * A userdata of it is returned to Lua and the session stays active as long as
 * the Lua script maintains the returned value in scope. If the Lua state is closed,
 * the pointer is garbage collected, and the callback can no longer be called.
 */
class callback_session
{
public:
   using id_type = unsigned long;
private:
   using active_sessions_type = std::map<id_type, luaosutils::callback_session*>;
   
   id_type m_ID;
   lua_State* m_L;
   int m_function;
   OSSESSION_ptr m_osSession;
   bool m_reportErrors;
   
   static active_sessions_type& _get_active_sessions()
   {
      static active_sessions_type g_activeSessions;
      return g_activeSessions;
   }
   
   static std::mutex& get_active_sessions_mutex()
   {
      static std::mutex activeSessionsMutex;
      return activeSessionsMutex;
   }
   
public:
   /** \brief Constructor.
    *
    * Because it may be necessary to know the id before constructing instances, the creation of the
    * unique instance identifier is external to the contructor function.
    *
    * \param L The Lua state.
    * \param func A reference to a Lua callback function.
    * \param id A process-level unique instance identifier. Use #get_new_session_id to generate it.
    */
   callback_session(lua_State* L, int func, id_type id) : m_L(L), m_function(func), m_ID(id), m_reportErrors(true)
   {
      get_active_sessions_mutex().lock();
      _get_active_sessions().emplace(id, this);
      get_active_sessions_mutex().unlock();
   }
   
   /** \brief Destructor. Attempts to cancel session if there is one. */
   ~callback_session()
   {
      get_active_sessions_mutex().lock();
      luaL_unref(m_L, LUA_REGISTRYINDEX, m_function);
      _get_active_sessions().erase(m_ID);
      get_active_sessions_mutex().unlock();
   }
   
   /** \brief Class-level function that generates a new value for use as a unique instance identifier. */
   static id_type get_new_session_id() // this should always be used to calculate the id
   {
      static std::mutex mtx;
      static id_type sessionID = 0;
      std::lock_guard<std::mutex> lock(mtx);
      return ++sessionID;
   }
   
   /** \brief Returns the Lua state for this instance. */
   lua_State* state() const { return m_L; }
   
   /** \brief Returns the Lua function for this instance. */
   int function() { return m_function; }
   
   ///** \brief Returns the OS session pointer for this instance. */
   //OSSESSION* os_session() const { return m_osSession.get(); }
   
   /** \brief Sets the OS session pointer for this instance. */
   void set_os_session(OSSESSION_ptr& session)
   {
      if (m_osSession.get() != session.get())
         m_osSession = std::move(session);
   }
   
   /** \brief Cancels any running request. */
   void cancel() { m_osSession = nullptr; }
   
   /** \brief Returns whether to report errors in a dialog box. */
   bool report_errors() const { return m_reportErrors; }
   
   /** \brief Sets whether to report errors in a dialog box. */
   void set_report_errors(bool state) { m_reportErrors = state; }
   
   /** \brief Class-level function that finds a running session from its unique instance identifier.
    *
    * \param id The unique instance identifier to find.
    */
   static luaosutils::callback_session* get_session_for_id(id_type id)
   {
      auto it = _get_active_sessions().find(id);
      if (it == _get_active_sessions().end())
         return nullptr;
      return it->second;
   }
   
   /** \brief Class-level function that returns true if the input session is currently still valid.
    *
    * Session instances are returned to Lua as userdata. Client Lua programs must maintain their references in
    * Lua variables. If the Lua state is closed of if the Lua variable goes out of scope and is garabage collected,
    * the session can no longer be used.
    *
    * \param session The session to find.
    * \returns true if the session is valid or false if it has been destroyed by Lua garbage collection.
    */
   static bool is_valid_session(luaosutils::callback_session *session)
   {
      return get_session_for_id(session->m_ID) != nullptr;
   }
};

}

#endif /* luaosutils_callback_session_hpp */
