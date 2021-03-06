//
//  ofxFBXManager.cpp
//  ScenesSetup_Nick
//
//  Created by Nick Hardeman on 12/20/13.
//
//

#include "ofxFBXManager.h"

ofxFBXAnimation dummyAnimation;

//--------------------------------------------------------------
ofxFBXManager::ofxFBXManager() {
    bAnimationsEnabled              = true;
    animationIndex                  = 0;
    dummyAnimation.name             = "dummy";
    poseIndex                       = 0;
    bPosesEnabled                   = false;
}

//--------------------------------------------------------------
void ofxFBXManager::setup( ofxFBXScene* aScene ) {
    fbxScene = aScene;
    if(fbxScene->getFBXScene() == NULL) {
        ofLogWarning("ofxFBXManager::setup : FbxScene is NULL, make sure to not unload the ofxFBXScene!");
        return;
    }
    aScene->populateAnimations( animations );
    
    // store transforms so that we can manipulate them and use them to draw later //
    meshTransforms.resize( aScene->getMeshes().size() );
    
    vector< shared_ptr<ofxFBXMesh> > fbxMeshes = aScene->getMeshes();
    meshes.resize( fbxMeshes.size() );
    for(int i = 0; i < fbxMeshes.size(); i++ ){
        fbxMeshes[i]->configureMesh( meshes[i] );
        meshTransforms[i].setParent( *this );
        meshTransforms[i].setTransformMatrix( fbxMeshes[i]->getGlobalTransformMatrix() );
    }
    
    aScene->populateSkeletons( skeletons );
    aScene->populatePoses( poses );
    aScene->populateCachedSkeletonAnimations( cachedSkeletonAnimations );
}

//--------------------------------------------------------------
void ofxFBXManager::update() {
    
    FbxPose * lPose = NULL;
    // poses will override the animations and the settings of the bones //
    if( arePosesEnabled() && hasPoses() && (poseIndex >= 0 && poseIndex < getNumPoses() ) ) {
//        cout << "Got a pose | " << ofGetFrameNum() << endl;
        lPose = fbxScene->getFBXScene()->GetPose( poseIndex );
    }
    
    if( !areAnimationsEnabled() || !hasAnimations() ) {
        FbxTime ttime(FBXSDK_TIME_INFINITE);
//        cout << "Calling update bones: " << " | " << ofGetElapsedTimef() << endl;
        for(int i = 0; i < skeletons.size(); i++ ) {
            skeletons[i]->update( ttime, lPose );
        }
        
    }
    
    if(animations.size() < 1) return;
    if(!areAnimationsEnabled()) return;
    
//    cout << "Should not be reaching here: ofxFBXManager :: update | " << ofGetFrameNum() << endl;
    
    animations[animationIndex].update();
    
//    cout << "ofxFBXManager :: update : animations | " << ofGetElapsedTimef() << endl;
    
    if(animations[animationIndex].isFrameNew() || animations[animationIndex].isPaused() ) {
        
        if( cachedSkeletonAnimations.size() ) {
            for(int i = 0; i < skeletons.size(); i++ ) {
                cachedSkeletonAnimations[animationIndex][i].update( animations[animationIndex].getFrameNum(), skeletons[i] );
            }
        } else {
            for(int i = 0; i < skeletons.size(); i++ ) {
                skeletons[i]->updateWithAnimation( animations[animationIndex].fbxCurrentTime, lPose );
            }
        }
    }
}

//--------------------------------------------------------------
void ofxFBXManager::lateUpdate() {
    
    if( !areAnimationsEnabled() || !hasAnimations() ) {
        FbxTime ttime(FBXSDK_TIME_INFINITE);
//        cout << "ofxFBXManager :: lateUpdate : external bones control " << ofGetFrameNum() << endl;
        for( int i = 0; i < skeletons.size(); i++ ) {
            skeletons[i]->lateUpdate();
        }
        
        vector< shared_ptr<ofxFBXMesh> > fbxMeshes = fbxScene->getMeshes();
        for(int i = 0; i < fbxMeshes.size(); i++ ) {
            fbxMeshes[i]->updateMesh( &meshes[i], ttime, currentFbxAnimationLayer, NULL );
        }
    }
    
    if(animations.size() > 0 && areAnimationsEnabled() ) {
        
//        cout << "ofxFBXManager :: lateUpdate : animations | " << ofGetElapsedTimef() << endl;
//        if(animations[animationIndex].isFrameNew() || animations[animationIndex].isPaused() ) {
            vector< shared_ptr<ofxFBXMesh> > fbxMeshes = fbxScene->getMeshes();
            for( int i = 0; i < skeletons.size(); i++ ) {
                skeletons[i]->lateUpdateWithAnimation();
            }
            
            for(int i = 0; i < fbxMeshes.size(); i++ ) {
                fbxMeshes[i]->updateMesh( &meshes[i], animations[animationIndex].fbxCurrentTime, currentFbxAnimationLayer, NULL );
            }
            
//        }
    }
}

