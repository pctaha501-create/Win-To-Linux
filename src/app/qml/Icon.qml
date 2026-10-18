import QtQuick

Item {
    id: root
    property string name: "file"
    property color color: "#64748b"
    property real size: 22
    width: size; height: size; implicitWidth: size; implicitHeight: size
    Canvas {
        id: canvas; anchors.fill: parent; renderTarget: Canvas.Image
        onPaint: {
            var c=getContext("2d"); c.clearRect(0,0,width,height); c.save(); c.scale(width/24,height/24)
            c.strokeStyle=root.color; c.fillStyle=root.color; c.lineWidth=1.8; c.lineCap="round"; c.lineJoin="round"
            function line(p){c.beginPath();c.moveTo(p[0],p[1]);for(var i=2;i<p.length;i+=2)c.lineTo(p[i],p[i+1]);c.stroke()}
            function rect(x,y,w,h,r){c.beginPath();c.moveTo(x+r,y);c.lineTo(x+w-r,y);c.quadraticCurveTo(x+w,y,x+w,y+r);c.lineTo(x+w,y+h-r);c.quadraticCurveTo(x+w,y+h,x+w-r,y+h);c.lineTo(x+r,y+h);c.quadraticCurveTo(x,y+h,x,y+h-r);c.lineTo(x,y+r);c.quadraticCurveTo(x,y,x+r,y);c.closePath();c.stroke()}
            if(root.name==="folder"||root.name==="folder-new"){
                c.beginPath();c.moveTo(2.5,7);c.lineTo(9,7);c.lineTo(11,9);c.lineTo(21,9);c.quadraticCurveTo(22,9,21.7,10.5);c.lineTo(20.5,18);c.quadraticCurveTo(20.3,19,19,19);c.lineTo(4,19);c.quadraticCurveTo(2.5,19,2.7,17.5);c.closePath();c.stroke();
                if(root.name==="folder-new"){line([12,11.5,12,16.5]);line([9.5,14,14.5,14])}
            } else if(root.name==="file"){
                c.beginPath();c.moveTo(6,2.8);c.lineTo(14,2.8);c.lineTo(19,7.8);c.lineTo(19,21);c.lineTo(6,21);c.closePath();c.stroke();line([14,3,14,8,19,8]);line([9,13,16,13]);line([9,16.5,16,16.5])
            } else if(root.name==="start") {c.fillRect(3,3,8,8);c.fillRect(13,3,8,8);c.fillRect(3,13,8,8);c.fillRect(13,13,8,8)
            } else if(root.name==="search") {c.beginPath();c.arc(10.5,10.5,6.5,0,Math.PI*2);c.stroke();line([15.5,15.5,21,21])
            } else if(root.name==="system"||root.name==="drive") {rect(2.5,3,19,14,2);line([12,17,12,21]);line([8,21,16,21]);if(root.name==="system")line([7,7,7,13,10,11,13,13,17,8]);else{c.beginPath();c.arc(17,13,1.2,0,Math.PI*2);c.fill()}
            } else if(root.name==="package") {line([3,7,12,2.5,21,7,12,11.5,3,7]);line([3,7,3,17,12,21.5,21,17,21,7]);line([12,11.5,12,21.5]);line([7.5,5,16.5,9.5])
            } else if(root.name==="settings") {c.beginPath();c.arc(12,12,7.5,0,Math.PI*2);c.stroke();c.beginPath();c.arc(12,12,2.7,0,Math.PI*2);c.stroke();for(var a=0;a<8;a++){var t=a*Math.PI/4;line([12+8.5*Math.cos(t),12+8.5*Math.sin(t),12+10*Math.cos(t),12+10*Math.sin(t)])}
            } else if(root.name==="bell") {c.beginPath();c.moveTo(5,17);c.quadraticCurveTo(7,14,7,10);c.arc(12,10,5,Math.PI,0);c.quadraticCurveTo(17,14,19,17);c.closePath();c.stroke();line([4,17,20,17]);c.beginPath();c.arc(12,19,1.4,0,Math.PI);c.stroke()
            } else if(root.name==="home") {line([2.5,11,12,3,21.5,11]);line([5,9.5,5,21,19,21,19,9.5]);line([10,21,10,14,14,14,14,21])
            } else if(root.name==="refresh") {c.beginPath();c.arc(12,12,8,Math.PI*.2,Math.PI*1.65);c.stroke();line([16,3.5,20.5,4,20,8]);line([8,20.5,3.5,20,4,16])
            } else if(root.name==="back") {line([19,12,5,12]);line([11,6,5,12,11,18])
            } else if(root.name==="grid") {for(var y=0;y<2;y++)for(var x=0;x<2;x++)rect(3+x*10,3+y*10,8,8,1.5)
            } else if(root.name==="list") {line([4,6,20,6]);line([4,12,20,12]);line([4,18,20,18]);c.fillRect(2,5,1.5,1.5);c.fillRect(2,11,1.5,1.5);c.fillRect(2,17,1.5,1.5)
            } else if(root.name==="copy") {rect(8,7,12,14,2);rect(4,3,12,14,2)
            } else if(root.name==="move") {line([3,12,20,12]);line([14,6,20,12,14,18]);line([4,5,4,19])
            } else if(root.name==="rename") {line([4,20,9,19,20,8,16,4,5,15,4,20]);line([14.5,5.5,18.5,9.5])
            } else if(root.name==="delete") {line([4,7,20,7]);line([9,4,15,4]);line([6,7,7,21,17,21,18,7]);line([10,10,10,18]);line([14,10,14,18])
            } else if(root.name==="sort") {line([5,6,5,20]);line([2.5,8.5,5,6,7.5,8.5]);line([12,7,21,7]);line([12,12,19,12]);line([12,17,17,17])
            } else if(root.name==="download") {line([12,3,12,15]);line([7,10,12,15,17,10]);line([4,19,20,19])
            } else {c.beginPath();c.arc(12,12,8,0,Math.PI*2);c.stroke();c.beginPath();c.arc(12,12,1.5,0,Math.PI*2);c.fill()}
            c.restore()
        }
    }
    onNameChanged: canvas.requestPaint()
    onColorChanged: canvas.requestPaint()
    onSizeChanged: canvas.requestPaint()
    Component.onCompleted: canvas.requestPaint()
}
