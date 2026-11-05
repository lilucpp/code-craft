import QtQuick
import QtQuick.Controls

Button {
    id: control

    // Variants: "default" | "primary" | "danger" | "dashed" | "text"
    property string buttonType: "default"
    property string buttonSize: "middle" // "small" | "middle" | "large"

    implicitHeight: buttonSize === "small" ? AntTheme.controlHeightSm :
                    buttonSize === "large" ? AntTheme.controlHeightLg : AntTheme.controlHeightBase
    implicitWidth: Math.max(buttonSize === "small" ? 64 : 80, contentItem.implicitWidth + 30)

    font.family: AntTheme.fontFamily
    font.pixelSize: AntTheme.fontSizeBase
    font.weight: AntTheme.fontWeightRegular

    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? AntTheme.textDisabled :
               control.buttonType === "primary" || control.buttonType === "danger" ? "#FFFFFF" :
               control.hovered ? AntTheme.primaryHover : AntTheme.textPrimary
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitHeight: control.implicitHeight
        radius: AntTheme.radiusControl

        color: {
            if (!control.enabled) return AntTheme.bgLayout;
            if (control.buttonType === "primary") {
                return control.down ? AntTheme.primaryActive : control.hovered ? AntTheme.primaryHover : AntTheme.primary;
            }
            if (control.buttonType === "danger") {
                return control.down ? "#D9363E" : control.hovered ? "#FF7875" : AntTheme.error;
            }
            if (control.buttonType === "text") {
                return control.down ? "rgba(0,0,0,0.08)" : control.hovered ? "rgba(0,0,0,0.04)" : "transparent";
            }
            return control.down ? AntTheme.bgLayout : "#FFFFFF";
        }

        border.color: {
            if (!control.enabled) return AntTheme.border;
            if (control.buttonType === "primary") return AntTheme.primary;
            if (control.buttonType === "danger") return AntTheme.error;
            if (control.buttonType === "text") return "transparent";
            return control.hovered ? AntTheme.primaryHover : AntTheme.border;
        }
        border.width: control.buttonType === "text" ? 0 : 1

        Behavior on color {
            ColorAnimation { duration: AntTheme.durationFast }
        }
        Behavior on border.color {
            ColorAnimation { duration: AntTheme.durationFast }
        }
    }
}
