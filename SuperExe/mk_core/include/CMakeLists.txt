# Install headers
install(FILES "mk_core.h"
        DESTINATION include/monkey/
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

file(GLOB headers "mk_core/*.h")
install(FILES ${headers}
        DESTINATION include/monkey/mk_core
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
