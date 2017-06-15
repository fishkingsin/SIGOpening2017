var socket;

var messageDiv;
var statusDiv;

// drawing vars
var canvas;
var ctx = null;
var color    = {};
var id     = -1;
var sketches = {};

$(window).load(function() {
  setupSocket();
  
  // document.getElementById("brow").textContent = " " + BrowserDetect.browser + " "
  //  + BrowserDetect.version +" " + BrowserDetect.OS +" ";

  // messageDiv   = document.getElementById("messages");
  // statusDiv  = document.getElementById("status");

  canvas    = document.getElementById("defaultCanvas0");
  if (canvas.getContext) {
    canvas.onmousedown  = onMouseDown;
    canvas.onmouseup  = onMouseUp;
    canvas.onmousemove  = onMouseMoved;

    canvas.ontouchstart   = onMouseDown;
    canvas.ontouchmove  = onMouseMoved;
    canvas.ontouchend   = onMouseUp;
    // ctx     = canvas.getContext('2d');
    // canvas.width  = window.innerWidth;
    // canvas.height = window.innerHeight;
    // window.addEventListener("resize", onresize);

    $('html, body').on('touchstart touchmove', function(e){ 
         //prevent native touch activity like scrolling
         e.preventDefault(); 
     });
  } else {
    alert("Sorry, your browser doesn't support canvas!");
  }
});


// send value from text input
// function sendMessageForm(){
//  socket.send(message.value);
//  message.value = "";
// }
function onresize(){
  canvas    = document.getElementById("defaultCanvas0");
  
  // canvas.width  = window.innerWidth;
  // canvas.height = window.innerHeight;
  // wOffset = Math.floor(canvas.width/360);
  // hOffset = Math.floor(canvas.height/144);
}

var bMouseDown = false;
function onMouseDown( e ){
  bMouseDown = true;
  if(e.pageX){
    var mouseX = e.pageX - this.offsetLeft;
    var mouseY = e.pageY - this.offsetTop;
    onMouseDraw( mouseX, mouseY );
  }else{
  	 e.preventDefault(); 
    var mouseX = e.originalEvent.touches[0].pageX - this.offsetLeft;
    var mouseY = e.originalEvent.touches[0].pageY - this.offsetTop;
    onMouseDraw( mouseX, mouseY );
  }
  
}

function onMouseMoved( e ){
  if ( bMouseDown ){
  	if(e.pageX){
  		var mouseX = e.pageX - this.offsetLeft;
  		var mouseY = e.pageY - this.offsetTop;
  		onMouseDraw( mouseX, mouseY );
  	}else{
  		e.stopPropagation();
  		var mouseX = e.originalEvent.touches[0].pageX - this.offsetLeft;
  		var mouseY = e.originalEvent.touches[0].pageY - this.offsetTop;
  		onMouseDraw( mouseX, mouseY );
  	}
  }
}

function onMouseUp( e ){
  bMouseDown = false;
}

// catch mouse events on canvas
function onMouseDraw( x, y ){
  var point = {point:{x:x/canvas.width,y:y/canvas.height},id:id,color:color};
  if ( socket.readyState == 1 ){
    sketches[id].points.push( point.point );
    if ( sketches[id].points.length > 500 ){
      sketches[id].points.shift();
    }
    socket.send(JSON.stringify(point));
    renderCanvas();
  }
}


function renderCanvas(){
  if ( ctx == null ) return;
  
  canvas.width = canvas.width;
  ctx.moveTo(0,0);
  for ( var _id in sketches ){
    var c = sketches[_id].color;
    var pts = sketches[_id].points;
    ctx.strokeStyle = 'rgb('+c.r+','+c.g+','+c.b+')';
    ctx.beginPath();
    if ( pts.length > 0 ){      
      ctx.moveTo(pts[0].x,pts[0].y);
      for ( var i=1, len = pts.length; i<len; i++){
        ctx.lineTo( pts[i].x, pts[i].y )
      }
      ctx.stroke();
    }
  }
}
// https://gist.github.com/sabman/1018594/b58cbe80342a7a9f302987e6585be27be270b6be
function componentToHex(c) {
  var hex = c.toString(16);
  var hexStr = hex.length == 1 ? "0" + hex : hex;
  return hexStr;
}

function rgbToHex(r, g, b) {
  return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}
// catch incoming messages + render them to the canvas

// setup web socket
function setupSocket(){

  // setup websocket
  // get_appropriate_ws_url is a nifty function by the libwebsockets people
  // it decides what the websocket url is based on the broswer url
  // e.g. https://mygreathost:9099 = wss://mygreathost:9099
  socket = new WebSocket(get_appropriate_ws_url());
  
  // open
  try {
    socket.onopen = function() {
      console.log("open");
      // statusDiv.style.backgroundColor = "#40ff40";
      // statusDiv.textContent = " websocket connection opened ";

    } 

    // received message
    socket.onmessage =function got_packet(msg) {
      var message = JSON.parse(msg.data);

      if ( message.setup ){
        // set up our drawing!
        color   = message.setup.color;
        id    = message.setup.id;
        canvas.style.backgroundColor = rgbToHex( parseInt(color.r),  parseInt(color.g),  parseInt(color.b));
        sketches[id] = {color:color, points:[]};
      } else if ( message.point ){
        var c = message.color;
        var _id = message.id;

        // if we don't know this one, add it to our list
        if ( !sketches[_id] ){
          sketches[_id] = {color:c, points:[]};
        }
        sketches[_id].points.push( message.point );
        if ( sketches[_id].points.length > 500 ){
          sketches[_id].points.shift();
        }
      } else if ( message.erase ){
        var _id = message.erase;
        if ( sketches[_id] ){
          delete sketches[_id];
        }
      }
      renderCanvas();
    }


    socket.onclose = function(){
      // statusDiv.style.backgroundColor = "#ff4040"; 
      // statusDiv.textContent = " websocket connection CLOSED ";
    }
  } catch(exception) {
    // alert('<p>Error' + exception);  
  }
}
