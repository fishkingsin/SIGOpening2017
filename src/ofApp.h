#pragma once

#include "ofMain.h"

#include "ofxLibwebsockets.h"
#include "ofxAnimatableFloat.h"
class Drawing {
public:
    int             _id;
    ofColor         color;
    vector<ofPoint> points;
    ofxLibwebsockets::Connection * conn;
    
    Drawing(){
        conn = new ofxLibwebsockets::Connection(); _id = -1;
    }
    
    void addPoint( ofPoint p ){
        if ( points.size() + 1 > 500 ){
            points.erase( points.begin() );
        }
        points.push_back( p );
    }
    
    string getColorJSON( bool object=false){
        int r = color.r;
        int g = color.g;
        int b = color.b;
        
        stringstream jsonstring;
        if ( object ){
            jsonstring<<"{\"color\":{\"r\":\""<<r<<"\",\"g\":\""<<g<<"\",\"b\":\""<<b<<"\"}}";
        } else {
            jsonstring<<"\"color\":{\"r\":\""<<r<<"\",\"g\":\""<<g<<"\",\"b\":\""<<b<<"\"}";
        }
        return jsonstring.str();
    }
    
    string getJSONString( string wrapper){
        
        stringstream jsonstring;
        if ( wrapper != "" ){
            jsonstring << "{\"" + wrapper << "\":";
        }
        jsonstring<<"{" +getColorJSON()+",\"id\":"<< _id<<"}";
        
        if ( wrapper != "" ){
            jsonstring << "}";
        }
        return jsonstring.str();
    }
};

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofxLibwebsockets::Server server;
    
    bool bConnected;
    
    map<int, Drawing *> drawings;
    
    vector<Drawing *> toDelete;
    
    int             canvasID;
    
    vector<ofPoint> points;
    
    // websocket methods
    void onConnect( ofxLibwebsockets::Event& args );
    void onOpen( ofxLibwebsockets::Event& args );
    void onClose( ofxLibwebsockets::Event& args );
    void onIdle( ofxLibwebsockets::Event& args );
    void onMessage( ofxLibwebsockets::Event& args );
    void onBroadcast( ofxLibwebsockets::Event& args );
    
    ofImage     srcImg;
    ofVideoPlayer     dstVideo;
    ofImage     brushImg;
    
    ofFbo       maskFbo;
    ofFbo       fbo;
    
    ofShader    shader;
    
    ofxAnimatableFloat animatableFloat;
    ofVideoPlayer openingVid,endLoop;
};
