import Qt 4.7

Rectangle {
  id: mainRectangle
  width: 800
  height: 480
  color: "#000"
  signal quitButtonClicked;
  signal nextButtonClicked;
  signal prevButtonClicked;
  signal playButtonClicked;
  signal sliderValueChanged(int value);
  signal playlistItemSelected(int index);

  //Component.onCompleted: { console.log("\n\nSTART"); }

  VisualItemModel {
    id: itemModel
    Item {
      width: mainRectangle.width - controls.width
      height: 480
      SongView { id: songView
        anchors.fill: parent
        color: '#000'

        states: State { name: "hidden"
          PropertyChanges {
            target: songView;
            opacity: 0
          }
          PropertyChanges {
            target: clock;
            opacity: 1
          }
          PropertyChanges {
            target: clockTimer;
            running: true
          }
          PropertyChanges {
            target: metaMouse;
            height: parent.height
          }
        }

        transitions: Transition {
          NumberAnimation {
            properties: "opacity";
            //easing.type: Easing.InBounce
            duration: 500
          }
        }

      }
      MetadataText { id: clock
        text: "CLOCK"
        width: 800-(800-665)*2
        height: 480
        anchors.right: undefined
        horizontalAlignment: "AlignRight"
        verticalAlignment: "AlignVCenter"
        opacity: 0
        font.pixelSize: 180
        Timer { id: clockTimer
          interval: 30000
          running: false
          repeat: true
          triggeredOnStart: true
          onTriggered: {
            clock.text = Qt.formatTime(new Date(), "hh:mm")
            //console.log("TICK", Qt.formatTime(new Date(), "hh:mm:ss"));
          }
        }
      }

      // Show/hide songView by touching it
      MouseArea { id: metaMouse
        anchors.left: songView.left
        anchors.top: songView.top
        anchors.right: songView.right
        height: parent.height - 128
        onClicked: songView.state = songView.state == "hidden" ? "" : "hidden"
      }
    }

    Playlist { id: playlist
      width: mainRectangle.width - controls.width
      height: 480
    }
  }

  ListView {
    id: view
    anchors {
      left: controls.right
      right: parent.right
      bottom: parent.bottom
      top: parent.top
    }
    model: itemModel
    highlightRangeMode: ListView.StrictlyEnforceRange
    orientation: ListView.Horizontal
    snapMode: ListView.SnapOneItem;
    //cacheBuffer: 2000000
    onCurrentIndexChanged: {
      songView.state = view.currentIndex == 1
          ? "hidden" : "";
    }
  }

  Rectangle { id: controls
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    width: 140
    color: '#000'

    Button { id: prevButton
      anchors.top: parent.top
      source: "file:///etc/hildon/theme/mediaplayer/Back.png"
      onClicked: prevButtonClicked()
    }

    Button { id: playButton
      anchors.verticalCenter: parent.verticalCenter
      source: "file:///etc/hildon/theme/mediaplayer/Play.png"
      onClicked: playButtonClicked()
    }

    Button { id: nextButton
      anchors.bottom: parent.bottom
      source: "file:///etc/hildon/theme/mediaplayer/Forward.png"
      onClicked: nextButtonClicked()
    }
  }

  Button { id: quitButton
    width: 112
    height: 56
    anchors.top: parent.top
    anchors.right: parent.right
    source: "file:///etc/hildon/theme/images/wmBackIconPressed.png"
    onClicked: {
      if (view.currentIndex == 1) {
        view.currentIndex = 0
      } else {
        quitButtonClicked()
      }
    }
  }

  function setSongTitle(text) {
    songView.songTitle = text
  }

  function setSongAlbum(text) {
    songView.albumName = text
  }

  function setSongArtist(text) {
    songView.artistName = text
  }

  function setAlbumArt(text) {
    //albumArtImage.source = text
  }

  function setPosition(text) {
    songView.songPositionText = text
  }

  function setSliderValue(number) {
    songView.slider.value = number
  }

  function setSliderMaximum(number) {
    songView.slider.maximum = number
  }

  function setPlayButtonIcon(text) {
    // XXX: text is empty sometimes
    if (text) playButton.source = text
  }

  function appendPlaylistItem(song, valueText, duration) {
      //console.log(song, valueText, duration, index)
      playlist.model.append ({ label: song + "<br>" +
                                      "<font color='#9c9a9c' size=1>" + valueText + "</font>"/*,
                               "name": song,
                               "duration": duration,
                               "albumArtist": valueText*/ })
  }

  function insertPlaylistItem(index, song, valueText, duration) {
      playlist.model.insert (index, { label: song + "<br>" +
                                             "<font color='#9c9a9c' size=1>" + valueText + "</font>"/*,
                                      "name": song,
                                      "duration": duration,
                                      "albumArtist": valueText*/ })
  }

  function setPlaylistItem(index, song, valueText, duration) {
      playlist.model.set (index, { label: song + "<br>" +
                                          "<font color='#9c9a9c' size=1>" + valueText + "</font>"/*,
                                   "name": song,
                                   "duration": duration,
                                   "albumArtist": valueText*/ })
  }

  function removePlaylistItem(index) {
      playlist.model.remove (index)
  }

  function clearPlaylist() {
      playlist.model.clear()
  }

  function onRowChanged(row) {
    playlist.setCurrentIndex(row)
  }
}
//1
