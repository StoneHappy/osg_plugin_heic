######################### OSG ######################################
find_package(OpenSceneGraph 3.6.0 REQUIRED osgDB)

if(OSG_FOUND)
    message("FOUND OSG")
else()
    message("NOT FOUND OSG")
endif()

find_package(libheif CONFIG REQUIRED)

add_subdirectory(Vendors)
add_subdirectory(Runtime)