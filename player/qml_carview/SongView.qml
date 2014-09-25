import QtQuick 1.0

Rectangle { id: songView
  property alias artistName: artistName.text
  property alias albumName: albumName.text
  property alias songTitle: songTitle.text
  property alias songPositionText: songPositionText.text
  property alias slider: slider
  property alias fmtxButton: fmtxButton

  MetadataText { id: artistName
    text: "Artist name"
  }

  MetadataText { id: albumName
    text: "Album name"
    color: "#9c9a9c"
    y: playButton.y / 2
  }


  MetadataText { id: songTitle
    text: "Song title"
    y: playButton.y
  }

  Text { id: songPositionText
    color: "#ffffff"
    text: "00:00/--:--"

    font.pointSize: 18
    anchors.right: slider.right
    anchors.bottom: slider.top
  }

  Slider { id: slider
    width: 800-(800-665)*2
    height: 64
    minimum: 0
    maximum: 100
    value: 0
    y: nextButton.y + 32

    onValueChanged: {
      if (slider.down)
        sliderValueChanged(value)
    }
  }

  Button { id: fmtxButton
    property alias indicator: indicator

    anchors.right: parent.right
    anchors.rightMargin: 36
    width: 60
    height: 60
    y: slider.y
    onClicked: fmtxButtonClicked()

    Rectangle { id: indicator
      anchors.fill: parent
      smooth: true
      radius: 6
      gradient: Gradient {
        GradientStop { id: stop1; position: 0.0 }
        GradientStop { id: stop2; position: 1.0 }
      }
      states: [
        State  {
          name: "disabled"
          PropertyChanges  { target: stop1; color: "lightgray" }
          PropertyChanges  { target: stop2; color: "gray" }
        },
        State  {
          name: "enabled"
          PropertyChanges  { target: stop1; color: "lightgreen" }
          PropertyChanges  { target: stop2; color: "green" }
        }
      ]

      Text {
          anchors.fill: parent
          verticalAlignment: "AlignVCenter"
          horizontalAlignment: "AlignHCenter"
          color: "black"
          text: "FM"
          font.pixelSize: 32
      }
    }
  }
}
