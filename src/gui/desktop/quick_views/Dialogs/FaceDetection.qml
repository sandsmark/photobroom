
import QtQuick 2.15
import "../Components" as Components

Item {
    GridView {
        id: guessedFaces
        objectName: "guessedFaces"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        state: "empty"

        model: {}

        states: [
            State {
                name: "empty"
                when: guessedFaces.model.count === 0

                PropertyChanges {
                    target: guessedFaces
                    height: 0
                }
            },
            State {
                name: "nonempty"
                when: guessedFaces.model.count !== 0

                PropertyChanges {
                    target: guessedFaces
                    height: cellHeight + 30
                }
            }
        ]

        transitions: Transition {
            PropertyAnimation { properties: "height"; easing.type: Easing.InOutQuad; duration: 200 }
        }
    }

    GridView {
        id: unknownFaces
        objectName: "unknownFaces"

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: guessedFaces.bottom
        anchors.bottom: parent.bottom
        delegate: Item {
            x: 5
            height: 50
            Column {
                Rectangle {
                    width: 40
                    height: 40
                    color: colorCode
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    x: 5
                    text: name
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                spacing: 5
            }
        }
        model: {}
        cellWidth: 70
        cellHeight: 70
    }

    Components.ThumbnailSlider {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }


}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
