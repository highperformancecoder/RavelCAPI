#pragma once

#include "ofMain.h"
#include "../../src/ravelCairo.h"
using namespace ravel;

class ofApp : public ofBaseApp
{
  RavelCairo<ofApp*> ravel;
public:
  
  void setup();
  void update() {}
  void draw();
  
  void keyPressed(int key) {}
  void keyReleased(int key) {}
  void mouseMoved(int x, int y ) {if (ravel.onMouseOver(x,y)) draw();}
  void mouseDragged(int x, int y, int button) {
    if (ravel.onMouseMotion(x,y)) draw();
  }
  void mousePressed(int x, int y, int button) {
    if (button==0) ravel.onMouseDown(x,y);
  }
  void mouseReleased(int x, int y, int button) {
    if (button==0) ravel.onMouseUp(x,y);}
  void mouseEntered(int x, int y) {}
  void mouseExited(int x, int y) {}
  void windowResized(int w, int h) {}
  void dragEvent(ofDragInfo dragInfo) {}
  void gotMessage(ofMessage msg) {}		
};
	
