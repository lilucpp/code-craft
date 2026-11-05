pragma Singleton
import QtQuick

QtObject {
    id: root

    // --- 1. Colors & Semantic Palette ---
    readonly property color primary: "#1677FF"
    readonly property color primaryHover: "#4096FF"
    readonly property color primaryActive: "#0958D9"
    readonly property color primaryBg: "#E6F4FF"

    readonly property color success: "#52C41A"
    readonly property color warning: "#FAAD14"
    readonly property color error: "#FF4D4F"
    readonly property color info: "#1677FF"

    // Surfaces (3-Layer Topology)
    readonly property color bgLayout: "#F5F5F5"
    readonly property color bgContainer: "#FFFFFF"
    readonly property color bgElevated: "#FFFFFF"

    // Neutrals
    readonly property color textPrimary: "#1F1F1F"    // rgba(0,0,0,0.88)
    readonly property color textSecondary: "#595959"  // rgba(0,0,0,0.65)
    readonly property color textDisabled: "#BFBFBF"   // rgba(0,0,0,0.25)
    readonly property color border: "#D9D9D9"         // rgba(0,0,0,0.15)
    readonly property color borderSecondary: "#F0F0F0"// rgba(0,0,0,0.06)

    // --- 2. Typography ---
    readonly property string fontFamily: "Segoe UI, Microsoft YaHei, PingFang SC, sans-serif"
    readonly property int fontSizeSm: 12
    readonly property int fontSizeBase: 14
    readonly property int fontSizeMd: 16
    readonly property int fontSizeLg: 20
    readonly property int fontSizeXl: 24

    readonly property int fontWeightRegular: Font.Normal  // 400
    readonly property int fontWeightBold: Font.DemiBold   // 600

    readonly property int lineHeightBase: 22

    // --- 3. Sizing & Spacing (4px Grid) ---
    readonly property int spaceXs: 4
    readonly property int spaceSm: 8
    readonly property int spaceMd: 16
    readonly property int spaceLg: 24
    readonly property int spaceXl: 32

    readonly property int controlHeightSm: 24
    readonly property int controlHeightBase: 32
    readonly property int controlHeightLg: 40

    // --- 4. Border Radius ---
    readonly property int radiusControl: 6
    readonly property int radiusContainer: 8
    readonly property int radiusTag: 4
    readonly property int radiusPill: 9999

    // --- 5. Motion Durations ---
    readonly property int durationFast: 100
    readonly property int durationMid: 200
    readonly property int durationSlow: 300
}
