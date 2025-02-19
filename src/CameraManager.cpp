//
//  CameraManager.cpp
//  Tribeca_Kinect
//
//  Created by Nick Hardeman on 4/21/12.
//

#include "CameraManager.h"

//--------------------------------------------------------------
void CameraManager::setup() {
    _longitude  = 2.f;
    _latitude   = 1.f;
    setCenter(0, 0, 0);
    
    cam.disableMouseInput();
    
    _numBeatHits    = 0;
    
    _nextLongitude  = ofRandom(-10, 10);
    _nextLatitude   = 1.f;
    
    _distance = _maxDistance;
    
    ofSetCoordHandedness(OF_RIGHT_HANDED);
    
    // Setup post-processing chain
    post.init(ofGetWidth(), ofGetHeight());
    post.createPass<FxaaPass>()->setEnabled(false);
    post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<DofPass>()->setEnabled(false);
    post.createPass<KaleidoscopePass>()->setEnabled(false);
    post.createPass<NoiseWarpPass>()->setEnabled(false);
    post.createPass<PixelatePass>()->setEnabled(false);
    post.createPass<EdgePass>()->setEnabled(false);
    post.createPass<VerticalTiltShifPass>()->setEnabled(false);
    post.createPass<GodRaysPass>()->setEnabled(false);
    post.createPass<ToonPass>()->setEnabled(false);
    post.createPass<SSAOPass>()->setEnabled(false ) ;
    
    bEnablePostFX = false ;
    
}

//--------------------------------------------------------------
void CameraManager::loadSettings() {
    gui->loadSettings("GUI/camera_.xml");
}

//--------------------------------------------------------------
void CameraManager::saveSettings() {
    gui->saveSettings("GUI/camera_.xml");
}

void CameraManager::begin()
{
    if ( !bEnablePostFX )
        cam.begin() ;
    else
    {
        // copy enable part of gl state
        glPushAttrib(GL_ENABLE_BIT);
     
        // setup gl state
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
       
        post.begin(cam);
        //ofClear( 0 , 0 , 0 , 1 ) ;
    }
}

void CameraManager::end()
{
    if ( !bEnablePostFX )
        cam.end() ;
    else
    {
        post.end() ;
    
        // restore gl state
        //glDisable(GL_CULL_FACE);
        //glDisable(GL_DEPTH_TEST);
        glPopAttrib() ;
       // glPopAttrib(GL_ENABLE_BIT);
    }
}

//--------------------------------------------------------------
void CameraManager::update() {
    
    if(!_bAuto) {

    } else {
        if(_longitude >= 179.1f) _longitude = -179.f;
        if(_longitude <= -179.1f) _longitude = 179.f;
        
        if(_latitude < -74.f) { 
            _latitude = -74.f; 
        }
        if(_latitude > 74.1f) { 
            _latitude = 74.f; 
        }
    }
    
    ofVec3f c = _center ;
    c.x = sin ( ofGetElapsedTimef() ) * camMovementFactor ;
    c.y = cos ( ofGetElapsedTimef() * 0.75 ) * camMovementFactor ;
    
    c.z =  cameraTargetZ ; //zOffset ;
    ofVec3f tar( c );
    _target.xeno( tar, _targetSpring);
    
    //distance		= maxDistance;
    
    _target.addDamping( _target.DAMPING );
    _target.vel	+= _target.acc;
    _target += _target.vel;
    
    //if(_bAuto) {
        ofVec3f orbitVec = getOrbitVec3f( _longitude, _latitude, _distance, _target);
        
        //_camParticle.xeno(currHeading, particleTargetXeno);
		_camParticle.steer(orbitVec, true, _steer, _distance*.6);
        
        _camParticle.addDamping( _camParticle.DAMPING );
		_camParticle.vel    += _camParticle.acc;
		_camParticle	+= _camParticle.vel;
    //} else {
        
    //}
    
    
    
    
    _camParticle.acc.set(0,0,0);
	//target.acc.set(0, 0, 0);
	
	cam.setPosition(_camParticle);
	
	cam.lookAt( _target, ofVec3f(0,1,0) );
}

//--------------------------------------------------------------
void CameraManager::draw() {
    ofSetColor(255, 0, 0);
    ofSphere(_target, 20);
    
    ofSetColor(0, 0, 255);
    ofSphere(_center, 20);
    
    ofSetColor(255, 255, 0);
    //ofSphere( _camParticle.loc, 20);
}

//--------------------------------------------------------------
bool CameraManager::debug() {
    return _bDebug;
}

//--------------------------------------------------------------
bool CameraManager::isAuto() {
    return _bAuto;
}

