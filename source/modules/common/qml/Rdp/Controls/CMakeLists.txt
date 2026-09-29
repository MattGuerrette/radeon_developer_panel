qt_add_library(rdp_controls STATIC)

qt_add_qml_module(rdp_controls
    URI Rdp.Controls
    SOURCES
        notifications.h
        notifications.cpp
        rdptablemodel.h
        rdptablemodel.cpp
    QML_FILES
        CollapsiblePane.qml
        DataTable.qml
        EmptyStateLabel.qml
        LogCell.qml
        NavigationBar.qml
        NavigationTab.qml
        PathSettingRow.qml
        RdpButton.qml
        RdpCheckBox.qml
        RdpComboBox.qml
        RdpIcon.qml
        RdpIconButton.qml
        RdpLabel.qml
        RdpProgressBar.qml
        RdpRadioButton.qml
        RdpScrollBar.qml
        RdpSearchField.qml
        RdpSpinBox.qml
        RdpTabButton.qml
        RdpTextField.qml
        RdpToolButton.qml
        SplitViewHandle.qml
        StatusDot.qml
    IMPORTS
        TARGET rdp_theme
)

devtools_target_options(rdp_controls)

target_include_directories(rdp_controls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rdp_controls
    PUBLIC
        Qt6::Quick
        rdp_theme
)
