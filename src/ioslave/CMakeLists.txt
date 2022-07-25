# build and install the staging ioslave

add_library(kio_filestash MODULE ioslave.cpp)

target_link_libraries (kio_filestash
    Qt::DBus
    KF5::KIOCore
    KF5::I18n
    KF5::ConfigCore
    KF5::ConfigGui
)

set_target_properties(kio_filestash PROPERTIES OUTPUT_NAME "filestash")
install(TARGETS kio_filestash DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio/)
