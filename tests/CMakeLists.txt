#build the target for the test cases for the KIO worker

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(workertest_SRCS
    workertest.cpp
)

enable_testing()
set(CMAKE_AUTOMOC_ON)
add_executable(workertest ${workertest_SRCS})
add_test(workertest workertest)

target_link_libraries(workertest
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::DBusAddons
    Qt::Core
    KF${QT_MAJOR_VERSION}::I18n
    Qt::DBus
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::Solid
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
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::DBusAddons
    Qt::DBus
    Qt::Core
)
