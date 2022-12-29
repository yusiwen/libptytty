cmake_minimum_required(VERSION 3.6)
project(libptytty VERSION 2.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(CheckTypeSize)
include(GNUInstallDirs)

set(VERSION ${PROJECT_VERSION})
set(prefix ${CMAKE_INSTALL_PREFIX})
set(includedir ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(libdir ${CMAKE_INSTALL_FULL_LIBDIR})

option(BUILD_SHARED_LIBS "build a shared library" ON)
option(UTMP_SUPPORT "enable utmp support" ON)
option(WTMP_SUPPORT "enable wtmp support (requires UTMP_SUPPORT)" ON)
option(LASTLOG_SUPPORT "enable lastlog support (requires UTMP_SUPPORT)" ON)

function(PT_FIND_FILE name id)
  if(DEFINED ${id})
    return()
  elseif(CMAKE_CROSSCOMPILING)
    message(STATUS "Checking for a fallback location of ${name} - define ${id} in config.h manually")
    set(${id} "" CACHE STRING "")
  else()
    foreach(arg ${ARGN})
      if(EXISTS ${arg})
        message(STATUS "Checking for a fallback location of ${name} - ${arg}")
        set(${id} ${arg} CACHE STRING "")
        return()
      endif()
    endforeach()
    message(STATUS "Checking for a fallback location of ${name} - not found")
    set(${id} "" CACHE STRING "")
  endif()
endfunction()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/test.cxx"
    "
     struct e { };

     void f ()
     {
       try
         {
           new e ();
         }
       catch (...)
         {
           throw;
         }
     }
    ")
  execute_process(
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp"
    COMMAND ${CMAKE_C_COMPILER} -x c++ -shared -fPIC -o test.so test.cxx -lsupc++
    OUTPUT_VARIABLE OUTPUT
    ERROR_VARIABLE OUTPUT
    RESULT_VARIABLE RESULT)
  if(RESULT EQUAL 0)
    message(STATUS "Checking for working libsupc++ - yes")
    set(HAVE_LIBSUPCXX 1 CACHE BOOL "")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "libsupc++ test succeeded with the following output:\n"
      "${OUTPUT}\n")
  else()
    message(STATUS "Checking for working libsupc++ - no")
    set(HAVE_LIBSUPCXX 0 CACHE BOOL "")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "libsupc++ test failed with the following output:\n"
      "${OUTPUT}\n")
  endif()
endif()

check_include_files(pty.h HAVE_PTY_H)
check_include_files(util.h HAVE_UTIL_H)
check_include_files(libutil.h HAVE_LIBUTIL_H)
check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files(stropts.h HAVE_STROPTS_H)

check_function_exists(revoke HAVE_REVOKE)
check_function_exists(_getpty HAVE__GETPTY)
check_function_exists(getpt HAVE_GETPT)
check_function_exists(posix_openpt HAVE_POSIX_OPENPT)
check_function_exists(isastream HAVE_ISASTREAM)
check_function_exists(setuid HAVE_SETUID)
check_function_exists(setreuid HAVE_SETREUID)
check_function_exists(setresuid HAVE_SETRESUID)

check_c_source_compiles(
  "#include <stdlib.h>

   int main ()
   {
     grantpt (0);
     unlockpt (0);
     ptsname (0);
   }
  "

  UNIX98_PTY)

if(NOT UNIX98_PTY)
  check_function_exists(openpty HAVE_OPENPTY)
  if(NOT HAVE_OPENPTY)
    find_library(LIBUTIL_PATH util)
    check_library_exists(util openpty ${LIBUTIL_PATH} HAVE_LIBUTIL)
    if(HAVE_LIBUTIL)
      set(HAVE_OPENPTY 1)
    endif()
  endif()
endif()

if(CMAKE_HOST_SOLARIS)
  # Enable declarations in utmp.h on Solaris when the XPG4v2 namespace is active
  set(__EXTENSIONS__ 1)
