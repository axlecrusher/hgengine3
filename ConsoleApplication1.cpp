// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Win32Window.h>
//#define GL_GLEXT_PROTOTYPES
//#include <glcorearb.h>
//#include <glext.h>
#include <glew.h>

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <HgElement.h>
#include <HgScene.h>
#include <shapes.h>
#include <oglDisplay.h>

#include <triangle.h>
#include <cube.h>

#include <oglShaders.h>
#include <HgMath.h>
#include <HgTypes.h>
#include <HgRenderQueue.h>

#include <HgShader.h>

#include <Gravity.h>
#include <HgInput.h>
#include <Projectile.h>
#include <HgModel.h>

#include <symbol_enumerator.h>
#include <FileWatch.h>
}




#define M_PI 3.14159265358979323846

float projection[16];

extern viewport view_port[];
HgCamera camera[2];

HANDLE endOfRenderFrame = NULL;

volatile int8_t needRender = 1;

#define EYE_DISTANCE -0.07f
uint8_t stereo_view = 0;

//uint8_t KeyDownMap[512];

MercuryWindow* window = NULL;

#define USE_RENDER_THREAD 0

void StartWindowSystem() {
	window = MercuryWindow::MakeWindow();

	GLenum err = glewInit();

	printf("%s\n", glGetString(GL_VERSION));
	printf("%s\n", glGetString(GL_VENDOR));

	GLint d;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &d);
	printf("GL_MAX_VERTEX_ATTRIBS %d\n", d);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &d);
	printf("GL_MAX_VERTEX_UNIFORM_COMPONENTS %d\n", d);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &d);
	printf("GL_MAX_VERTEX_UNIFORM_VECTORS %d\n", d);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &d);
	printf("GL_MAX_VERTEX_UNIFORM_BLOCKS %d\n", d);

}

void BeginFrame() {
		window->PumpMessages();
		window->Clear();
		_projection = projection;
}

int32_t RenderThreadLoop() {
	MercuryWindow* w = window;

	uint8_t stop_frame = 0;
	while (1) {
		BeginFrame();

		while (stop_frame == 0) {
			render_packet* x = hgRenderQueue_pop();
			while (x == NULL) {
				Sleep(1);
				x = hgRenderQueue_pop();
			}

			if (x->renderData == NULL) {
				stop_frame = 1;
			}
			else {
				draw_render_packet(x);
			}

			free(x);
		}

		needRender = 1;

//		glFlush();
//		glFinish();

//		w->SwapBuffers();

		//signal main game thread that we are done rendering frame.
		//swap buffers is slow, calling this before swapping allows the main thread to begin processing the next frame step
		//I think this screws up the main thread's time step because the time will be read well before the frame is done causing jitter in time based movement.
		if (!SetEvent(endOfRenderFrame))
		{
			printf("SetEvent failed endOfRenderFrame (%d)\n", GetLastError());
			return 1;
		}

		stop_frame = 0;

		w->SwapBuffers();

		
//		Sleep(10);
	}
}

DWORD WINAPI StartRenderThread(LPVOID lpParam) {
	StartWindowSystem();
	return RenderThreadLoop();
}

volatile LONG itrctr;
HgScene scene;

DWORD WINAPI PrintCtr(LPVOID lpParam) {
	while (1) {
		printf("UPS %u e_count %d %d\n", itrctr, scene.size_used, scene._size);
		itrctr = 0;
		CheckFilesForChange();
		Sleep(1000);
	}
}

#define ANI_TRIS 400

void fire(HgScene* scene) {
	HgElement* element = NULL;

	scene_newElement(scene, &element);
	projectile_create(element);

	vector3 v = { 0 };
	v.components.z = -1; //into screen from camera seems to be -1

	camera->rotation.w = -camera->rotation.w;
	v = vector3_quat_rotate(&v, &camera->rotation);
	element->rotation = camera->rotation;
	camera->rotation.w = -camera->rotation.w;

	ProjectileData *pd = (ProjectileData*)element->extraData;
	pd->direction = v;
	element->position = camera->position;
}

