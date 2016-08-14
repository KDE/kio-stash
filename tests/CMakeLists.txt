#build the target for the test cases for the ioslave

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(slavetest_SRCS
    slavetest.cpp
)

enable_testing()
set(CMAKE_AUTOMOC_ON)
add_executable(slavetest ${slavetest_SRCS})
add_test(slavetest slavetest)

target_link_libraries(slavetest
    KF5::KIOCore
    KF5::DBusAddons
    Qt5::Core
    KF5::I18n
    Qt5::DBus
    KF5::KIOCore
    KF5::Solid
    Qt5::Test
)

# next target
set(testdaemon_SRCS
    testdaemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/iodaemon/stashnotifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/iodaemon/fs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/iodaemon/stash_adaptor.cpp
)

add_executable(testdaemon ${testdaemon_SRCS})

target_link_libraries(testdaemon
    KF5::KIOCore
    KF5::DBusAddons
    Qt5::DBus
    Qt5::Core
)
