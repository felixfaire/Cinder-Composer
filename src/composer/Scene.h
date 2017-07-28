//
//  Scene.h
//
//  Created by Felix Faire on 18/01/2017.
//
//

#ifndef Scene_h
#define Scene_h

#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;

class Scene;
typedef std::shared_ptr<Scene> SceneRef;

/**
    This abstract class contains the contents of the scene
    and can be rendered to a layer
*/
class Scene
{
public:
    
    Scene()
    {
    }
    
    void drawInternal()
    {
        if (mAlpha != 0.0f)
            draw();
    }
    
    virtual void update() = 0;
    virtual void draw() = 0;
    
    Anim<float> mAlpha = 1.0f;
};



#endif /* Scene_h */
