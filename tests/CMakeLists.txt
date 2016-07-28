include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(slavetest_SRCS
    slavetest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ioslave/ioslave.cpp
)
enable_testing()
set(CMAKE_AUTOMOC_ON)
add_executable(slavetest ${slavetest_SRCS})
add_test(slavetest slavetest)

target_link_libraries(slavetest
   KF5::I18n
   Qt5::DBus
   KF5::KIOCore
   KF5::Solid
   Qt5::Test
)
