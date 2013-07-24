import QtQuick 1.0

Image {
  signal clicked
  width: 128
  height: 128
  scale: mouse.pressed ? 0.8 : 1.0
  smooth: true//mouse.pressed
  MouseArea {
    id: mouse
    anchors.fill: parent
    onClicked: parent.clicked()
  }
}
