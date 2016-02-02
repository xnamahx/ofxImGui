#if !defined(TARGET_RASPBERRY_PI)

#include "EngineGLFW.h"

unsigned int EngineGLFW::vaoHandle = 0;

void EngineGLFW::setup(ImGuiIO* io_)
{
	io = io_;

	io->KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
	io->KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io->KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io->KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io->KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io->KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	io->KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	io->KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io->KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io->KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io->KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io->KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io->KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io->KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io->KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io->KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io->KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io->KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io->KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	if (ofIsGLProgrammableRenderer())
	{
		io->RenderDrawListsFn = programmableRendererDrawLists;
	}
	else
	{
		io->RenderDrawListsFn = glRendererDrawLists;
	}

	io->SetClipboardTextFn = &BaseEngine::setClipboardString;
	io->GetClipboardTextFn = &BaseEngine::getClipboardString;

#ifdef _WIN32
	auto window = (ofAppGLFWWindow*)ofGetWindowPtr();
	io->ImeWindowHandle = glfwGetWin32Window(window->getGLFWWindow());
#endif

	ofAddListener(ofEvents().keyReleased, this, &EngineGLFW::onKeyReleased);

	ofAddListener(ofEvents().keyPressed, (BaseEngine*)this, &BaseEngine::onKeyPressed);
	ofAddListener(ofEvents().mousePressed, (BaseEngine*)this, &BaseEngine::onMousePressed);
	ofAddListener(ofEvents().mouseReleased, (BaseEngine*)this, &BaseEngine::onMouseReleased);
	ofAddListener(ofEvents().mouseScrolled, (BaseEngine*)this, &BaseEngine::onMouseScrolled);
	ofAddListener(ofEvents().windowResized, (BaseEngine*)this, &BaseEngine::onWindowResized);

}


void EngineGLFW::programmableRendererDrawLists(ImDrawData * draw_data)
{
	//ImGuiIO * io = &ImGui::GetIO();

	// Backup GL state
	GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
	GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
	GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
	GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
	GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	// Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
	ImGuiIO* io = &ImGui::GetIO();
	int fb_width = (int)(io->DisplaySize.x * io->DisplayFramebufferScale.x);
	int fb_height = (int)(io->DisplaySize.y * io->DisplayFramebufferScale.y);
	draw_data->ScaleClipRects(io->DisplayFramebufferScale);

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

	const float ortho_projection[4][4] =
	{
		{ 2.0f / io->DisplaySize.x, 0.0f,                   0.0f, 0.0f },
		{ 0.0f,                  2.0f / -io->DisplaySize.y, 0.0f, 0.0f },
		{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
		{ -1.0f,                  1.0f,                   0.0f, 1.0f },
	};

	glUseProgram(shaderHandle);
	glUniform1i(attribLocationTex, 0);
	glUniformMatrix4fv(attribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

	glBindVertexArray(vaoHandle);

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

		for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++)
		{
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}

	// Restore modified GL state
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
	glBindVertexArray(last_vertex_array);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFunc(last_blend_src, last_blend_dst);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
}

void EngineGLFW::glRendererDrawLists(ImDrawData * draw_data)
{
	GLint last_blend_src;
	GLint last_blend_dst;
	GLint last_blend_equation_rgb;
	GLint last_blend_equation_alpha;

	glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
	glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);

	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	ImGuiIO * io = &ImGui::GetIO();
	float fb_height = io->DisplaySize.y * io->DisplayFramebufferScale.y;
	draw_data->ScaleClipRects(io->DisplayFramebufferScale);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, io->DisplaySize.x, io->DisplaySize.y, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList * cmd_list = draw_data->CmdLists[n];
		const unsigned char * vtx_buffer = (const unsigned char *)&cmd_list->VtxBuffer.front();
		const ImDrawIdx * idx_buffer = &cmd_list->IdxBuffer.front();

		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void *)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void *)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void *)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd * pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else {
				ofTexture tex;
				tex.texData.textureTarget = GL_TEXTURE_2D;
				tex.setUseExternalTextureID((intptr_t)pcmd->TextureId);
				tex.bind();
				glScissor(
					(int)pcmd->ClipRect.x,
					(int)(fb_height - pcmd->ClipRect.w),
					(int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
					(int)(pcmd->ClipRect.w - pcmd->ClipRect.y)
					);
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_INT, idx_buffer);
				tex.unbind();
			}
			idx_buffer += pcmd->ElemCount;
		}
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFunc(last_blend_src, last_blend_dst);

	last_enable_blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	last_enable_cull_face ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	last_enable_depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	last_enable_scissor_test ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

