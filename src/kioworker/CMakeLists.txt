# build and install the staging KIO worker

add_library(kio_filestash MODULE filestash.cpp)

target_link_libraries (kio_filestash
    Qt::DBus
    KF${QT_MAJOR_VERSION}::KIOCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
)

set_target_properties(kio_filestash PROPERTIES OUTPUT_NAME "filestash")
install(TARGETS kio_filestash DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio/)
