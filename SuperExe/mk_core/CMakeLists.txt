SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
include (CheckCSourceCompiles)

set(src
  mk_iov.c
  mk_file.c
  mk_rconf.c
  mk_string.c
  mk_memory.c
  mk_event.c
  mk_utils.c
  mk_rbtree.c
  )

# Validate timerfd_create()
check_c_source_compiles("
  #include <sys/timerfd.h>
  int main() {
     return timerfd_create(CLOCK_REALTIME, 0);
  }" HAVE_TIMERFD_CREATE)

if (HAVE_TIMERFD_CREATE)
  add_definitions(-DHAVE_TIMERFD_CREATE)
endif()

# Validate eventfd()
check_c_source_compiles("
  #include <sys/eventfd.h>
  int main() {
     return eventfd(0, EFD_CLOEXEC);
  }" HAVE_EVENTFD)

if (HAVE_EVENTFD)
  add_definitions(-DHAVE_EVENTFD)
endif()

include_directories(include)
add_library(mk_core STATIC ${src})
target_link_libraries(mk_core ${CMAKE_THREAD_LIBS_INIT})

if(NOT WITH_SYSTEM_MALLOC)
  target_link_libraries(mk_core libjemalloc)
endif()

add_subdirectory(include)