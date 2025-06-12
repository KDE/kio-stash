#build the target for the test cases for the KIO worker

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(workertest_SRCS
    workertest.cpp
)

enable_testing()
set(CMAKE_AUTOMOC_ON)
add_executable(workertest ${workertest_SRCS})
add_test(NAME workertest COMMAND workertest)

target_link_libraries(workertest
    KF6::KIOCore
    KF6::DBusAddons
    Qt::Core
    KF6::I18n
    Qt::DBus
    KF6::KIOCore
    KF6::Solid
    Qt::Test
)

#next target
set(testdaemon_SRCS
    testdaemon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/iodaemon/stashnotifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/iodaemon/fs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/iodaemon/stash_adaptor.cpp
)

add_executable(testdaemon ${testdaemon_SRCS})

target_link_libraries(testdaemon
    KF6::KIOCore
    KF6::DBusAddons
    Qt::DBus
    Qt::Core
)
