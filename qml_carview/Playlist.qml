import QtQuick 1.0


Item {
  property alias model: playlistModel

  ListModel { id: playlistModel }

  Component { id: highlight
    Rectangle {
      width: listView.currentItem.width+5; height: listView.currentItem.height
      color: "#1c1c1c"; radius: 0
      y: listView.currentItem.y
      Behavior on y {
        SpringAnimation {
          spring: 3
          damping: 0.2
        }
      }
    }
  }

  Component { id: listDelegate
    Item {
      id: delegateItem
      width: listView.width
      height: 110

      Text {
        text: label
        font.pixelSize: 48
        color: "white"
      }

      MouseArea {
        anchors.fill: delegateItem
        onClicked: {
          listView.currentIndex = index
          playlistItemSelected(index)
        }
      }
    }
  }

  ListView {
    id: listView
    anchors.fill: parent
    model: playlistModel
    delegate: listDelegate
    highlight: highlight
  }

  function setCurrentIndex(index) {
    listView.currentIndex = index;
  }
}
