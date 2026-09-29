qt_add_library(rdp_views STATIC)

qt_add_qml_module(rdp_views
    URI Rdp.Views
    QML_FILES
        AddApplicationDialog.qml
        ApplicationFiltersPane.qml
        ApplicationsPane.qml
        AvailableFeaturesPane.qml
        BlocklistPane.qml
        BottomBar.qml
        CapturePage.qml
        ConnectionPage.qml
        ModuleHost.qml
        NotificationOverlay.qml
        OutputLogView.qml
        SettingsPage.qml
        SidebarView.qml
        SystemInfoView.qml
        SystemPage.qml
        TimeoutDialog.qml
    IMPORTS
        TARGET rdp_theme
        TARGET rdp_backend
        TARGET rdp_controls
)

devtools_target_options(rdp_views)

target_link_libraries(rdp_views
    PUBLIC
        Qt6::Quick
        rdp_theme
        rdp_backend
        rdp_controls
)