endif()

check_include_files("sys/types.h;utmp.h" HAVE_UTMP_H)
if(HAVE_UTMP_H)
  set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;utmp.h")
  check_type_size("struct utmp" STRUCT_UTMP)
  if (HAVE_STRUCT_UTMP)
    check_struct_has_member(
      "struct utmp"
      ut_host
      "sys/types.h;utmp.h"
      HAVE_UTMP_HOST)
    check_struct_has_member(
      "struct utmp"
      ut_pid
      "sys/types.h;utmp.h"
      HAVE_UTMP_PID)

    check_function_exists(updwtmp HAVE_UPDWTMP)
    check_include_files("sys/types.h;lastlog.h" HAVE_LASTLOG_H)
    if (HAVE_LASTLOG_H)
      set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;lastlog.h")
    endif()
    check_type_size("struct lastlog" STRUCT_LASTLOG)
  endif()
  set(CMAKE_EXTRA_INCLUDE_FILES)

  PT_FIND_FILE(
    utmp
    PT_UTMP_FILE
    "/var/run/utmp"
    "/var/adm/utmp"
    "/etc/utmp"
    "/usr/etc/utmp"
    "/usr/adm/utmp")

  PT_FIND_FILE(
    wtmp
    PT_WTMP_FILE
    "/var/log/wtmp"
    "/var/adm/wtmp"
    "/etc/wtmp"
    "/usr/etc/wtmp"
    "/usr/adm/wtmp")

  PT_FIND_FILE(
    lastlog
    PT_LASTLOG_FILE
    "/var/log/lastlog"
    "/var/adm/lastlog")
endif()

check_include_files(utmpx.h HAVE_UTMPX_H)
if(HAVE_UTMPX_H)
  set(CMAKE_EXTRA_INCLUDE_FILES "utmpx.h")
  check_type_size("struct utmpx" STRUCT_UTMPX)
  if (HAVE_STRUCT_UTMPX)
    check_struct_has_member(
      "struct utmpx"
      ut_host
      "sys/types.h;utmpx.h"
      HAVE_UTMPX_HOST)

    check_function_exists(updwtmpx HAVE_UPDWTMPX)
    check_function_exists(updlastlogx HAVE_UPDLASTLOGX)
    check_type_size("struct lastlogx" STRUCT_LASTLOGX)
  endif()
  set(CMAKE_EXTRA_INCLUDE_FILES)

  PT_FIND_FILE(
    wtmpx
    PT_WTMPX_FILE
    "/var/log/wtmpx"
    "/var/adm/wtmpx")

  PT_FIND_FILE(
    lastlogx
    PT_LASTLOGX_FILE
    "/var/log/lastlogx"
    "/var/adm/lastlogx")
endif()

if(CMAKE_HOST_SOLARIS)
  # Enable declarations of msg_control and msg_controllen on Solaris
  set(CMAKE_REQUIRED_QUIET ON)
  check_c_source_compiles(
    "
     int main ()
     {
     #if __STDC_VERSION__ < 199901L
       error
     #endif
     }
    "

    C99)
  set(CMAKE_REQUIRED_QUIET OFF)
  if(C99)
    set(_XOPEN_SOURCE 600)
  else()
    set(_XOPEN_SOURCE 500)
  endif()
  set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=${_XOPEN_SOURCE})

  find_library(LIBSOCKET_PATH socket)
  check_library_exists(socket sendmsg ${LIBSOCKET_PATH} HAVE_LIBSOCKET)
  if(HAVE_LIBSOCKET)
    set(CMAKE_REQUIRED_LIBRARIES socket)
  endif()
endif()

