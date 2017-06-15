var current;
var previous;

var sounds=[]
function setup() {
  var vanvas = createCanvas(windowWidth,windowHeight);
  // canvas.id("sketchCanvas");
  colorMode(HSB, 100);
  current = createVector(0,0);
  previous = createVector(0,0);
  var random = parseInt(getRandomArbitrary(0,4));
  sounds.push(loadSound("assets/sfx/"+random+"/a.mp3"));
  sounds.push(loadSound("assets/sfx/"+random+"/b.mp3"));
  sounds.push(loadSound("assets/sfx/"+random+"/c.mp3"));
  sounds.push(loadSound("assets/sfx/"+random+"/d.mp3"));
}

function draw() {
  current.x = mouseX;
  current.y = mouseY;
  background(0);
  
  blendMode(ADD);
  var force = p5.Vector.sub(current, previous);
  force.mult(0.05);
  var v = force;
  
  // console.log(force);
  var t = millis()*0.0009 * (v.x+1);

  var x = width*0.5;
  var y = height*0.5;
  var num = 5;
  var offset=10 * (v.x+1);
  // New particle's force is based on mouse movement

  for (var i = 0; i< num; i ++) {
    var tx = x+sin(t*i+i)*offset;
    var ty = y+cos(t*i+i)*offset;

    stroke((i/(num*1.0))*100, 100, 100);
    strokeWeight(10); 
    noFill();
    var radius     = 200;
    ellipse(tx, ty, radius, radius);
  }
  blendMode(NORMAL);
  if(current.x != previous.x && current.y != previous.y){
    if(sounds.length>0){
      var random = parseInt(getRandomArbitrary(0,4));
      if ( !sounds[random].isPlaying() && sounds[random].isLoaded()){
        sounds[random].play();
      }
    }
    
  }
  //   onMouseDraw( current.x*2, current.y*2 );


  previous.x = current.x;
  previous.y = current.y;
}

function getRandomArbitrary(min, max) {
  return Math.random() * (max - min) + min;
}
