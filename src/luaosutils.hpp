//
//  luaosutils.hpp
//  luaosutils
//
//  Created by Robert Patterson on 9/9/2022.
//  Copyright © 2022 Robert Patterson. All rights reserved.
//  (Usage permitted by MIT License. See LICENSE file in this repository.)
//

#ifndef luaosutils_hpp
#define luaosutils_hpp

#if defined(__GNUC__)
#define OPERATING_SYSTEM MAC_OS
#elif defined(_MSC_VER)
#define OPERATING_SYSTEM WINDOWS
#else
#define OPERATING_SYSTEM UNKNOWN
#endif

#include <stdio.h>
#include <string>
#include <functional>
#include <set>

#include "lua.hpp"

#ifndef __OBJC__
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#endif // __GNUC__
#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedObject.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // __GNUC__
#endif // __OBJC__

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifndef __OBJC__
/** \brief this class is used to make guarantee that a Lua state is still active when a callback occurs.
 * A RefCountedPtr of it is returned to Lua and the session stays active as long as
 * the Lua script maintains the returned value in scope. If the Lua state is closed,
 * the pointer goes out of scope, and the callback can no longer be called.
 */
class __luaosutils_callback_session
{
   using id_type = unsigned long;
   using active_sessions_type = std::set<id_type>;
   
   id_type m_ID;
   luabridge::LuaRef m_luaRef; // to take up some space
   
   static active_sessions_type& _get_active_sessions()
   {
      static active_sessions_type g_activeSessions;
      return g_activeSessions;
   }
   
   static id_type _get_session_id()
   {
      static id_type sessionID = 0;
      return ++sessionID;
   }
   
public:
   __luaosutils_callback_session(luabridge::LuaRef& luaRef) : m_luaRef(luaRef)
   {
      m_ID = _get_session_id();
      _get_active_sessions().insert(m_ID);
   }
   ~__luaosutils_callback_session()
   {
      _get_active_sessions().erase(m_ID);
   }

   lua_State* state() const { return m_luaRef.state(); }
   
   luabridge::LuaRef& function() { return m_luaRef; }
   
   static bool is_valid_session(__luaosutils_callback_session *session)
   {
      return _get_active_sessions().find(session->m_ID) != _get_active_sessions().end();
   }
};
#endif // __OBJC__

using __download_callback = std::function<void (bool, const std::string&)>;

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT int luaopen_luaosutils(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // luaosutils_hpp