void CameraManager::updateFft( float _low , float _mid , float _high )
{
    low = _low ;
    mid = _mid ;
    high = _high ;
    
    _numBeatHits++;
    if(_numBeatHits >= _maxBeatHits) {
        if(_bAuto) {
            if(ofRandom(0,1) > .5) {
                ofxUISlider* slider = ((ofxUISlider*)gui->getWidget("CAM_LONGITUDE"));
                _longitude = ofRandom( _minLongitude , _maxLongitude );
            }
            if(ofRandom(0,1) > .3) {
                ofxUISlider* slider = ((ofxUISlider*)gui->getWidget("CAM_LATITUDE"));
                _latitude = ofRandom( _minLatitude , _maxLatitude );
            }
            ofxUISlider* slider = ((ofxUISlider*)gui->getWidget("CAM_DISTANCE"));
            _distance = ofRandom(_minDistance, _maxDistance);
            slider->setValue(_distance);
        }
        
        _numBeatHits = 0;
    }

}

//--------------------------------------------------------------
float CameraManager::getDistance() {
    return _distance;
}

//--------------------------------------------------------------
float CameraManager::getDistanceToTarget() {
    return (_camParticle - _target).length();
}

//--------------------------------------------------------------
void CameraManager::setMinDistance( float a_minDist ) {
    _minDistance    = a_minDist;
}

//--------------------------------------------------------------
float CameraManager::getMinDistance() {
    return _minDistance;
}

//--------------------------------------------------------------
void CameraManager::setMaxDistance( float a_maxDist ) {
    _maxDistance    = a_maxDist;
}

//--------------------------------------------------------------
float CameraManager::getMaxDistance() {
    return _maxDistance;
}

// we need a custom orbit function, instead of ofNode //
//--------------------------------------------------------------
ofVec3f CameraManager::getOrbitVec3f(float a_longitude, float a_latitude, float a_radius, const ofVec3f& a_centerPoint) {
	ofVec3f p(0, 0, a_radius);
	p.rotate(ofClamp(a_latitude, -89, 89), ofVec3f(1, 0, 0));
	p.rotate(a_longitude, ofVec3f(0, 1, 0));
	p += a_centerPoint;
	return p;
}

//--------------------------------------------------------------
void CameraManager::setCenter(float a_x, float a_y, float a_z) {
    _center.set(a_x, a_y, a_z);
}

//--------------------------------------------------------------
void CameraManager::setCenter(ofVec3f a_center) {
    _center.set(a_center);
}

