cmake_minimum_required(VERSION 3.8)

option(BUILD_CLIENT "build robot's client components" ON)
option(BUILD_COMMON  "build robot's common components" ON)
option(BUILD_V1 "build robot's V1 components" ON)
option(BUILD_MODULES "build robot's V1 components" ON)
option(BUILD_EXAMPLES "build robot's examples" ON)

if(BUILD_CLIENT)
    message(DEBUG "Adding SDK-Client client subdirectory...")
    add_subdirectory(client)
endif()

if(BUILD_COMMON)
    message(DEBUG "Adding SDK-Client Common subdirectory...")
    add_subdirectory(common)
endif()

# if(BUILD_V1)
#     message(DEBUG "Adding SDK-Client V1 subdirectory...")
#     add_subdirectory(v1)
# endif()

if(BUILD_MODULES)
    message(DEBUG "Adding SDK-Client BUILD_MODULES subdirectory...")
    add_subdirectory(modules)
endif()

# if(BUILD_EXAMPLES)
#     message(DEBUG "Adding SDK-Client BUILD_EXAMPLES subdirectory...")
#     add_subdirectory(examples)
# endif()
