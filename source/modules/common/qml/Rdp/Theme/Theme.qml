pragma Singleton

import QtQuick

QtObject {
    id: root

    // Ordered to match QtCommon::QtUtils::ColorThemeOption, which is what the
    // "application_theme" setting stores.
    enum ColorTheme {
        Light,
        Dark,
        System
    }

    // Mirrors the RDP "Color Theme" setting. Main.qml binds this to the saved value.
    property int colorTheme: Theme.System

    readonly property bool systemDark: Application.styleHints.colorScheme === Qt.Dark
    readonly property bool darkMode: colorTheme === Theme.System
                                     ? systemDark
                                     : colorTheme === Theme.Dark

    // -- Qt palette equivalents -------------------------------------------
    readonly property color window: darkMode ? "#232323" : "#ffffff"
    readonly property color windowText: darkMode ? "#f0f0f0" : "#000000"
    readonly property color base: darkMode ? "#232323" : "#ffffff"
    readonly property color alternateBase: darkMode ? "#141414" : "#f0f0f0"
    readonly property color text: windowText
    readonly property color disabledText: "#808080"
    readonly property color placeholderText: "#808080"
    readonly property color highlight: darkMode ? Qt.rgba(40 / 255, 80 / 255, 160 / 255, 140 / 255)
                                                : "#8fc1e7"
    readonly property color highlightedText: darkMode ? "#ffffff" : "#000000"
    readonly property color link: darkMode ? "#2a82da" : "#0000ff"
    readonly property color tooltipBase: darkMode ? "#3c3c3c" : "#f0e6c8"

    readonly property color light: darkMode ? "#4b4b4b" : "#ffffff"
    readonly property color midlight: darkMode ? "#2a2a2a" : "#cacaca"
    readonly property color mid: darkMode ? "#262626" : "#b8b8b8"
    readonly property color dark: darkMode ? "#212121" : "#9f9f9f"

    // -- RDP specific ------------------------------------------------------
    readonly property color navigationBar: "#333333"
    readonly property color navigationText: "#666666"
    readonly property color navigationTextHover: "#999999"
    readonly property color navigationTextSelected: "#ffffff"
    readonly property color accent: "#e01e37"
    readonly property color accentDim: "#871410"

    readonly property color sidebar: light
    readonly property color paneBackground: midlight
    readonly property color paneBorder: darkMode ? light : dark
    readonly property color controlBorder: darkMode ? "#5a5a5a" : "#9f9f9f"
    readonly property color controlBackground: base
    readonly property color hover: darkMode ? "#3a3a3a" : "#e6e6e6"
    readonly property color separator: mid

    readonly property color iconAccent: "#2a60ad"
    readonly property color iconMuted: disabledText
    readonly property color statusOnline: "#24a600"
    readonly property color statusOffline: "#ff0000"
    readonly property color statusGood: darkMode ? "#4fc24f" : "#008000"
    readonly property color statusWarning: "#c7a426"
    readonly property color statusError: "#e01e37"

    // -- Qt Quick Controls palette -----------------------------------------
    // Feeds the Fusion style the colours above, so the parts the Rdp* controls
    // do not override themselves -- focus rings, popup frames, header sections,
    // scroll bars, text selection -- draw in RDP's colours rather than the
    // host system palette. Applied once on the root ApplicationWindow, from
    // where it propagates down the item tree.
    readonly property Palette controlPalette: Palette {
        window: root.window
        windowText: root.windowText
        base: root.base
        alternateBase: root.alternateBase
        text: root.text
        button: root.controlBackground
        buttonText: root.windowText
        brightText: root.highlightedText
        light: root.light
        midlight: root.midlight
        mid: root.mid
        dark: root.dark
        shadow: root.dark
        highlight: root.highlight
        highlightedText: root.highlightedText
        link: root.link
        linkVisited: root.link
        toolTipBase: root.tooltipBase
        toolTipText: root.windowText
        placeholderText: root.placeholderText
        accent: root.accent

        disabled.text: root.disabledText
        disabled.windowText: root.disabledText
        disabled.buttonText: root.disabledText
        disabled.placeholderText: root.placeholderText
    }

    // -- Metrics -----------------------------------------------------------
    readonly property int navigationBarHeight: 44
    readonly property int paneSpacing: 15
    readonly property int paneHeaderHeight: 34
    readonly property int contentMargin: 12
    readonly property int sidebarMargin: 10
    readonly property int controlHeight: 26
    readonly property int iconButtonSize: 20
    readonly property int tabHeight: 30

    // -- Typography (point sizes mirror the RDP stylesheet) ----------------
    readonly property real bodyPointSize: 9.75
    readonly property real smallPointSize: 8
    readonly property real tablePointSize: 9
    readonly property real tabPointSize: 8.5
    readonly property real navigationPointSize: 10
    readonly property string monoFamily: "monospace"
}
