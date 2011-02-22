var ws = new WSMessage({
  url : 'ws://localhost:8080',
  autoRecovery : true,
  listeners : {
      open : function(){console.info('onopen!!')},
      close : function(){console.info('onclose!!')},
      move : onMove,
      unregister : onUnKinectRegister,
      register : onKinectRegister,
  }
});

var pointer = {
  locX : 0,
  locY : 0,
  size : 10,
  capture : true,
  captureTime : 0,
  visible : true
}


function onMove(data) {
  pointer.locX = Math.floor(data.x/100*WIDTH);
  pointer.locY = Math.floor(data.y/100*HEIGHT);
  //pointer.size = data.z;
  pointer.size = 80;
}

function onHandClick() {
  pointer.capture = true;
  pointer.captureTime = 5;
}

function onUnKinectRegister() {
  pointer.visible = false;
}

function onKinectRegister() {
  pointer.visible = true;
}

var WIDTH;
var HEIGHT; 

function canvasStart() {
  HEIGHT  = $(window).height();
  WIDTH = $(window).width();
  console.info('start canvas');
  var canvas  = document.getElementById('canvas1');

  $(canvas).attr({
    width : WIDTH,
    height : HEIGHT 
  });
  if ((ctx = canvas.getContext && canvas.getContext('2d'))) {
    pointer = {
      visible : true,
      locX : 0,
      locY : 0,
      size : 10
    }
    setInterval(draw, 30);
    return;
  }
  alert('Error!! cannot get 2d context');
}

var ctx;
function draw() {

  ctx.globalCompositeOperation = "source-over";
  ctx.fillStyle = "rgba(8,8,12,0.1)";
  ctx.fillRect(0, 0, WIDTH, HEIGHT);
  //ctx.clearRect(0, 0, cInfo.WIDTH, cInfo.HEIGHT);
  ctx.globalCompositeOperation = "lighter";

  if (pointer.visible) {
    ctx.beginPath();
    if (pointer.captureTime > 0) {
      ctx.fillStyle = 'rgba(640,20,64, 0.8)';
      pointer.captureTime -= 1;
    } else {
      ctx.fillStyle = 'rgba(20,20,64, 0.8)';
    }
    ctx.arc(pointer.locX, pointer.locY, pointer.size, 0, Math.PI*2.0, true);
    ctx.fill();
  }

}

$(canvasStart);
