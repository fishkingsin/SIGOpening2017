#include "ofApp.h"
int r = 100;
//--------------------------------------------------------------
void ofApp::setup(){
//    ofSetLogLevel(OF_LOG_VERBOSE);
    // setup a server with default options on port 9092
    // - pass in true after port to set up with SSL
    //bConnected = server.setup( 9093 );
    
    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9092;
    bConnected = server.setup( options );
    
    
    // this adds your app as a listener for the server
    server.addListener(this);
    
    ofBackground(0);
    ofSetFrameRate(60);
    canvasID = 0;
    
    // OF drawing
    srcImg.load("final_03.jpg");
    dstVideo.load("looping.mov");
    openingVid.load("trailer-withmusic.mp4");
    openingVid.setPaused(true);
    endLoop.load("loopingend.mov");
    endLoop.setPaused(true);
    dstVideo.play();
    dstVideo.setLoopState(OF_LOOP_NORMAL);
    brushImg.load("brush.png");
    srcImg.resize(ofGetWidth(), ofGetHeight());
//    dstImg.resize(ofGetWidth(), ofGetHeight());
    int width = srcImg.getWidth();
    int height = srcImg.getHeight();
    
    maskFbo.allocate(width,height);
    fbo.allocate(width,height);
    
    // There are 3 of ways of loading a shader:
    //
    //  1 - Using just the name of the shader and ledding ofShader look for .frag and .vert:
    //      Ex.: shader.load( "myShader");
    //
    //  2 - Giving the right file names for each one:
    //      Ex.: shader.load( "myShader.vert","myShader.frag");
    //
    //  3 - And the third one it�s passing the shader programa on a single string;
    //
    
    
#ifdef TARGET_OPENGLES
    shader.load("shaders_gles/alphamask.vert","shaders_gles/alphamask.frag");
#else
    if(ofIsGLProgrammableRenderer()){
        string vertex = "#version 150\n\
        \n\
        uniform mat4 projectionMatrix;\n\
        uniform mat4 modelViewMatrix;\n\
        uniform mat4 modelViewProjectionMatrix;\n\
        \n\
        \n\
        in vec4  position;\n\
        in vec2  texcoord;\n\
        \n\
        out vec2 texCoordVarying;\n\
        \n\
        void main()\n\
        {\n\
        texCoordVarying = texcoord;\
        gl_Position = modelViewProjectionMatrix * position;\n\
        }";
        string fragment = "#version 150\n\
        \n\
        uniform sampler2DRect tex0;\
        uniform sampler2DRect maskTex;\
        in vec2 texCoordVarying;\n\
        \
        out vec4 fragColor;\n\
        void main (void){\
        vec2 pos = texCoordVarying;\
        \
        vec3 src = texture(tex0, pos).rgb;\
        float mask = texture(maskTex, pos).r;\
        \
        fragColor = vec4( src , mask);\
        }";
        shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
        shader.bindDefaults();
        shader.linkProgram();
    }else{
        string shaderProgram = "#version 120\n \
        #extension GL_ARB_texture_rectangle : enable\n \
        \
        uniform sampler2DRect tex0;\
        uniform sampler2DRect maskTex;\
        \
        void main (void){\
        vec2 pos = gl_TexCoord[0].st;\
        \
        vec3 src = texture2DRect(tex0, pos).rgb;\
        float mask = texture2DRect(maskTex, pos).r;\
        \
        gl_FragColor = vec4( src , mask);\
        }";
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
        shader.linkProgram();
    }
#endif
    
    // otherwise it will bring some junk with it from the memory    
    maskFbo.begin();
    ofClear(0,0,0,255);
    maskFbo.end();
    
    fbo.begin();
    ofClear(0,0,0,255);
    fbo.end();
    Drawing * d = new Drawing();
    d->_id = canvasID++;
    d->color.set(ofRandom(255),ofRandom(255),ofRandom(255));
    
    drawings.insert( make_pair( d->_id, d ));
    animatableFloat.setup();
    animatableFloat.reset(1);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    openingVid.update();
    endLoop.update();
    animatableFloat.update(ofGetLastFrameTime());
    if ( toDelete.size() > 0 ){
        for ( auto & i : toDelete ){
            drawings.erase(i->_id);
            
            server.send("{\"erase\":\"" + ofToString( i->_id ) + "\"}" );
        }
        toDelete.clear();
    }
    maskFbo.begin();
    ofSetColor(255,255);
    vector<ofPoint>::iterator it;
    for(it=points.begin(); it != points.end() ; ++it){
        brushImg.draw((*it).x-r,(*it).y-r,r*2,r*2);
    }
    maskFbo.end();
    
    // HERE the shader-masking happends
    //
    fbo.begin();
    // Cleaning everthing with alpha mask on 0 in order to make it transparent for default
    ofClear(0, 0, 0, 0);
    
    shader.begin();
    shader.setUniformTexture("maskTex", maskFbo.getTexture(), 1 );
    
    dstVideo.draw(0,0);
    
    shader.end();
    fbo.end();
    
    ofNoFill();
    map<int, Drawing*>::iterator it2 = drawings.begin();
    for (it2; it2 != drawings.end(); ++it2){
        Drawing * d = it2->second;
        ofSetColor( d->color );
        ofBeginShape();
        for ( int i=0; i<d->points.size(); i++){
            ofVertex( d->points[i].x,d->points[i].y);
        }
        ofEndShape(false);
    }
    ofFill();
    for(it=points.begin(); it != points.end() ; ++it){
        brushImg.draw((*it).x-r,(*it).y-r,r*2,r*2);
        points.erase(it);
        --it;
    }

    dstVideo.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    //TODO fade out mask
    //play video
    ofSetColor(255);
    if(openingVid.getCurrentFrame() != openingVid.getTotalNumFrames() && openingVid.isPlaying()){
        openingVid.draw(0,0);
    }else if (openingVid.getCurrentFrame() != openingVid.getTotalNumFrames() && endLoop.isPaused()){
        endLoop.play();
        endLoop.setLoopState(OF_LOOP_NORMAL);
    }else{
        endLoop.draw(0,0);
    }
    
    ofPushStyle();
    ofSetColor(255,255*animatableFloat.val());
    
    
    srcImg.draw(0,0);
    
    fbo.draw(0,0);
    ofPopStyle();
    if(ofGetLogLevel() == OF_LOG_VERBOSE){
        if ( bConnected ){
            ofDrawBitmapString("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL"), 20, 20);
            
            ofSetColor(150);
            ofDrawBitmapString("Click anywhere to open up client example", 20, 40);
        } else {
            ofDrawBitmapString("WebSocket setup failed :(", 20,20);
        }
    }
    
    
}

