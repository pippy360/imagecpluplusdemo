# Install script for directory: /Users/tmurphy/git/image/opencv/modules

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/tmurphy/git/image/opencv/opencv_default/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/calib3d/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/core/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/dnn/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/features2d/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/flann/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/gapi/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/highgui/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/imgcodecs/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/imgproc/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/java/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/js/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/ml/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/objdetect/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/photo/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/python/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/stitching/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/ts/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/video/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/videoio/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/.firstpass/world/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/core/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/flann/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/imgproc/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/photo/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/python_tests/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/dnn/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/features2d/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/calib3d/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/objdetect/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/video/cmake_install.cmake")
  include("/Users/tmurphy/git/image/opencv/opencv_default/modules/js/cmake_install.cmake")

endif()