# Check for unix-compliant filehandle passing ability
check_c_source_compiles(
  "
   #include <stddef.h> // broken bsds (is that redundant?) need this
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <sys/uio.h>

   int main ()
   {
     struct msghdr msg;
     struct iovec iov;
     char buf [100];
     char data = 0;

     iov.iov_base = &data;
     iov.iov_len  = 1;

     msg.msg_iov        = &iov;
     msg.msg_iovlen     = 1;
     msg.msg_control    = buf;
     msg.msg_controllen = sizeof buf;

     struct cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);
     cmsg->cmsg_level = SOL_SOCKET;
     cmsg->cmsg_type  = SCM_RIGHTS;
     cmsg->cmsg_len   = 100;

     *(int *)CMSG_DATA (cmsg) = 5;

     return sendmsg (3, &msg, 0);
   }
  "

  HAVE_UNIX_FDPASS)
if(NOT HAVE_UNIX_FDPASS)
  message(FATAL_ERROR "libptytty requires unix-compliant filehandle passing ability")
endif()

check_c_source_runs(
  "
   #include <grp.h>
   #include <sys/stat.h>
   #include <sys/types.h>
   #include <unistd.h>

   int main ()
   {
     struct stat st;
     struct group *gr = getgrnam (\"tty\");
     char *tty = ttyname (0);
     return
       !(gr
         && tty
         && stat (tty, &st) == 0
         && st.st_gid == gr->gr_gid);
   }
  "

  TTY_GID_SUPPORT)

configure_file(
  config.h.cmake
  config.h)

add_library(ptytty
  src/c-api.C
  src/fdpass.C
  src/logging.C
  src/proxy.C
  src/ptytty.C)
target_include_directories(ptytty PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(ptytty PROPERTIES VERSION 0)
if(HAVE_LIBSUPCXX)
  set_target_properties(ptytty PROPERTIES LINKER_LANGUAGE C)
  target_link_libraries(ptytty PRIVATE supc++)
  set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
endif()
if(HAVE_LIBSOCKET)
  target_link_libraries(ptytty PRIVATE socket)
  list(APPEND LIBS -lsocket)
endif()
if(HAVE_LIBUTIL)
  target_link_libraries(ptytty PRIVATE util)
  list(APPEND LIBS -lutil)
endif()

configure_file(
  libptytty.pc.in
  libptytty.pc)

add_executable(c-sample eg/c-sample.c)
target_include_directories(c-sample PRIVATE src)
target_link_libraries(c-sample ptytty)

add_custom_target(alldoc
  COMMAND pod2man -n libptytty -r ${PROJECT_VERSION} -q\" -c LIBPTYTTY -s3
            < ${CMAKE_SOURCE_DIR}/doc/libptytty.3.pod
            > ${CMAKE_SOURCE_DIR}/doc/libptytty.3
  COMMAND pod2text
            < ${CMAKE_SOURCE_DIR}/doc/libptytty.3.pod
            > ${CMAKE_SOURCE_DIR}/README
  VERBATIM)

install(
  TARGETS ptytty
  DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(
  FILES src/libptytty.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
  FILES doc/libptytty.3
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
install(
  FILES ${CMAKE_BINARY_DIR}/libptytty.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(DISTFILES
  CMakeLists.txt
  COPYING
  Changes
  README
  config.h.cmake
  doc/libptytty.3
  doc/libptytty.3.pod
  eg/c-sample.c
  libptytty.pc.in
  src/c-api.C
  src/ecb.h
  src/estl.h
  src/fdpass.C
  src/libptytty.h
  src/logging.C
  src/proxy.C
  src/ptytty.C
  src/ptytty.h
  src/ptytty_conf.h)

set(DISTDIR ${PROJECT_NAME}-${PROJECT_VERSION})

add_custom_target(distdir
  DEPENDS alldoc
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND rm -rf ${CMAKE_BINARY_DIR}/${DISTDIR}
  COMMAND rsync -aRv ${DISTFILES} ${CMAKE_BINARY_DIR}/${DISTDIR})

add_custom_target(dist
  COMMAND tar cvf - ${DISTDIR} | gzip -vf9 > ${DISTDIR}.tar.gz)

add_dependencies(dist distdir)
