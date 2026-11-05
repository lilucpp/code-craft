import QtQuick

Rectangle {
    id: card

    property string title: ""
    default property alias content: innerContainer.data

    implicitWidth: 360
    implicitHeight: headerRow.visible ? innerContainer.implicitHeight + 64 : innerContainer.implicitHeight + 32

    radius: AntTheme.radiusContainer
    color: AntTheme.bgContainer
    border.color: AntTheme.borderSecondary
    border.width: 1

    Column {
        anchors.fill: parent
        anchors.margins: AntTheme.spaceMd
        spacing: AntTheme.spaceSm

        Row {
            id: headerRow
            width: parent.width
            visible: card.title.length > 0
            spacing: AntTheme.spaceSm

            Text {
                text: card.title
                font.family: AntTheme.fontFamily
                font.pixelSize: AntTheme.fontSizeMd
                font.weight: AntTheme.fontWeightBold
                color: AntTheme.textPrimary
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: AntTheme.borderSecondary
            visible: headerRow.visible
        }

        Item {
            id: innerContainer
            width: parent.width
            implicitHeight: childrenRect.height
        }
    }
}
