cmake_minimum_required(VERSION 2.8.12)
set(QT5_MIN_VERSION "5.4.0")
set(KF5_MIN_VERSION "5.22.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(
    CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
        ${ECM_KDE_MODULE_DIR}
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(FeatureSummary)

find_package(Qt5 ${QT5_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    KIO
    DBusAddons
    I18n
)

#qt5_add_dbus_adaptor(STAGING_SRCS org.kde.StagingNotifier.xml stagingnotifier.h stagingnotifier)
#setup kioslave
add_library(kio_staging MODULE kio_staging.cpp)
target_link_libraries (kio_staging
 KF5::KIOCore
 Qt5::DBus
 KF5::I18n
 KF5::ConfigCore
 KF5::ConfigGui
)
set_target_properties(kio_staging PROPERTIES OUTPUT_NAME "staging")

install(TARGETS kio_staging DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio)

#setup kded
add_library(stagingnotifier MODULE stagingnotifier.cpp)
add_library(staging_adaptor STATIC staging_adaptor.cpp)
kcoreaddons_desktop_to_json(stagingnotifier stagingnotifier.desktop DEFAULT_SERVICE_TYPE) #not sure what this is for tbh
target_link_libraries(staging_adaptor Qt5::DBus Qt5::Core)
target_link_libraries(stagingnotifier staging_adaptor KF5::KIOCore KF5::DBusAddons Qt5::DBus Qt5::Core)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.StagingNotifier.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(TARGETS stagingnotifier DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)
