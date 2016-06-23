project(KIOFileStash)
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

add_subdirectory(dbus)
add_subdirectory(src)

#qt5_add_dbus_adaptor(STAGING_SRCS org.kde.StagingNotifier.xml stashnotifier.h stashnotifier)
#setup kioslave

#setup kded
add_library(stashnotifier MODULE stashnotifier.cpp fs.cpp)
add_library(stash_adaptor STATIC stash_adaptor.cpp)
kcoreaddons_desktop_to_json(stashnotifier stashnotifier.desktop DEFAULT_SERVICE_TYPE) #not sure what this is for tbh
target_link_libraries(stash_adaptor Qt5::DBus Qt5::Core)
target_link_libraries(stashnotifier stash_adaptor KF5::KIOCore KF5::DBusAddons Qt5::DBus Qt5::Core)
install(TARGETS stashnotifier DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
