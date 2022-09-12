//
//  luaosutils_callback_session.hpp
//  luaosutils
//
//  Created by Robert Patterson on 9/11/22.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//

#ifndef luaosutils_callback_session_hpp
#define luaosutils_callback_session_hpp

#include <map>

#include "luaosutils.hpp"
#include "luaosutils_os.h"

/** \brief This class is used to guarantee that a Lua state is still active when a callback occurs.
 * A userdata of it is returned to Lua and the session stays active as long as
 * the Lua script maintains the returned value in scope. If the Lua state is closed,
 * the pointer is garbage collected, and the callback can no longer be called.
 */
class luaosutils_callback_session
{
public:
   using id_type = unsigned long;
private:
   using active_sessions_type = std::map<id_type, luaosutils_callback_session*>;
   
   id_type m_ID;
   luabridge::LuaRef m_luaRef;
   OSSESSION_ptr m_osSession;

   static active_sessions_type& _get_active_sessions()
   {
      static active_sessions_type g_activeSessions;
      return g_activeSessions;
   }
   
public:
   /** \brief Constuctor.
    *
    * Because it may be necessary to know the id before constructing instances, the creation of the
    * unique instance identifier is external to the contructor function.
    *
    * \param luaRef A luabridge::LuaRef containing a Lua callback function.
    * \param id A process-level unique instance identifier. Use #get_new_session_id to generate it.
    */
   luaosutils_callback_session(luabridge::LuaRef& luaRef, id_type id) : m_luaRef(luaRef), m_ID(id), m_osSession(nullptr)
   {
      _get_active_sessions().emplace(id, this);
   }
   
   /** \brief Destructor. Attempts to cancel session if there is one. */
   ~luaosutils_callback_session()
   {
#if OPERATING_SYSTEM == MAC_OS
      if (this->os_session())
         __cancel_session(this->os_session());
#endif
      
      _get_active_sessions().erase(m_ID);
   }

   /** \brief Class-level function that generates a new value for use as a unique instance identifier. */
   static id_type get_new_session_id() // this should always be used to calculate the id
   {
      static id_type sessionID = 0;
      return ++sessionID;
   }

   /** \brief Returns the Lua state for this instance. */
   lua_State* state() const { return m_luaRef.state(); }
   
   /** \brief Returns the Lua function for this instance. */
   luabridge::LuaRef& function() { return m_luaRef; }
   
   /** \brief Returns the OS session pointer for this instance. */
   OSSESSION_ptr os_session() const { return m_osSession; }
 
   /** \brief Sets the OS session pointer for this instance. */
   void set_os_session(OSSESSION_ptr session) { m_osSession = session; }
   
   /** \brief Class-level function that finds a running session from its unique instance identifier.
    *
    * \param id The unique instance identifier to find.
    */
   static luaosutils_callback_session* get_session_for_id(id_type id)
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
   static bool is_valid_session(luaosutils_callback_session *session)
   {
      return get_session_for_id(session->m_ID) != nullptr;
   }
};

#endif /* luaosutils_callback_session_hpp */
