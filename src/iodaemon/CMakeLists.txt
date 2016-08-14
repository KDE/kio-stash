#build and install the daemon for the ioslave

add_library(stashnotifier MODULE stashnotifier.cpp fs.cpp stash_adaptor.cpp)

target_link_libraries(stashnotifier
    KF5::KIOCore
    KF5::DBusAddons
    Qt5::DBus
    Qt5::Core
)

install(TARGETS stashnotifier DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)
