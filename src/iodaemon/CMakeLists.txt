#build and install the daemon for the KIO worker

add_library(stashnotifier MODULE stashnotifier.cpp fs.cpp stash_adaptor.cpp)

target_link_libraries(stashnotifier
    KF6::KIOCore
    KF6::DBusAddons
    Qt::DBus
    Qt::Core
)

install(TARGETS stashnotifier DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)
