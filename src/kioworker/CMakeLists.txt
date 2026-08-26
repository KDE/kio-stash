# build and install the staging KIO worker

add_library(kio_filestash MODULE filestash.cpp)

target_link_libraries (kio_filestash
    Qt::DBus
    KF6::KIOCore
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigGui
)

set_target_properties(kio_filestash PROPERTIES
    OUTPUT_NAME "filestash"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kio"
)
install(TARGETS kio_filestash DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kio/)
