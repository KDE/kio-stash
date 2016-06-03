cmake_minimum_required(VERSION 3.5)
set(QT_MIN_VERSION     "5.4.0")
set(KF5_MIN_VERSION    "5.16.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(
    CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
        ${ECM_KDE_MODULE_DIR}
)

# set up kf5

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(FeatureSummary)
add_library(kio_staging MODULE kio_staging.cpp)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED KIO)
find_package(KF5Archive ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5DBusAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Service ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5DocTools ${KF5_DEP_VERSION})
find_package(KF5Solid ${KF5_DEP_VERSION} REQUIRED)

target_link_libraries (kio_staging
 KF5::KIOCore
 Qt5::DBus
 KF5::I18n
 KF5::ConfigCore
 KF5::ConfigGui
)
set_target_properties(kio_staging PROPERTIES OUTPUT_NAME "staging")

install(TARGETS kio_staging DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kio )