void(*submit_for_render)(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx);

void submit_for_render_threaded(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx) {
	if (s == NULL) {
		hgRenderQueue_push(create_render_packet(NULL, 0, NULL, NULL, 0));
		return;
	}
	HgElement* e = get_element(s,idx);
	hgRenderQueue_push(create_render_packet(e, viewport_idx, camera + 0, s, idx)); //submit to renderer
}

void submit_for_render_serial(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx) {
	if (s == NULL) {
		window->SwapBuffers();
		return;
	}
//	printf("serial\n");
	hgViewport(viewport_idx);

	HgElement* e = get_element(s, idx);
	RenderData* rd = e->m_renderData;
	if (rd->shader) VCALL(rd->shader, enable);

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated

	setGlobalUniforms(camera);
	setLocalUniforms(&e->rotation, &e->position, e->scale);

	rd->renderFunc(rd);
}

void quick_render(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx) {
	HgElement* e = get_element(s, idx);
	RenderData* rd = e->m_renderData;
//	if (rd->shader) VCALL(rd->shader, enable);
	hgViewport(viewport_idx);

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated

	setGlobalUniforms(camera);
//	setLocalUniforms(&e->rotation, &e->position, e->scale);

	rd->renderFunc(rd);
}

void vertex_print(const vertex* v) {
	printf("%f %f %f\n", v->array[0], v->array[1], v->array[2]);
}

int SymnumCheck(const char * path, const char * name, void * location, long size)
{
	if (strncmp(name, "REGISTER", 8) == 0)
	{
		typedef void(*sf)();
		sf fn = (sf)location;
		fn();
	}
	return 0;
}


int main()
{
	EnumerateSymbols(SymnumCheck);

//	MercuryWindow* w = MercuryWindow::MakeWindow();
//	generateVoxelVBO();

	hgvbo_init(&staticVbo, VBO_VC);
	hgvbo_init(&staticVboVNU, VBO_VNU);

//	model_data d = LoadModel("test.hgmdl");
//	hgvbo_add_data_vc(&staticVbo, d.vertices, d.vertices, d.vertex_count);

	_create_shader = HGShader_ogl_create;
	new_RenderData = new_renderData_ogl;

	if (stereo_view) {
		setup_viewports(1280, 480);
//		Perspective2(60, 1280.0 / 480.0, 0.1f, 100.0f, projection);
	}
	else {
		setup_viewports(640, 480);
	}

	Perspective2(60, 640.0 / 480.0, 0.1f, 100.0f, projection);

	uint8_t s = sizeof(HgElement);
	printf("element size %d\n", s);
	printf("vertex size %d\n", sizeof(vertex));
	printf("render_packet size %d\n", sizeof(render_packet));
	printf("oglrd size %d\n", sizeof(OGLRenderData));
	printf("vbo type size %d\n", sizeof(VBO_TYPE));

//	Perspective(60, 640.0 / 480.0, 0.1f, 100.0f, projection);
//	Perspective2(60, 640.0/480.0, 0.1f, 100.0f,projection);
//	Perspective2(60, 320.0 / 480.0, 0.1f, 100.0f, projection);

	quaternion_init(&camera[0].rotation);
	memset(camera[0].position.array, 0, sizeof camera[0].position);
	camera[0].position.components.z = 1.5f;
	camera[0].position.components.y = 2.0f;
	toQuaternion2(0, 15, 0, &camera[0].rotation); //y,x,z

	camera[1] = camera[0];
	camera[1].position.components.x += EYE_DISTANCE;

//	MatrixMultiply4f(projection, camera, view);

	print_matrix(projection);
	printf("\n");

	scene_init(&scene);
//	uint32_t tris[ANI_TRIS];
	HgElement* tris[ANI_TRIS];

	GravityField gravity = { 0 };
	allocate_space(&gravity, ANI_TRIS);
	gravity.scene = &scene;
	gravity.vector.components.y = -1;
uint32_t i;
	{
		HgElement* element = NULL;

		scene_newElement(&scene, &element);
		shape_create_triangle(element);
		element->position.components.x = 1.5f;
		element->position.components.z = -1.0f;
		//	toQuaternion2(0, 0, 90, &element->rotation);

		scene_newElement(&scene, &element);
		shape_create_triangle(element);
		element->position.components.x = -0.0f;
		element->position.components.z = -2.0f;
		//	toQuaternion2(45,0,0,&element->rotation);

		
		for (i = 0; i < ANI_TRIS; i++) {
			scene_newElement(&scene, &element);
			tris[i] = element;
//			gravity.indices[i] = tris[i];
			//		shape_create_triangle(element);
			shape_create_cube(element);
			float x = (i % 20)*1.1f;
			float z = (i / 20)*1.1f;
			element->position.components.y = 5.0f;
			element->position.components.x = -10.0f + x;
			element->position.components.z = -2.0f - z;
			element->scale = 0.3f;
//			element->m_renderData->shader = HGShader_acquire("test_vertex2.glsl", "test_frag2.glsl");
			element->m_renderData->shader = HGShader_acquire("basic_light1_v.glsl", "basic_light1_f.glsl");
		}

		scene_newElement(&scene, &element);
		model_load(element, "test.hgmdl");
		element->scale = 0.5f;
		element->position.components.z = -4;
//		element->position.components.y = 2;
//		toQuaternion2(0, 90, 0, &element->rotation);
//		element->m_renderData->shader = HGShader_acquire("test_vertex2.glsl", "test_frag2.glsl");
		element->m_renderData->shader = HGShader_acquire("basic_light1_v.glsl", "basic_light1_f.glsl");
		//	model_data d = LoadModel("test.hgmdl");

		//	model_data d = LoadModel("test.hgmdl");
	}
	
	HANDLE thread1 = CreateThread(NULL, 0, &PrintCtr, NULL, 0, NULL);

#if (USE_RENDER_THREAD)
	endOfRenderFrame = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		NULL  // object name
	);

	HANDLE thread = CreateThread(NULL, 0, &StartRenderThread, NULL, 0, NULL);
	submit_for_render = submit_for_render_threaded;