//--------------------------------------------------------------
void CameraManager::guiEvent(ofxUIEventArgs &e) {
    string ename = e.widget->getName();
    //cout << " CAMEAR MANAGER GUI EVENT " << ename << endl ;
    

    if ( ename == "LOAD SETTINGS" && e.getButton()->getValue() == true )
        loadSettings() ;
    
    if ( ename == "SAVE SETTINGS" && e.getButton()->getValue() == true )
        saveSettings() ;

    if(ename == "Debug") {
        _bDebug = ((ofxUIToggle*)e.widget)->getValue();
    } else if (ename == "CAMERA_AUTO") {
        _bAuto = ((ofxUIToggle*)e.widget)->getValue();
    } else if (ename == "CAM_MIN_DISTANCE" ) {
        _minDistance = ((ofxUISlider*)e.widget)->getScaledValue();
    } else if (ename == "CAM_MIN_DISTANCE" ) {
        _maxDistance = ((ofxUISlider*)e.widget)->getScaledValue();
    } else if (ename == "TARGET_SPRING" ) {
        _targetSpring = ((ofxUISlider*)e.widget)->getScaledValue();
    } else if (ename == "STEER" ) {
        _steer = ((ofxUISlider*)e.widget)->getScaledValue();
    } else if (ename == "PARTICLE_DAMPING" ) {
        _camParticle.DAMPING = ((ofxUISlider*)e.widget)->getScaledValue();
    } else if (ename == "MAX_BEAT_HITS" ) {
        _maxBeatHits = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    
    if ( ename == "CAM_LONGITUDE" )
    {
        _minLongitude = ((ofxUIRangeSlider*)e.widget)->getScaledValueLow() ;
        _maxLongitude = ((ofxUIRangeSlider*)e.widget)->getScaledValueHigh() ;
    }
    
    if ( ename == "CAM_LATITUDE" )
    {
        _minLatitude = ((ofxUIRangeSlider*)e.widget)->getScaledValueLow() ;
        _maxLatitude = ((ofxUIRangeSlider*)e.widget)->getScaledValueHigh() ;
    }
    
    else if (ename == "Z OFFSET" ) {
        zOffset = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    else if (ename == "CAM MOVEMENT" ) {
        camMovementFactor = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    else if (ename == "CAMERA TARGET Z" ) {
        cameraTargetZ = ((ofxUISlider*)e.widget)->getScaledValue();
    }
    
    
     else if ( ename == "FXAA PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[0]->enable() ;
         else
             post.getPasses()[0]->disable() ;
     }
     else if ( ename == "BLOOM PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[1]->enable() ;
         else
             post.getPasses()[1]->disable() ;

     }
     else if ( ename == "DOF PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[2]->enable() ;
         else
             post.getPasses()[2]->disable() ;

     }
     else if ( ename == "KALEIDOSCOPE PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[3]->enable() ;
         else
             post.getPasses()[3]->disable() ;

     }
     else if ( ename == "NOISE WARP PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[4]->enable() ;
         else
             post.getPasses()[4]->disable() ;

     }
     else if ( ename == "PIXELATE PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[5]->enable() ;
         else
             post.getPasses()[5]->disable() ;

     }
     else if ( ename == "EDGE PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[6]->enable() ;
         else
             post.getPasses()[6]->disable() ;

     }
     else if ( ename == "VERTICAL TILT SHIFT" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[7]->enable() ;
         else
             post.getPasses()[7]->disable() ;

     }
     else if ( ename == "GOD RAYS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[8]->enable() ;
         else
             post.getPasses()[8]->disable() ;
     }
     else if ( ename == "TOON PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[9]->enable() ;
         else
             post.getPasses()[9]->disable() ;
     }
     else if ( ename == "SSAO PASS" )
     {
         if ( e.getToggle()->getValue() )
             post.getPasses()[10]->enable() ;
         else
             post.getPasses()[10]->disable() ;
     }

}

//--------------------------------------------------------------
void CameraManager::setupGui( float a_x, float a_y ) {
    gui = new ofxUIScrollableCanvas( a_x, a_y, 320, ofGetHeight() + 300 - a_y );
    
    float GUI_WIDGET_WIDTH = 300;
    float GUI_SLIDER_HEIGHT = 16;

    gui->addWidgetDown(new ofxUILabel("Camera Settings", OFX_UI_FONT_LARGE));
    gui->addSpacer( GUI_WIDGET_WIDTH, 1);
    //gui->addSpacer(GUI_WIDGET_WIDTH, 2);
    gui->addButton( "SAVE SETTINGS" , false ) ;
    gui->addButton( "LOAD SETTINGS" , false ) ; 
   // gui->addWidgetDown( new ofxUIToggle("Debug", false, 16, 16 ) );
    gui->addToggle("CAMERA_AUTO", false );
    
    gui->addSlider("MAX_BEAT_HITS", 1, 400, &_maxBeatHits, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    
    gui->addSpacer( GUI_WIDGET_WIDTH, 1);
    
    gui->addSlider("CAM_DISTANCE", 1, 500, &_distance, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    gui->addSlider("CAM_MIN_DISTANCE", 1, 250, &_minDistance, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    gui->addSlider("CAM_MAX_DISTANCE", 1, 500, &_maxDistance, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    
    gui->addRangeSlider("CAM_LONGITUDE", -179 , 179, _minLongitude, _maxLongitude , GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT ) ;
    gui->addRangeSlider("CAM_LATITUDE", -179 , 179, _minLatitude, _maxLatitude , GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT ) ;
    
    //gui->addSlider("CAM_LONGITUDE", -179, 179, &_longitude, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    //gui->addSlider("CAM_LATITUDE", -89, 89, &_latitude, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    
    gui->addSpacer( GUI_WIDGET_WIDTH, 1);
    
    gui->addSlider( "TARGET_SPRING", .0001, .5f, &_targetSpring, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    gui->addSlider( "STEER", .0, 10.f, &_steer, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    gui->addSlider( "PARTICLE_DAMPING", .0, 3.f, &_camParticle.DAMPING, GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    
    gui->addSlider( "Z OFFSET", -4000 , 4000 , &zOffset ,  GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    gui->addSlider( "CAM MOVEMENT", 0 , 500 , &camMovementFactor ,  GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    
    gui->addSlider( "CAMERA TARGET Z", -1000 , 1000 , &cameraTargetZ ,  GUI_WIDGET_WIDTH, GUI_SLIDER_HEIGHT );
    
    gui->addToggle("ENABLE FX" , &bEnablePostFX ) ; 
    gui->addToggle("FXAA PASS", false ) ;
    gui->addToggle("BLOOM PASS", false ) ;
    gui->addToggle("DOF PASS", false ) ;
    gui->addToggle("KALEIDOSCOPE PASS", false ) ;
    gui->addToggle("NOISE WARP PASS", false ) ;
    gui->addToggle("PIXELATE PASS", false ) ;
    gui->addToggle("EDGE PASS", false ) ;
    gui->addToggle("VERTICAL TILT SHIFT" , false ) ; 
    gui->addToggle("GOD RAYS", false) ; 
    gui->addToggle("TOON PASS", false ) ;
    gui->addToggle("SSAO PASS", false ) ; 
    
    //cameraTargetZ
    gui->setScrollArea(a_x, a_y, 320, ofGetHeight() - 10 - a_y);
    ofAddListener( gui->newGUIEvent, this, &CameraManager::guiEvent );
}








