
import QtQuick 2.14
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.14


Item
{
    id: rootId
    objectName: "photos_view"       // used by c++ part to find this view and set proper model

    property alias model: photosViewId.model

    ColumnLayout {

        anchors.fill: parent

        TimeRange {
            id: timeRangeId
        }

        GridView {
            id: photosViewId

            property int thumbnailSize: 160
            property int thumbnailMargin: 2

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            clip: true
            cellWidth: thumbnailSize + thumbnailMargin
            cellHeight: thumbnailSize + thumbnailMargin
            delegate: PhotoDelegate { }
            highlight: highlightId
            keyNavigationEnabled: true

            ScrollBar.vertical: ScrollBar { }

            Component {
                id: highlightId

                Rectangle {
                    width: view.cellWidth;
                    height: view.cellHeight
                    color: "lightsteelblue";
                    opacity: 0.7
                    x: view.currentItem.x
                    y: view.currentItem.y
                    z: 1
                    Behavior on x { SpringAnimation { spring: 3; damping: 0.2 } }
                    Behavior on y { SpringAnimation { spring: 3; damping: 0.2 } }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