#else
	StartWindowSystem();
	submit_for_render = submit_for_render_serial;
#endif

	/*
	StartThreadData *mkThreadData;
	void * m_thread;

	mkThreadData = new StartThreadData;
	mkThreadData->m_pData = data;
	mkThreadData->m_pFunc = fn;
	m_thread = CreateThread(NULL, 0, &StartThread, mkThreadData, 0, NULL);
	*/

//	vertices triangle;
//	gen_triangle(&triangle);

#ifdef WIN32
//	SetupOGLExtensions();
#endif

	uint32_t stime = GetTickCount();
	uint32_t time = stime;
	uint32_t dtime = time - stime;

	uint32_t ltime = 0;
	uint32_t ddtime = time - stime;

	int8_t do_render = 1;
	uint8_t did_change = 0;

	int16_t mouse_x, mouse_y;
	mouse_x = mouse_y = 0;
	while (1) {
		dtime = GetTickCount() - stime;
		ddtime = dtime - ltime;
		ltime = dtime;

#if (!USE_RENDER_THREAD)
		BeginFrame();
#endif

		if (ddtime > 0) {
			vector3 v;
			vector3_zero(&v);

			if (KeyDownMap['w']) v.components.z -= 1.0f;
			if (KeyDownMap['s']) v.components.z += 1.0f;
			if (KeyDownMap['a']) v.components.x -= 1.0f;
			if (KeyDownMap['d']) v.components.x += 1.0f;
			if (KeyDownMap['r']) {
				mouse_x = 0; mouse_y = -42;
			}

			if (KeyDownMap[' ']) {
//				printf("fire!\n");
				fire(&scene);
			}

//			if (v.components.z > 0) DebugBreak();

			float scale = (1.0f / 1000.0f) * ddtime;
			v = vector3_normalize(&v);
//			v = vector3_scale(&v, -1.0f);

			camera->rotation.w = -camera->rotation.w; //invert coordinate system for vector rotation
			v= vector3_quat_rotate(&v, &camera->rotation);
			camera->rotation.w = -camera->rotation.w; //restore

//			v = vector3_normalize(&v);

			v = vector3_scale(&v, scale);
			camera->position = vector3_add(&camera->position, &v);

			mouse_x = (MOUSE_INPUT.dx + mouse_x) % 2000;
			mouse_y = (MOUSE_INPUT.dy + mouse_y) % 2000;


			//seperate quaternions keep the camera from rolling when yawing and pitching
			quaternion yaw, pitch;
			toQuaternion2((-MOUSE_INPUT.dx / 2000.0f)*360, 0, 0, &yaw);
			toQuaternion2(0, (-MOUSE_INPUT.dy / 2000.0f) * 360, 0, &pitch);

			camera->rotation = quat_mult(&pitch, &camera->rotation);
			camera->rotation = quat_mult(&camera->rotation, &yaw);
			//normalize quaternion?

			MOUSE_INPUT.dx = 0;
			MOUSE_INPUT.dy = 0;
		}

		if (needRender > 0) {
			do_render = needRender;
			needRender = 0;
		}

		if (dtime > 10000) { // && did_change==0) {
			did_change = 1;

//			if (ddtime>0) gravity_update(&gravity, ddtime);
			/*
			for (i = 0; i < ANI_TRIS; i++) {
				change_to_triangle(tris[i]);
			}
			*/
		}

//		printf("dtime: %d\n", ddtime);

		{
			HgElement* element = tris[0];
			//		y,x,z
			toQuaternion2((dtime % 10000) / 27.777777777777777777777777777778, 0, 0, &element->rotation);

			for (i = 0; i < ANI_TRIS; i++) {
				HgElement* e = tris[i];
				memcpy(&e->rotation, &element->rotation, sizeof element->rotation);
			}
		}

		camera[1] = camera[0];
		camera[1].position.components.x += EYE_DISTANCE;

		for (uint32_t i=0; i<scene._size; ++i) {
//			if (IS_USED(&scene, i) == 0) continue;
			if(is_used(&scene, i) == 0) continue;
			HgElement* e = get_element(&scene,i);

			if (ddtime > 0) {
				VCALL_IDX(e, updateFunc, ddtime);
			}

			/* FIXME: WARNING!!! if this loop is running async to the render thread, element deletion can cause a crash!*/
			if (CHECK_FLAG(e, HGE_DESTROY) > 0) {
				scene_delete_element(&scene, i);
				continue;
			}
			if ((CHECK_FLAG(e, HGE_HIDDEN) == 0) && (do_render > 0)) {
				submit_for_render(stereo_view, camera + 0, &scene, i);
//				quick_render(2, camera + 1, &scene, i);
			}
		}

		if (stereo_view && do_render > 0) {
			for (uint32_t i = 0; i < scene._size; ++i) {
				if (is_used(&scene, i) == 0) continue;
				HgElement* e = get_element(&scene, i);
				if (CHECK_FLAG(e, HGE_HIDDEN) == 0) submit_for_render(2, camera + 1, &scene, i);
			}
		}

//		scene_clearUpdate(&scene);

		InterlockedAdd(&itrctr,1);


		if ((do_render > 0)) submit_for_render(2, camera + 1, NULL, 0);
//			hgRenderQueue_push(create_render_packet(NULL, 2, camera + 1, NULL, 0)); //null element to indicate end of frame

		do_render = 0;

//		Sleep(1);

#if  (USE_RENDER_THREAD)
		DWORD dwWaitResult = WaitForSingleObject(
			endOfRenderFrame, // event handle
			INFINITE);    // indefinite wait

		ResetEvent(endOfRenderFrame);
#else
		needRender = 1;
#endif
	}

    return 0;
}

