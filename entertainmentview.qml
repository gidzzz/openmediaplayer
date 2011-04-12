import Qt 4.7

Rectangle {
    id: mainRectangle
    width: 800
    height: 480
    color: "#333333"
    signal quitButtonClicked;
    signal nextButtonClicked;
    signal prevButtonClicked;
    signal playButtonClicked;
    signal sliderValueChanged(int value);
    signal playlistItemSelected(int index);

    Image {
        id: quitButton
        x: 688
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0
        source: "file:///etc/hildon/theme/images/wmBackIconPressed.png"
        scale: quitMouse.pressed ? 0.8 : 1.0
        smooth: quitMouse.pressed
        MouseArea {
            id: quitMouse
            x: 0
            y: 0
            width: 112
            height: 56
            anchors.fill: parent
            onClicked: {
                quitButtonClicked()
                Qt.quit
            }
        }
    }

    Flipable {
        id: flipable
        width: 376
        height: 312

        Rectangle {
            id: metadataRectangle
            x: 0
            y: 0
            color: "#333333"
            anchors.rightMargin: 0
            anchors.bottomMargin: 0
            anchors.leftMargin: 0
            anchors.topMargin: 0
            anchors.fill: parent
            Text {
                id: songTitle
                x: 32
                y: 76
                color: "#ffffff"
                text: "Song title"
            }

            Text {
                id: albumName
                x: 32
                y: 240
                color: "#9c9a9c"
                text: "Album name"
            }

            Text {
                id: artsitName
                x: 32
                y: 213
                color: "#ffffff"
                text: "Artist name"
            }

            Text {
                id: songPositionText
                x: 271
                y: 180
                color: "#ffffff"
                text: "00:00/00:00"
                font.pointSize: 13
                anchors.right: slider.right
            }

            Slider {
                id: slider; width: 318; height: 50

                x: 32
                y: 125
                minimum: 0
                maximum: 100
                value: 0

                onValueChanged: {
                    if (slider.down)
                        sliderValueChanged(value)
                }
            }
        }

        // The model:
        ListModel {
            id: playlistModel
        }

        Component {
            id: highlight
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

        Component {
            id: listDelegate

            Item {
                id: delegateItem
                width: listView.width; height: 55
                clip: true

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 10

                    Column {
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            text: name
                            font.pixelSize: 18
                            color: "white"
                        }
                        Row {
                            spacing: 5
                            Text { text: albumArtist; font.pixelSize: 13; color: "#9c9a9c" }
                        }
                    }
                }

                Row {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    spacing: 10

                    Text {
                        id: durationText
                        anchors.verticalCenter: parent.verticalCenter
                        text: duration
                        font.pixelSize: 15
                        color: "white"
                    }
                }

                MouseArea {
                    anchors.fill: delegateItem
                    onClicked: {
                        listView.currentIndex = index
                        playlistItemSelected(index)
                    }
                }

                // Animate adding and removing of items:

                ListView.onAdd: SequentialAnimation {
                    PropertyAction { target: delegateItem; property: "height"; value: 0 }
                    NumberAnimation { target: delegateItem; property: "height"; to: 55; duration: 250; easing.type: Easing.InOutQuad }
                }

                ListView.onRemove: SequentialAnimation {
                    PropertyAction { target: delegateItem; property: "ListView.delayRemove"; value: true }
                    NumberAnimation { target: delegateItem; property: "height"; to: 0; duration: 250; easing.type: Easing.InOutQuad }

                    // Make sure delayRemove is set back to false so that the item can be destroyed
                    PropertyAction { target: delegateItem; property: "ListView.delayRemove"; value: false }
                }
            }
        }


        ListView {
            id: listView
            anchors.fill: flipable
            model: playlistModel
            delegate: listDelegate
            highlight: highlight
            highlightFollowsCurrentItem: false
            focus: true
            maximumFlickVelocity: 1000
        }

        property bool flipped: false
        x: 37

        front: metadataRectangle
        back: listView

        transform: Rotation {
            id: rotation
            origin.x: flipable.width/2
            origin.y: flipable.height/2
            axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
            angle: 0    // the default angle
        }

        states: State {
            name: "back"
            PropertyChanges { target: rotation; angle: 180 }
            when: flipable.flipped
        }

        transitions: Transition {
            NumberAnimation { target: rotation; property: "angle"; duration: 500 }
        }

        PropertyAnimation on x { to: 400; duration: 1000; easing.type: Easing.OutBack }
        y: 0
        PropertyAnimation on y { to: 84; duration: 1000; easing.type: Easing.OutBack }
    }

    Rectangle {
        id: albumRectangle
        x: 0
        y: 700
        Image {
            id: albumArtImage
            x: 0
            y: 0
            width: 360
            height: 360
            source: "/usr/share/icons/hicolor/295x295/hildon/mediaplayer_default_album.png"
            smooth: true
        }

        // mirror image - album art and a gradient filled rectangle for darkening
        Item {
            id: reflectedImage
            x: 35
            y: 0
            width: albumArtImage.width; height: albumArtImage.height
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: albumArtImage.horizontalCenter

            // transform this item (the image and rectangle) to create the
            // mirror image using the values from the Path
            transform : [
                Rotation {
                    angle: -180; origin.y: albumArtImage.height
                    axis.x: 1; axis.y: 0; axis.z: 0
                },
                Scale {

                    origin.x: albumArtImage.width/2; origin.y: albumArtImage.height/2
                }
            ]

            // mirror image
            Image {
                width: albumArtImage.width; height: albumArtImage.height
                source: albumArtImage.source
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: 0.3
            }

            // mirror image dimming gradient filled rectangle
            Rectangle {
                x: 0
                y: 0
                width: albumArtImage.width; height: albumArtImage.height
                anchors.horizontalCenterOffset: 0
                gradient: Gradient {
                    GradientStop {
                        position: 0.99
                        color: "#66000000"
                    }

                    GradientStop {
                        position: 0.01
                        color: "#333333"
                    }
                }
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
        PropertyAnimation on x { to: 20; duration: 1000; easing.type: Easing.OutBack }
        PropertyAnimation on y { to: 50; duration: 1000; easing.type: Easing.OutBack }

        MouseArea {
            anchors.fill: albumArtImage
            onClicked: flipable.flipped = !flipable.flipped
        }
    }

    PropertyAnimation {
        id: animateAlbumArt
        target: albumRectangle;
        property: "x";
        to: 400 ;
        duration: 1000;
        easing.type: Easing.InOutBack
    }

    PropertyAnimation {
        id: animateMetadata
        target: flipable
        property: "x"
        to: 10
        duration: 1000
        easing.type: Easing.InOutBack
    }

    Timer {
        id: animationTimer
        interval: 5000; running: true; repeat: true
        onTriggered: {
            if (!listView.moving && !slider.down) {
                animateAlbumArt.running = true
                animateMetadata.running = true

                if (albumRectangle.x == 400) {
                    animateAlbumArt.to = 20
                    animateMetadata.to = 400
                }
                else if(albumRectangle.x == 20) {
                    animateAlbumArt.to = 400
                    animateMetadata.to = 10
                }
            }
        }
    }

    Rectangle {
        id: controls
        x: 267
        y: 470
        width: 267
        height: 64
        color: "#00000000"
        radius: 0
        anchors.horizontalCenterOffset: 1
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 1

        PropertyAnimation on y { to: 416; duration: 1000; easing.type: Easing.OutBack }

        PropertyAnimation {
            id: hideControls
            target: controls
            property: "y"
            to: mainRectangle.height+70
            duration: 500
            easing.type: Easing.InOutBack
        }

        PropertyAnimation {
            id: showControls
            target: controls
            property: "y"
            to: mainRectangle.height-64
            duration: 500
            easing.type: Easing.InOutBack
        }

        Image {
            id: prevButton
            width: 64
            height: 64
            anchors.left: parent.left
            anchors.leftMargin: 0
            opacity: 1
            source: "file:///etc/hildon/theme/mediaplayer/Back.png"

            MouseArea {
                id: prevButtonArea
                opacity: 1
                anchors.fill: parent
                onClicked: prevButtonClicked()
            }
        }

        Image {
            id: playButton
            x: 93
            y: 0
            width: 64
            height: 64
            anchors.horizontalCenter: parent.horizontalCenter
            source: "file:///etc/hildon/theme/mediaplayer/Play.png"

            MouseArea {
                id: playButtonArea
                anchors.fill: parent
                onClicked: playButtonClicked()
            }
        }

        Image {
            id: nextButton
            x: 186
            y: 0
            width: 64
            height: 64
            anchors.right: parent.right
            anchors.rightMargin: 0
            source: "file:///etc/hildon/theme/mediaplayer/Forward.png"

            MouseArea {
                id: nextButtonArea
                x: 0
                y: 0
                anchors.rightMargin: 0
                anchors.bottomMargin: 0
                anchors.leftMargin: 0
                anchors.topMargin: 0
                anchors.fill: parent
                onClicked: nextButtonClicked()
            }
        }
    }

    MouseArea {
        id: controlsToggleArea
        x: 0
        y: 397
        width: 267
        height: 83
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.top: parent.top
        anchors.topMargin: 397

        onClicked: {
            if (controls.y == mainRectangle.height+70)
                showControls.running = true
            else
                hideControls.running = true
        }
    }

    MouseArea {
        id: controlsToggleArea1
        x: 534
        y: 397
        width: 267
        height: 83
        anchors.top: parent.top
        anchors.topMargin: 397
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        onClicked: {
            if (controls.y == mainRectangle.height+70)
                showControls.running = true
            else
                hideControls.running = true
        }
    }


    function setSongTitle(text) {
        songTitle.text = text
    }

    function setSongAlbum(text) {
        albumName.text = text
    }

    function setSongArtist(text) {
        artsitName.text = text
    }

    function setAlbumArt(text) {
        albumArtImage.source = text
    }

    function setPosition(text) {
        songPositionText.text = text
    }

    function setSliderValue(number) {
        slider.value = number
    }

    function setSliderMaximum(number) {
        slider.maximum = number
    }

    function setPlayButtonIcon(text) {
        playButton.source = text
    }

    function addPlaylistItem(song, valueText, duration, index) {
        playlistModel.append ({
                              "name": song,
                              "duration": duration,
                              "albumArtist": valueText,
                              "index": index
                          })
    }

    function onRowChanged(row) {
        console.log (listView.count)
        console.log (row)
    }
}
