#if !defined(TARGET_RASPBERRY_PI)
#pragma once
#include "BaseEngine.h"

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

class EngineGLFW : public BaseEngine
{
public:

	EngineGLFW()
	{

	};

	//BaseEngine required
	void setup(ImGuiIO*);
	bool createDeviceObjects();
	void invalidateDeviceObjects();
	void onKeyReleased(ofKeyEventArgs& event);

	//custom 
	static void programmableRendererDrawLists(ImDrawData * draw_data);
	static void glRendererDrawLists(ImDrawData * draw_data);

	static unsigned int vaoHandle;

};
#endif