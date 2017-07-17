//
//  Layer.h
//
//  Created by Felix Faire on 18/01/2017.
//
//

#ifndef Layer_h
#define Layer_h


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;

class Layer;
typedef std::shared_ptr<Layer> LayerRef;

/**
    This base class represents a layer in 
    a renderable scene. This layer can be given a scenes
    postprocessing fx and even other layers to render
*/
class Layer
{
public:
    Layer( int width , int height, int msaa = 0 )
    {
        mFbo = gl::Fbo::create( width, height, gl::Fbo::Format().samples( msaa ) );
    }
    
    void update()
    {
        for (const auto& scene : mScenes)
            scene->update();
        
        renderInternal();
    }
    
    void renderInternal()
    {
        if (mAlpha.value() == 0.0f)
            return;
        
        for (const auto& layer : mLayers)
            layer->renderInternal();
        
        {
            gl::ScopedFramebuffer frameScp( mFbo );
            gl::ScopedViewport viewScp( mFbo->getSize() );
            
            if (mFade == 0.0f)
            {
                gl::clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
            }
            else
            {
                gl::ScopedGlslProg colScp( gl::getStockShader( gl::ShaderDef().color() ) );
                gl::color( 0.0f, 0.0f, 0.0f, 1.0f - mFade );
                gl::drawSolidRect( getWindowBounds() );
            }
            
            for (const auto& layer : mLayers)
                layer->drawTex( getWindowBounds() );
            
            for (const auto& scene : mScenes)
                scene->drawInternal();
        
        }
        
    }
    
    void drawTex( Rectf bounds ) const
    {
        if (mAlpha == 0.0f)
            return;
        
        gl::ScopedBlendAlpha alphaScp;
        gl::color( 1.0f, 1.0f, 1.0f, mAlpha.value() );
        gl::draw( mFbo->getColorTexture(), bounds );
    }
    
    void drawFinalTex( Rectf bounds )
    {
        if (mAlpha == 0.0f)
            return;
        
        gl::FboRef finalTexture = mFbo;
        
        if (mPostprocesses.size() > 0)
        {
            postProcessTexture();
            finalTexture = mFinalFbo;
        }
        
        gl::ScopedBlendAlpha alphaScp;
        gl::color( 1.0f, 1.0f, 1.0f, mAlpha.value() );
        gl::draw( finalTexture->getColorTexture(), bounds );
    }
    
    void postProcessTexture()
    {
        // Copy Fbo into post pipeline
        {
            gl::ScopedFramebuffer   fboScp( mFinalFbo );
            gl::ScopedViewport      viewScp( mFinalFbo->getSize() );
            gl::ScopedGlslProg      glScp( gl::getStockShader( gl::ShaderDef().texture() ));
            gl::ScopedTextureBind   texScp( mFbo->getColorTexture() );
            gl::ScopedMatrices      matScp;
            gl::setMatricesWindow( mFinalFbo->getSize() );
            gl::ScopedBlendAlpha    alphaScp;
            gl::clear();
            
            gl::drawSolidRect( Rectf(0, 0, mFinalFbo->getWidth(), mFinalFbo->getHeight()) );
        }
        
        // Process all post activities
        for (const auto& process : mPostprocesses)
        {
            process->process( mFinalFbo->getColorTexture(), mProcessFbo );
            std::swap( mProcessFbo, mFinalFbo );
        }
    }
    
    void addLayer( std::shared_ptr<Layer> newLayer )            { mLayers.push_back( newLayer ); }
    void addScene( std::shared_ptr<Scene> newScene )            { mScenes.push_back( newScene ); }
    
    void addPostProcess( std::shared_ptr<Postprocess> newProcess )
    {
        mPostprocesses.push_back( newProcess );
        
        if (mProcessFbo == nullptr)
        {
            mProcessFbo = gl::Fbo::create( mFbo->getWidth(), mFbo->getHeight() );
            mFinalFbo = gl::Fbo::create( mFbo->getWidth(), mFbo->getHeight() );
        }
    }
    
    void showScene( int index, float duration )
    {
        CI_ASSERT( index < mScenes.size() );
        
        for (int i = 0; i < mScenes.size(); ++i)
        {
            float alpha = (index == i) ? 1.0f : 0.0f;
            timeline().apply( &mScenes[i]->mAlpha, alpha, duration );
        }
    }
    
    void showLayer( int index, float duration )
    {
        CI_ASSERT( index < mLayers.size() );
        
        for (int i = 0; i < mLayers.size(); ++i)
        {
            float alpha = (index == i) ? 1.0f : 0.0f;
            timeline().apply( &mLayers[i]->mAlpha, alpha, duration );
        }
    }
    
    
    Anim<float>                                 mAlpha = 1.0f;
    float                                       mFade = 0.0f;
    
    // Optional array of other layers
    std::vector<std::shared_ptr<Layer>>         mLayers;
    
    // Optional Array of scenes
    std::vector<std::shared_ptr<Scene>>         mScenes;
    
    // Optional array of postprocessing stages
    std::vector<std::shared_ptr<Postprocess>>   mPostprocesses;
    
    gl::FboRef                                  mFbo;
    gl::FboRef                                  mProcessFbo;
    gl::FboRef                                  mFinalFbo;
};

#endif /* Layer_h */
