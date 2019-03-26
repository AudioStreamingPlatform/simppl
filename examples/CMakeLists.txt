
include_directories(. ../include)


add_executable(echo
    echo/main.cpp
)

target_link_libraries(echo simppl)


add_executable(mixed 
	mixed/mixed.cpp
)

target_link_libraries(mixed simppl)

if(HAVE_OBJECT_MANAGER)
    add_executable(object_manager object_manager/object_manager.cpp)
    target_link_libraries(object_manager simppl)
endif()
