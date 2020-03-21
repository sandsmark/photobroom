
import QtQuick 2.14
import photo_broom.qml 1.0


Item
{
    id: rootId

    GridView {
        id: photosViewId
        objectName: "photos_view"       // used by c++ part to find this view and set proper model

        property int thumbnailSize: 160
        property int thumbnailMargin: 2

        anchors.fill: parent

        delegate: delegateId
        cellWidth: thumbnailSize + thumbnailMargin
        cellHeight: thumbnailSize + thumbnailMargin
    }

    ScrollBar {
        id: verticalScrollBarId
        width: 12; height: photosViewId.height-12
        anchors.right: photosViewId.right
        opacity: 1
        orientation: Qt.Vertical
        position: photosViewId.visibleArea.yPosition
        pageSize: photosViewId.visibleArea.heightRatio
    }

    Component {
        id: delegateId

        Rectangle {
            id: rectId

            width:  photosViewId.thumbnailSize
            height: photosViewId.thumbnailSize

            border.width: 1

            Photo {
                id: imageId
                anchors.centerIn: parent
                height: parent.height
                width:  parent.width

                source: photoProperties.path
                photoHeight: photoProperties.height
                photoWidth: photoProperties.width

                thumbnails: thumbnailsManager.get()
            }
        }
    }
}
