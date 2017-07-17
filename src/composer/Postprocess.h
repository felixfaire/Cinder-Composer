//
//  Postprocess.h
//
//  Created by Felix Faire on 18/01/2017.
//
//

#ifndef SceneLayer_h
#define SceneLayer_h


#include "cinder/gl/gl.h"


using namespace ci;
using namespace ci::app;


/**
    This abstract class represents a post processing stage that can happen
    on a layer
*/
class Postprocess
{
public:
    Postprocess(){}
    
    virtual void process( const gl::TextureRef& inputTexture, gl::FboRef& targetFbo ) = 0;
};

/**
    This class allows the simple specification of a fragment shader to do the post processing
    the shader must specify
    
    uniform sampler2D tex0;
    in vec2 TexCoord
*/
class ShaderPostprocess;
typedef std::shared_ptr<ShaderPostprocess> ShaderPostprocessRef;

class ShaderPostprocess : public Postprocess
{
public:
    ShaderPostprocess( DataSourceRef fragmentShaderSource )
    {
        CI_ASSERT(fragmentShaderSource->isFilePath());
        
        gl::GlslProg::Format fmt;
        fmt.vertex( getThruVertex() ).fragment( fragmentShaderSource );
        createShader( fmt );
    }
    
    ShaderPostprocess( const std::string& fragmentShaderString )
    {
        gl::GlslProg::Format fmt;
        fmt.vertex( getThruVertex() ).fragment( fragmentShaderString );
        createShader( fmt );
    }
    
    void process( const gl::TextureRef& texture, gl::FboRef& targetFbo ) override
    {
        CI_ASSERT(texture != nullptr);
        CI_ASSERT(targetFbo != nullptr);
        CI_ASSERT(mPostProcessShader != nullptr);
        
        gl::ScopedFramebuffer   fboScp( targetFbo );
        gl::ScopedViewport      viewScp( targetFbo->getSize() );
        gl::ScopedGlslProg      glScp( mPostProcessShader );
        gl::ScopedTextureBind   texScp( texture );
        gl::ScopedMatrices      matScp;
        gl::setMatricesWindow( targetFbo->getSize() );
        gl::clear();
        
        mPostProcessShader->uniform( "tex0", 0 );
        updateUniforms( mPostProcessShader );
        gl::drawSolidRect( Rectf(0, 0, targetFbo->getWidth(), targetFbo->getHeight()) );
    }
    
    virtual void updateUniforms( gl::GlslProgRef& prog ) {}
    
private:

    const std::string getThruVertex()
    {
        return CI_GLSL( 150,

            uniform mat4 ciModelViewProjection;

            in vec4 ciPosition;
            in vec2 ciTexCoord0;

            out vec2 TexCoord;

            void main()
            {
                TexCoord = ciTexCoord0;
                gl_Position = ciModelViewProjection * ciPosition;
            }
            
        );
    }
    
    void createShader( const gl::GlslProg::Format& fmt )
    {
        try
        {
          mPostProcessShader = gl::GlslProg::create( fmt );
        }
        catch( const std::exception& e )
        {
          std::cout << "PostProcess shader error: " << e.what() << std::endl;
          CI_ASSERT( false );
        }
    }
    
    gl::GlslProgRef mPostProcessShader;
};



#endif /* SceneLayer_h */
