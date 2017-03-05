import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.2


Window {
    id: mainWindow
    visible: true
    width: 300
    height:  600


    Column{
        spacing: 15
        width: parent.width
        height: parent.height
        Text{
            text: "Front"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            anchors.margins: 10
            objectName: "frontSlider"
            width: parent.width
            minimumValue: 0
            maximumValue: 255
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/front", value)
        }
        Text{
            text: "Back"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            width: parent.width
            objectName: "backSlider"
            minimumValue: 0
            maximumValue: 255
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/back", value)
        }
        Text{
            text: "Table"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            width: parent.width
            objectName: "tableSlider"
            minimumValue: 0
            maximumValue: 255
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/table", value)
        }
        Text{
            text: "Ambient Power"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            width: parent.width
            objectName: "ambientPowerSlider"
            minimumValue: 0
            maximumValue: 100
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/ambientPower", value)
        }
        Text{
            text: "Red"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            id: redValue
            objectName: "redSlider"
            width: parent.width
            minimumValue: 0
            maximumValue: 255
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/ambient",  redValue.value + "," + greenValue.value + "," + blueValue.value)
        }
        Text{
            text: "Green"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            id: greenValue
            objectName: "greenSlider"
            width: parent.width
            minimumValue: 0
            maximumValue: 255
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/ambient",  redValue.value + "," + greenValue.value + "," + blueValue.value)
        }
        Text{
            text: "Blue"
            font.family: "Ubuntu"
            font.weight: Font.Light
            font.pixelSize: 25
        }
        Slider{
            id: blueValue
            objectName: "blueSlider"
            width: parent.width
            minimumValue: 0
            maximumValue: 255
            updateValueWhileDragging: true
            value: 50
            stepSize: 1
            onValueChanged: app.notifyObservers("/lampa/ambient",  redValue.value + "," + greenValue.value + "," + blueValue.value)
        }
    }


}
