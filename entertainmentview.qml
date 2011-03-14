import Qt 4.7

Rectangle {
    id: rectangle1
    width: 800
    height: 480
    color: "#000000"
    signal quitButtonClicked;

    Image {
        id: quitButton
        x: 688
        y: 0
        source: "file:///etc/hildon/theme/images/wmBackIconPressed.png"
        scale: quitMouse.pressed ? 0.8 : 1.0
        smooth: quitMouse.pressed
        MouseArea {
            id: quitMouse
            x: 0
            y: 0
            width: 112
            height: 56
            anchors.rightMargin: 0
            anchors.bottomMargin: 0
            anchors.leftMargin: 0
            anchors.topMargin: 0
            anchors.fill: parent
            anchors.margins: -10
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
            color: "#000000"
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
                y: 230
                color: "#9c9a9c"
                text: "Album name"
            }

            Text {
                id: artsitName
                x: 32
                y: 202
                color: "#ffffff"
                text: "Artist name"
            }

            Item {
                id: slider; width: 317; height: 50

                // value is read/write.
                property real value: 1
                onValueChanged: updatePos();
                property real maximum: 1
                property real minimum: 1
                property int xMax: width - handle.width - 4
                x: 32
                y: 125
                onXMaxChanged: updatePos();
                onMinimumChanged: updatePos();

                function updatePos() {
                    if (maximum > minimum) {
                        var pos = 2 + (value - minimum) * slider.xMax / (maximum - minimum);
                        pos = Math.min(pos, width - handle.width - 2);
                        pos = Math.max(pos, 2);
                        handle.x = pos;
                    } else {
                        handle.x = 2;
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    border.color: "white"; border.width: 0; radius: 8
                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: "#66343434"
                        }

                        GradientStop {
                            position: 1
                            color: "#66000000"
                        }
                    }
                }

                Rectangle {
                    id: handle; smooth: true
                    y: 2; width: 51; height: slider.height-4; radius: 6
                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: "#d3d3d3"
                        }

                        GradientStop {
                            position: 1
                            color: "#808080"
                        }
                    }

                    MouseArea {
                        id: mouse
                        drag.maximumY: 100
                        drag.maximumX: 100
                        anchors.fill: parent; drag.target: parent
                        drag.axis: Drag.XAxis; drag.minimumX: 2;                        onPositionChanged: { value = (maximum - minimum) * (handle.x-2) / slider.xMax + minimum; }
                    }
                }
            }
        }

        property bool flipped: false
        x: 37

        front: metadataRectangle
        back: Image {
            source: albumArtImage.source; height: 360; width: 360;
        }


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

        PropertyAnimation on x { to: 392; duration: 1000; easing.type: Easing.OutBack }
        y: -700
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
                        color: "#000000"
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
        to: 30
        duration: 1000
        easing.type: Easing.InOutBack
    }

    Timer {
        id: animationTimer
        interval: 1000; running: true; repeat: true
        onTriggered: {
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
}
