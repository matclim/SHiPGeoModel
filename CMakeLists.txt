cmake_minimum_required(VERSION 3.16)
project(LeadPlateGeoModel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GeoModelCore REQUIRED)
# GeoModelIO provides GeoModelDBManager + GeoModelWrite (WriteGeoModel.h)
find_package(GeoModelIO REQUIRED)

add_library(LeadPlateGeoPlugin SHARED
  src/BarLayer.cpp
  src/MaterialManager.cpp
  src/CalorimeterBuilder.cpp
  src/ConfigReader.cpp
  src/Fibre_HPLayer.cpp
)

target_include_directories(LeadPlateGeoPlugin PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(LeadPlateGeoPlugin PUBLIC
  GeoModelCore::GeoModelKernel
  GeoModelIO::GeoModelWrite
)

add_executable(make_leadplate_db
  src/make_db.cpp
)

target_include_directories(make_leadplate_db PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(make_leadplate_db PRIVATE
  LeadPlateGeoPlugin
  GeoModelCore::GeoModelKernel
  GeoModelIO::GeoModelWrite
)
