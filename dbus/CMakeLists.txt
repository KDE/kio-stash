# install the dbus interface xml file into the correct place
install(
    FILES org.kde.kio.StashNotifier.xml
    DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)
