#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
    ofDisableArbTex();
    
    ofxFBXSceneSettings settings;
    
    string filename = "humanoid.fbx";
    
    if( scene.load(filename, settings) ) {
        cout << "testApp :: loaded the scene OK" << endl;
    } else {
        cout << "testApp :: Error loading the scene" << endl;
    }
    
    cam.lookAt( ofVec3f(0,0,0) );
    cam.setDistance( 550 );
    cam.setFarClip(6000);
    cam.setNearClip( .5f );
    
    fbxMan.setup( &scene );
    fbxMan.setAnimation(0);
    
    bRenderNormals  = false;
    bRenderMeshes   = true;
    bDrawBones      = false;
    
}

//--------------------------------------------------------------
void testApp::update() {
    
//    light.setPosition( cos(ofGetElapsedTimef()) * 20.f, sin(ofGetElapsedTimef()*4) * 50 + 10, sin(ofGetElapsedTimef()) * 20.f + 100);
    
    fbxMan.update();
    
    // perform any bone manipulation here //
    
    fbxMan.lateUpdate();
    
}

//--------------------------------------------------------------
void testApp::draw() {
    
    ofSetColor(255, 255, 255);
    
    glEnable( GL_DEPTH_TEST );
    
    
    cam.begin();
    
    ofEnableLighting();
    light.enable();
    
    if( bRenderMeshes ) {
        ofSetColor( 255, 255, 255 );
        fbxMan.draw();
    }
    
    light.disable();
    ofDisableLighting();
    
    if(bDrawBones) {
        fbxMan.drawSkeletons( 6. );
    }
    
    if( bRenderNormals ) {
        ofSetColor( 255, 0, 255 );
        fbxMan.drawMeshNormals( 1, false );
    }
    
    
    ofSetColor( light.getDiffuseColor() );
    light.draw();
    
    cam.end();
    
    glDisable( GL_DEPTH_TEST );
    
    int numBones = 0;
    vector< shared_ptr<ofxFBXSkeleton> >& skeletons = fbxMan.getSkeletons();
    for( int i = 0; i < skeletons.size(); i++ ) {
        numBones += skeletons[i]->getNumBones();
    }
    
    ofSetColor( 60, 60, 60 );
    stringstream ds;
    ds << "Render normals (n): " << bRenderNormals << endl;
    ds << "Render meshes (m): " << bRenderMeshes << endl;
    ds << "Render " << numBones << " bones (b): " << bDrawBones << endl;
    if( fbxMan.areAnimationsEnabled() ) {
        ds << "Toggle play/pause (spacebar): playing: " << fbxMan.getCurrentAnimation().isPlaying() << endl;
        ds << "Previous/Next animation (up/down): " << fbxMan.getCurrentAnimation().name << endl;
    }
    ds << "Scale is " << fbxMan.getScale() << endl;
    if( fbxMan.getNumPoses() > 0 ) {
        ds << "Pose: " << fbxMan.getCurrentPose()->getName() << " num poses: " << fbxMan.getNumPoses() << " enabled (p): " << fbxMan.arePosesEnabled() << endl;
    }
    ofDrawBitmapString( ds.str(), 50, 30 );
    
    
    for(int i = 0; i < fbxMan.getNumAnimations(); i++ ) {
        stringstream ss;
        ofxFBXAnimation& anim = fbxMan.getAnimation( i );
        if( i == fbxMan.getCurrentAnimationIndex() ) {
            ss << "- ";
        }
        ss << "name: " << anim.name << " " << ofToString(anim.getPositionSeconds(), 3) << " | " << ofToString(anim.getDurationSeconds(), 3) << " frame: " << anim.getFrameNum() << " / " << anim.getTotalNumFrames() << endl;
        ofDrawBitmapString( ss.str(), 50, i * 30 + 450 );
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key) {
    if(scene.getNumAnimations() > 1) {
        if(key == OF_KEY_DOWN ) {
            int newAnimIndex = fbxMan.getCurrentAnimationIndex()+1;
            if(newAnimIndex > scene.getNumAnimations()-1 ) {
                newAnimIndex = 0;
            }
            fbxMan.setAnimation( newAnimIndex );
            
        }
        if(key == OF_KEY_UP ) {
            int newAnimIndex = fbxMan.getCurrentAnimationIndex()-1;
            if(newAnimIndex < 0 ) {
                newAnimIndex = scene.getNumAnimations()-1;
            }
            fbxMan.setAnimation( newAnimIndex );
        }
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key) {
    if(key == ' ') {
        fbxMan.getCurrentAnimation().togglePlayPause();
    }
    if(key == 'n') {
        bRenderNormals = !bRenderNormals;
    }
    if(key == 'm') {
        bRenderMeshes = !bRenderMeshes;
    }
    if(key == 'b') {
        bDrawBones = !bDrawBones;
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)    {
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button) {
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h) {
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg) {
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo) {
    
}
















