

import QtQuick 2.15
import photo_broom.qml 1.0

Flickable {
    id: flickableArea

    property alias source: photo.source

    contentWidth: area.width
    contentHeight: area.height

    function zoomToFit() {
        area.zoomToFit();
    }

    Item {
        id: area

        readonly property int freeZoomMode: 0
        readonly property int zoomToFitMode: 1
        readonly property int fullZoomMode: 2

        property int zoomType: freeZoomMode

        width: Math.max(flickableArea.width, photo.width * photo.scale)
        height: Math.max(flickableArea.height, photo.height * photo.scale)

        function zoomToFit() {
            photo.scale = flickableArea.width / photo.width;
        }

        function followMouse(oldScale, newScale, mouseX, mouseY, currentXoffset, currentYoffset)
        {
            var factor = newScale / oldScale;
            var offset = 0.0;

            if (area.width > flickableArea.width)
            {
                var oldMouseXAbs = mouseX;
                var newMouseXAbs = oldMouseXAbs * factor;
                var centerX = oldMouseXAbs - currentXoffset
                offset = newMouseXAbs - centerX;
                offset = Math.max(offset, 0);                               // eliminate negative numbers
                offset = Math.min(offset, area.width - flickableArea.width) // eliminate values above edge
                flickableArea.contentX = offset;
            }

            if (area.height > flickableArea.height)
            {
                var oldMouseYAbs = mouseY;
                var newMouseYAbs = oldMouseYAbs * factor;
                var centerY = oldMouseYAbs - currentYoffset
                offset = newMouseYAbs - centerY;
                offset = Math.max(offset, 0);                                 // eliminate negative numbers
                offset = Math.min(offset, area.height - flickableArea.height) // eliminate values above edge
                flickableArea.contentY = offset;
            }
        }

        Picture {
            id: photo

            anchors.centerIn: parent

            width: implicitWidth
            height: implicitHeight
        }

        MouseArea {
            anchors.fill: parent

            onWheel: {
                var pictureScale = photo.scale

                if (wheel.angleDelta.y > 0 && pictureScale < 8) {
                    pictureScale *= 1.4;
                } else if (wheel.angleDelta.y < 0 && pictureScale > 1/8) {
                    pictureScale /= 1.4;
                }

                var currentScale = photo.scale;
                var currentXoffset = flickableArea.contentX;
                var currentYoffset = flickableArea.contentY;

                photo.scale = pictureScale;
                area.zoomType = area.freeZoomMode

                area.followMouse(currentScale, photo.scale, wheel.x, wheel.y, currentXoffset, currentYoffset);
            }

            onDoubleClicked: {
                if (area.zoomType !== area.zoomToFitMode) {
                    flickableArea.contentX = 0;
                    flickableArea.contentY = 0;

                    area.zoomToFit();
                    area.zoomType = area.zoomToFitMode
                } else {
                    var currentScale = photo.scale;
                    var currentXoffset = flickableArea.contentX;
                    var currentYoffset = flickableArea.contentY;

                    photo.scale = 1.0
                    area.zoomType = area.fullZoomMode

                    area.followMouse(currentScale, photo.scale, mouse.x, mouse.y, currentXoffset, currentYoffset);
                }
            }
        }
    }
}