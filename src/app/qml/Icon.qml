import QtQuick

Item {
    id: root
    property string name: "file"
    property color color: "#64748b"
    property real size: 22
    width: size
    height: size
    implicitWidth: size
    implicitHeight: size

    Canvas {
        id: canvas
        anchors.fill: parent
        renderTarget: Canvas.Image
        onPaint: {
            var c = getContext("2d")
            c.clearRect(0, 0, width, height)
            c.save()
            c.scale(width / 24, height / 24)
            c.strokeStyle = root.color
            c.fillStyle = root.color
            c.lineWidth = 1.8
            c.lineCap = "round"
            c.lineJoin = "round"
            function line(points) { c.beginPath(); c.moveTo(points[0], points[1]); for (var i=2;i<points.length;i+=2) c.lineTo(points[i],points[i+1]); c.stroke() }
            if (root.name === "folder") {
                c.beginPath(); c.moveTo(3,7); c.lineTo(9.2,7); c.lineTo(11.2,9); c.lineTo(21,9); c.quadraticCurveTo(22,9,22,10.5); c.lineTo(20.5,18); c.quadraticCurveTo(20.3,19,19,19); c.lineTo(4,19); c.quadraticCurveTo(2.5,19,2.7,17.5); c.lineTo(3,7); c.stroke()
            } else if (root.name === "file") {
                c.beginPath(); c.moveTo(6,2.8); c.lineTo(14,2.8); c.lineTo(19,7.8); c.lineTo(19,21); c.lineTo(6,21); c.closePath(); c.stroke(); line([14,3,14,8,19,8]); line([9,13,16,13]); line([9,16.5,16,16.5])
            } else if (root.name === "start") {
                c.fillRect(3,3,8,8); c.fillRect(13,3,8,8); c.fillRect(3,13,8,8); c.fillRect(13,13,8,8)
            } else if (root.name === "search") {
                c.beginPath(); c.arc(10.5,10.5,6.5,0,Math.PI*2); c.stroke(); line([15.5,15.5,21,21])
            } else if (root.name === "system") {
                c.beginPath(); c.roundRect(2.5,3,19,14,2); c.stroke(); line([12,17,12,21]); line([8,21,16,21]); line([7,7,7,13,10,11,13,13,17,8])
            } else if (root.name === "package") {
                line([3,7,12,2.5,21,7,12,11.5,3,7]); line([3,7,3,17,12,21.5,21,17,21,7]); line([12,11.5,12,21.5]); line([7.5,5,16.5,9.5])
            } else if (root.name === "settings") {
                c.beginPath(); c.arc(12,12,7.5,0,Math.PI*2); c.stroke(); c.beginPath(); c.arc(12,12,2.7,0,Math.PI*2); c.stroke(); for(var a=0;a<8;a++){var t=a*Math.PI/4; line([12+8.5*Math.cos(t),12+8.5*Math.sin(t),12+10*Math.cos(t),12+10*Math.sin(t)])}
            } else if (root.name === "bell") {
                c.beginPath(); c.moveTo(5,17); c.quadraticCurveTo(7,14,7,10); c.arc(12,10,5,Math.PI,0); c.quadraticCurveTo(17,14,19,17); c.closePath(); c.stroke(); line([4,17,20,17]); c.beginPath(); c.arc(12,19,1.4,0,Math.PI); c.stroke()
            } else if (root.name === "home") {
                line([2.5,11,12,3,21.5,11]); line([5,9.5,5,21,19,21,19,9.5]); line([10,21,10,14,14,14,14,21])
            } else if (root.name === "refresh") {
                c.beginPath(); c.arc(12,12,8,Math.PI*.2,Math.PI*1.65); c.stroke(); line([16,3.5,20.5,4,20,8]); line([8,20.5,3.5,20,4,16])
            } else if (root.name === "back") { line([19,12,5,12]); line([11,6,5,12,11,18])
            } else if (root.name === "grid") { for(var y=0;y<2;y++) for(var x=0;x<2;x++){ c.beginPath(); c.roundRect(3+x*10,3+y*10,8,8,1.5); c.stroke() }
            } else {
                c.beginPath(); c.arc(12,12,8,0,Math.PI*2); c.stroke(); c.beginPath(); c.arc(12,12,1.5,0,Math.PI*2); c.fill()
            }
            c.restore()
        }
    }
    onNameChanged: canvas.requestPaint()
    onColorChanged: canvas.requestPaint()
    onSizeChanged: canvas.requestPaint()
    Component.onCompleted: canvas.requestPaint()
}