//--------------------------------------------------------------
void ofxFBXManager::draw() {
    drawMeshes();
}

//--------------------------------------------------------------
void ofxFBXManager::drawMeshes() {
    vector< shared_ptr<ofxFBXMesh> > fbxMeshes = fbxScene->getMeshes();
    for(int i = 0; i < fbxMeshes.size(); i++ ) {
        meshTransforms[i].transformGL();
        fbxMeshes[i]->draw( &meshes[i] );
        meshTransforms[i].restoreTransformGL();
    }
}

//--------------------------------------------------------------
void ofxFBXManager::drawMeshNormals( float aLen, bool aBFaceNormals ) {
    vector< shared_ptr<ofxFBXMesh> > fbxMeshes = fbxScene->getMeshes();
    for(int i = 0; i < fbxMeshes.size(); i++ ) {
        meshTransforms[i].transformGL();
        fbxMeshes[i]->drawNormals( &meshes[i], aLen, aBFaceNormals );
        meshTransforms[i].restoreTransformGL();
    }
}

//--------------------------------------------------------------
void ofxFBXManager::drawSkeletons( float aLen ) {
    transformGL();
    for(int i = 0; i < skeletons.size(); i++ ) {
        skeletons[i]->draw( aLen );
    }
    restoreTransformGL();
}

//--------------------------------------------------------------
vector< ofMesh >& ofxFBXManager::getMeshes() {
    return meshes;
}

//--------------------------------------------------------------
ofxFBXScene* ofxFBXManager::getFbxScene() {
    return fbxScene;
}

#pragma mark - Animation
//--------------------------------------------------------------
int ofxFBXManager::getNumAnimations() {
    return animations.size();
}

//--------------------------------------------------------------
int ofxFBXManager::getCurrentAnimationIndex() {
    return animationIndex;
}

//--------------------------------------------------------------
ofxFBXAnimation& ofxFBXManager::getCurrentAnimation() {
    if(animations.size() < 1) {
        ofLogWarning("ofxFBXAnimation :: return dummy animation, there are no animations");
        return dummyAnimation;
    }
    return animations[ animationIndex ];
}

//--------------------------------------------------------------
ofxFBXAnimation& ofxFBXManager::getAnimation( int aIndex ) {
    if( aIndex > animations.size() -1 ) {
        ofLogWarning( "ofxFBXManager :: getAnimation : index is too high " ) << aIndex;
        aIndex = ofClamp(aIndex, 0, animations.size()-1);
    }
    if( animations.size() == 0 ) {
        aIndex = 0;
        animations.push_back( dummyAnimation );
    }
    
    return animations[ aIndex ];
}

//--------------------------------------------------------------
void ofxFBXManager::setAnimation( int aIndex ) {
    if(animations.size() < 1) {
        ofLogWarning("ofxFBXManager :: setAnimation : returning because there are no animations!");
        return;
    }
    if(aIndex >= animations.size()) {
        aIndex = ofClamp(aIndex, 0, animations.size()-1);
        ofLogWarning("ofxFBXManager :: setAnimation : index to high, clamping to ") << aIndex;
    }
    FbxAnimStack * lCurrentAnimationStack = fbxScene->getFBXScene()->FindMember<FbxAnimStack>( (&animations[aIndex].fbxname)->Buffer() );
    if (lCurrentAnimationStack == NULL) {
        // this is a problem. The anim stack should be found in the scene!
        ofLogWarning("ofxFBXManager :: setAnimation : the anim stack was not found in the scene!");
        return;
    }
    currentFbxAnimationLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
    fbxScene->getFBXScene()->GetEvaluator()->SetContext( lCurrentAnimationStack );
    animationIndex = aIndex;
}