void EngineGLFW::onKeyReleased(ofKeyEventArgs& event)
{
	int key = event.keycode;
	io->KeysDown[key] = false;

	io->KeyCtrl = io->KeysDown[GLFW_KEY_LEFT_CONTROL] || io->KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io->KeyShift = io->KeysDown[GLFW_KEY_LEFT_SHIFT] || io->KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io->KeyAlt = io->KeysDown[GLFW_KEY_LEFT_ALT] || io->KeysDown[GLFW_KEY_RIGHT_ALT];
	if (key < GLFW_KEY_ESCAPE)
	{
		io->AddInputCharacter((unsigned short)event.codepoint);
	}
}

bool EngineGLFW::createDeviceObjects()
{
	// Backup GL state
	GLint last_texture, last_array_buffer, last_vertex_array;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

	if (ofIsGLProgrammableRenderer())
	{
		string header = "#version 330\n";

		const GLchar *vertex_shader =
			"#version 330\n"
			"uniform mat4 ProjMtx;\n"
			"in vec2 Position;\n"
			"in vec2 UV;\n"
			"in vec4 Color;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"	Frag_UV = UV;\n"
			"	Frag_Color = Color;\n"
			"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const GLchar* fragment_shader =
			"#version 330\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
			"}\n";

		

		shaderHandle = glCreateProgram();
		vertHandle = glCreateShader(GL_VERTEX_SHADER);
		fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(vertHandle, 1, &vertex_shader, 0);
		glShaderSource(fragHandle, 1, &fragment_shader, 0);
		glCompileShader(vertHandle);
		glCompileShader(fragHandle);
		glAttachShader(shaderHandle, vertHandle);
		glAttachShader(shaderHandle, fragHandle);
		glLinkProgram(shaderHandle);

		attribLocationTex = glGetUniformLocation(shaderHandle, "Texture");
		attribLocationProjMtx = glGetUniformLocation(shaderHandle, "ProjMtx");
		attribLocationPosition = glGetAttribLocation(shaderHandle, "Position");
		attribLocationUV = glGetAttribLocation(shaderHandle, "UV");
		attribLocationColor = glGetAttribLocation(shaderHandle, "Color");

		glGenBuffers(1, &vboHandle);
		glGenBuffers(1, &elementsHandle);

		glGenVertexArrays(1, &vaoHandle);
		glBindVertexArray(vaoHandle);

		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);

		glEnableVertexAttribArray(attribLocationPosition);
		glEnableVertexAttribArray(attribLocationUV);
		glEnableVertexAttribArray(attribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
		glVertexAttribPointer(attribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(attribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(attribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

	}

	// Build texture atlas
	unsigned char* pixels;
	int width, height;
	io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

															  // Upload texture to graphics system
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io->Fonts->TexID = (void *)(intptr_t)fontTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return true;
}


void EngineGLFW::invalidateDeviceObjects()
{
	if (vaoHandle) glDeleteVertexArrays(1, &vaoHandle);
	if (vboHandle) glDeleteBuffers(1, &vboHandle);
	if (elementsHandle) glDeleteBuffers(1, &elementsHandle);
	vaoHandle = vboHandle = elementsHandle = 0;

	glDetachShader(shaderHandle, vertHandle);
	glDeleteShader(vertHandle);
	vertHandle = 0;

	glDetachShader(shaderHandle, fragHandle);
	glDeleteShader(fragHandle);
	fragHandle = 0;

	glDeleteProgram(shaderHandle);
	shaderHandle = 0;

	if (fontTexture)
	{
		glDeleteTextures(1, &fontTexture);
		fontTexture = 0;
	}
}


#endif
