#build and install the daemon for the ioslave

add_library(stashnotifier MODULE stashnotifier.cpp fs.cpp stash_adaptor.cpp)
#add_library(test_daemon MODULE  testdaemon.cpp stashnotifier.cpp fs.cpp stash_adaptor.cpp)

#kcoreaddons_desktop_to_json(stashnotifier stashnotifier.desktop DEFAULT_SERVICE_TYPE) #not sure what this is for tbh
target_link_libraries(stashnotifier
    KF5::KIOCore
    KF5::DBusAddons
    Qt5::DBus
    Qt5::Core
)

install(TARGETS stashnotifier DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)

# next target
set(testdaemon_SRCS
    testdaemon.cpp
    stashnotifier.cpp
    fs.cpp
    stash_adaptor.cpp
)

add_executable(testdaemon ${testdaemon_SRCS})

target_link_libraries(testdaemon
    KF5::KIOCore
    KF5::DBusAddons
    Qt5::DBus
    Qt5::Core
)
