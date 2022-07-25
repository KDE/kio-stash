project(KIOFileStash)
cmake_minimum_required(VERSION 3.5)
set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.96.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.84.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(
    CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(FeatureSummary)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING) #prevents implicit conversion of QString to QUrl
add_definitions(-DTRANSLATION_DOMAIN=\"kio5_stash\")

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus
    Test
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    KIO
    DBusAddons
    I18n
)

add_subdirectory(dbus)
add_subdirectory(src)
add_subdirectory(tests)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
