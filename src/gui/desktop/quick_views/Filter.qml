import QtQuick 2.0
import QtQuick.Controls 2.15

Item {
    id: column

    height: childrenRect.height
    width: childrenRect.width

    clip: true

    state: "simpleState"

    Row {
        id: row

        Text {
            id: search
            text: qsTr("Search:")
        }

        TextInput{
            id: searchExpression
            width: 200
        }

        Switch {
            id: advancedOptionsButton
            text: qsTr("Advanced")
        }

    }

    Row {
        id: row1
        anchors.top: row.bottom

        Text {
            id: categoriesText
            text: qsTr("Category:")
        }

        ComboBox {
            id: comboBox
        }
    }

    states: [
        State {
            name: "simpleState"
            when: advancedOptionsButton.checked == false

            PropertyChanges {
                target: column
                height: row.height
            }
        },
        State {
            name: "advancedState"
            when: advancedOptionsButton.checked

            PropertyChanges {
                target: column
                height: row.height + row1.height
            }
        }
    ]

    transitions: Transition {
        PropertyAnimation { properties: "height"; easing.type: Easing.InOutQuad; duration: 200 }
    }
}