//--------------------------------------------------------------
void ofxFBXManager::enableAnimations() {
    if(!hasAnimations()) {
        ofLogWarning("ofxFBXManager :: enableAnimations : there are no animations in this scene. Disabling.");
        disableAnimations();
        return;
    }
    bAnimationsEnabled = true;
    for( int i = 0; i < skeletons.size(); i++ ) {
        skeletons[i]->enableAnimation();
    }
}

//--------------------------------------------------------------
void ofxFBXManager::disableAnimations() {
    bAnimationsEnabled = false;
    for( int i = 0; i < skeletons.size(); i++ ) {
        skeletons[i]->disableAnimation();
    }
}

//--------------------------------------------------------------
void ofxFBXManager::toggleAnimationsEnabled() {
    if( bAnimationsEnabled ) {
        disableAnimations();
    } else {
        enableAnimations();
    }
}

//--------------------------------------------------------------
bool ofxFBXManager::areAnimationsEnabled() {
    return (bAnimationsEnabled && hasAnimations());
}

//--------------------------------------------------------------
bool ofxFBXManager::hasAnimations() {
    if( animations.size() > 1 ) return true;
    if( animations.size() == 1 ) {
        if( animations[0].name == "dummyAnimation" ) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

#pragma mark - Bones
//--------------------------------------------------------------
vector< shared_ptr<ofxFBXSkeleton> >& ofxFBXManager::getSkeletons() {
    return skeletons;
}

// called after we manipulate the skeletons, so that if other items are using
// the bones, than it will reset them //
//--------------------------------------------------------------
void ofxFBXManager::resetSkeletons() {
    for( int i = 0; i < skeletons.size(); i++ ) {
        skeletons[i]->reset();
    }
}

//--------------------------------------------------------------
bool ofxFBXManager::hasBones() {
    return getNumBones() > 0;
}

//--------------------------------------------------------------
int ofxFBXManager::getNumSkeletons() {
    return skeletons.size();
}

//--------------------------------------------------------------
int ofxFBXManager::getNumBones() {
    int tNumBones = 0;
    for( int i = 0; i < skeletons.size(); i++ ) {
        tNumBones += skeletons[i]->getNumBones();
    }
    return tNumBones;
}

//--------------------------------------------------------------
ofxFBXBone* ofxFBXManager::getBone( string aBoneName, int aSkeletonIndex ) {
    if( aSkeletonIndex < 0 ) return NULL;
    if( aSkeletonIndex >= skeletons.size() ) return NULL;
    return skeletons[ aSkeletonIndex ]->getBone( aBoneName );
}

//--------------------------------------------------------------
string ofxFBXManager::getSkeletonInfo() {
    string retStr = "";
    for( int i = 0; i < skeletons.size(); i++ ) {
        retStr += skeletons[i]->toString();
    }
    return retStr;
}

#pragma mark - Poses
//--------------------------------------------------------------
bool ofxFBXManager::hasPoses() {
    return getNumPoses() > 0;
}

//--------------------------------------------------------------
bool ofxFBXManager::arePosesEnabled() {
    return bPosesEnabled;
}

//--------------------------------------------------------------
void ofxFBXManager::enablePoses() {
    bPosesEnabled = true;
}

//--------------------------------------------------------------
void ofxFBXManager::disablePoses() {
    bPosesEnabled = false;
}

//--------------------------------------------------------------
void ofxFBXManager::togglePosesEnabled() {
    if( arePosesEnabled() ) disablePoses();
    else enablePoses();
}

//--------------------------------------------------------------
int ofxFBXManager::getNumPoses() {
    return (int)poses.size();
}

//--------------------------------------------------------------
int ofxFBXManager::setPoseIndex( int aIndex ) {
    aIndex = ofClamp( aIndex, 0, getNumPoses()-1 );
    poseIndex = aIndex;
}

//--------------------------------------------------------------
shared_ptr< ofxFBXPose > ofxFBXManager::getCurrentPose() {
    if( hasPoses() && (poseIndex >= 0 && poseIndex < getNumPoses() ) ) {
        return poses[ poseIndex ];
    }
    shared_ptr<ofxFBXPose> tpose;
    return tpose;
}

//--------------------------------------------------------------
vector< shared_ptr<ofxFBXPose> > ofxFBXManager::getPoses() {
    return poses;
}











