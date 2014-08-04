# Install script for directory: C:/opencv/sources/doc

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/opencv/sources/build/install")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/doc" TYPE FILE FILES
    "C:/opencv/sources/doc/haartraining.htm"
    "C:/opencv/sources/doc/check_docs_whitelist.txt"
    "C:/opencv/sources/doc/CMakeLists.txt"
    "C:/opencv/sources/doc/license.txt"
    "C:/opencv/sources/doc/packaging.txt"
    "C:/opencv/sources/doc/opencv.jpg"
    "C:/opencv/sources/doc/acircles_pattern.png"
    "C:/opencv/sources/doc/opencv-logo-white.png"
    "C:/opencv/sources/doc/opencv-logo.png"
    "C:/opencv/sources/doc/opencv-logo2.png"
    "C:/opencv/sources/doc/pattern.png"
    "C:/opencv/sources/doc/opencv2manager.pdf"
    "C:/opencv/sources/doc/opencv2refman.pdf"
    "C:/opencv/sources/doc/opencv_cheatsheet.pdf"
    "C:/opencv/sources/doc/opencv_tutorials.pdf"
    "C:/opencv/sources/doc/opencv_user.pdf"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/doc/vidsurv" TYPE FILE FILES
    "C:/opencv/sources/doc/vidsurv/Blob_Tracking_Modules.doc"
    "C:/opencv/sources/doc/vidsurv/Blob_Tracking_Tests.doc"
    "C:/opencv/sources/doc/vidsurv/TestSeq.doc"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

