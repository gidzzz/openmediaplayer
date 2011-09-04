import QtQuick 1.0

Rectangle { id: songView
  property alias artistName: artistName.text
  property alias albumName: albumName.text
  property alias songTitle: songTitle.text
  property alias songPositionText: songPositionText.text
  property alias slider: slider

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
    text: "00:00/00:00"

    font.pointSize: 13
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

}
