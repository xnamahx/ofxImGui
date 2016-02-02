#pragma once
#include "ofMain.h"
#include "imgui.h"

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

class BaseEngine
{
public:
    
    BaseEngine()
    {
        io = NULL;
    }
    virtual ~BaseEngine()
    {
        io = NULL;
    }
    
    ImGuiIO* io; 
    
    
    virtual void setup(ImGuiIO*)=0;
    virtual bool createDeviceObjects()=0;
    
    virtual void onKeyReleased(ofKeyEventArgs& event)=0;
    
    
    void onKeyPressed(ofKeyEventArgs& event);
    void onMousePressed(ofMouseEventArgs& event);
    void onMouseReleased(ofMouseEventArgs& event);
    void onMouseScrolled(ofMouseEventArgs& event);
    void onWindowResized(ofResizeEventArgs& window);

    GLuint loadTextureImage2D(unsigned char * pixels, int width, int height);
    
    static const char* getClipboardString();
    static void setClipboardString(const char * text);
    
    static unsigned int vboHandle;
    static unsigned int elementsHandle;

	static int attribLocationProjMtx;
	static int attribLocationTex;
	static int attribLocationColor;
	static int attribLocationPosition;
	static int attribLocationUV;

	static int shaderHandle;
	static int vertHandle;
	static int fragHandle;

	static GLuint fontTexture;
    static ofShader shader;
};

