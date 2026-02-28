cmake_minimum_required(VERSION 3.16)
project(LeadPlateGeoModel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GeoModelCore REQUIRED)
find_package(GeoModelIO REQUIRED)
find_package(GeoModelG4 REQUIRED)

# ---- Geant4 ----
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# ---- ROOT (for digitisation executable) ----
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)

# ---- Geometry construction library ----
add_library(CaloGeo STATIC
  src/MaterialManager.cpp
  src/ConfigReader.cpp
  src/CalorimeterBuilder.cpp
  src/BarLayer.cpp
  src/Fibre_HPLayer.cpp
)
target_include_directories(CaloGeo PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(CaloGeo PUBLIC
  GeoModelCore::GeoModelKernel
)

# ---- Digitisation library ----
add_library(CaloSysDigitiser STATIC
  src/CaloSysDigitiser.cc
)
target_include_directories(CaloSysDigitiser PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- DB writer ----
add_executable(make_leadplate_db
  src/make_db.cpp
)
target_include_directories(make_leadplate_db PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(make_leadplate_db PRIVATE
  CaloGeo
  GeoModelIO::GeoModelWrite
  GeoModelIO::GeoModelDBManager
)

# ---- Geant4 simulation ----
add_executable(run_g4
  src/main_g4.cpp
  src/DetectorConstruction.cc
  src/CaloSD.cc
  src/EventStore.cc
  src/RunAction.cc
  src/EventAction.cc
  src/RunConfig.cc
  src/PrimaryGeneratorAction.cc
)
target_include_directories(run_g4 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(run_g4 PRIVATE
  CaloGeo
  ${Geant4_LIBRARIES}
)

# Link GeoModel -> Geant4 bridge (target names differ across installs)
if (TARGET GeoModelG4::GeoModel2G4)
  target_link_libraries(run_g4 PRIVATE GeoModelG4::GeoModel2G4)
elseif (TARGET GeoModel2G4)
  target_link_libraries(run_g4 PRIVATE GeoModel2G4)
else()
  message(FATAL_ERROR "GeoModel2G4 target not found. Check targets exported by GeoModelG4Config.cmake")
endif()

if (TARGET GeoModelG4::GeoMaterial2G4)
  target_link_libraries(run_g4 PRIVATE GeoModelG4::GeoMaterial2G4)
elseif (TARGET GeoMaterial2G4)
  target_link_libraries(run_g4 PRIVATE GeoMaterial2G4)
endif()

# ---- Digitisation executable ----
add_executable(digitise_calo;
  src/digitise_calo.cpp   
)
target_include_directories(digitise_calo PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ROOT modern targets:
if (TARGET ROOT::Core)
  target_link_libraries(digitise_calo PRIVATE
    CaloSysDigitiser 
    ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  )
else()
  # Fallback for older ROOT configs
  include(${ROOT_USE_FILE})
  target_include_directories(digitise_calo PRIVATE ${ROOT_INCLUDE_DIRS})
  target_link_libraries(digitise_calo PRIVATE CaloSysDigitiser ${ROOT_LIBRARIES})
endif()