//--------------------------------------------------------------
void ofApp::onConnect( ofxLibwebsockets::Event& args ){
    cout<<"on connected"<<endl;
}

//--------------------------------------------------------------
void ofApp::onOpen( ofxLibwebsockets::Event& args ){
    cout<<"new connection open"<<endl;
    cout<<args.conn.getClientIP()<< endl;
    Drawing * d = new Drawing();
    d->_id = canvasID++;
    d->color.set(ofColor::fromHsb(ofRandom(360),256,256));;
    d->conn = &( args.conn );
    
    drawings.insert( make_pair( d->_id, d ));
    
    // send "setup"
    args.conn.send( d->getJSONString("setup") );
    
    // send drawing so far
    map<int, Drawing*>::iterator it = drawings.begin();
    for (it; it != drawings.end(); ++it){
        Drawing * drawing = it->second;
        if ( d != drawing ){
            for ( int i=0; i<drawing->points.size(); i++){
                string x = ofToString(drawing->points[i].x);
                string y = ofToString(drawing->points[i].y);
                server.send( "{\"id\":"+ ofToString(drawing->_id) + ",\"point\":{\"x\":\""+ x+"\",\"y\":\""+y+"\"}," + drawing->getColorJSON() +"}");
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::onClose( ofxLibwebsockets::Event& args ){
    cout<<"on close"<<endl;
    // remove from color map
    for ( auto & it : drawings){
        Drawing * d = it.second;
        if ( *d->conn == args.conn ){
            toDelete.push_back(it.second);
            d->conn == NULL;
        }
    }

}

//--------------------------------------------------------------
void ofApp::onIdle( ofxLibwebsockets::Event& args ){
    cout<<"on idle"<<endl;
}

//--------------------------------------------------------------
void ofApp::onMessage( ofxLibwebsockets::Event& args ){
//    cout<<"got message "<<args.message<<endl;
    
    try{
        // trace out string messages or JSON messages!
        if ( !args.json.isNull() ){
            ofPoint point = ofPoint( args.json["point"]["x"].asFloat() * ofGetWidth() , args.json["point"]["y"].asFloat() * ofGetHeight());
            
            // for some reason these come across as strings via JSON.stringify!
            int r = ofToInt(args.json["color"]["r"].asString());
            int g = ofToInt(args.json["color"]["g"].asString());
            int b = ofToInt(args.json["color"]["b"].asString());
            ofColor color = ofColor( r, g, b );
            
            int _id = ofToInt(args.json["id"].asString());
            
            map<int, Drawing*>::const_iterator it = drawings.find(_id);
            Drawing * d = it->second;
            d->addPoint(point);
            points.push_back(point);
        } else {
        }
        // send all that drawing back to everybody except this one
        vector<ofxLibwebsockets::Connection *> connections = server.getConnections();
        for ( int i=0; i<connections.size(); i++){
            if ( (*connections[i]) != args.conn ){
                connections[i]->send( args.message );
            }
        }
    }
    catch(exception& e){
        ofLogError() << e.what();
    }
}

//--------------------------------------------------------------
void ofApp::onBroadcast( ofxLibwebsockets::Event& args ){
    cout<<"got broadcast "<<args.message<<endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if ( key == '`' ){
        string url = "http";
        if ( server.usingSSL() ){
            url += "s";
        }
        url += "://localhost:" + ofToString( server.getPort() );
        ofLaunchBrowser(url);
    }
    if(key == 'r'){
        maskFbo.begin();
        ofClear(0,0,0,255);
        maskFbo.end();
        animatableFloat.reset(1);

    }
    if(key == ' '){
        openingVid.play();
        openingVid.setLoopState(OF_LOOP_NONE);
        animatableFloat.animateTo(0);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    ofPoint p(x,y);
    
    points.push_back(p);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    ofPoint p(x,y);
    
    points.push_back(p);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}
