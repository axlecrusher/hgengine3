// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include <Win32Window.h>

#include <stdio.h>
#include <stdlib.h>

#include <HgElement.h>
#include <HgScene.h>
#include <shapes.h>

#include <HgMath.h>
#include <HgTypes.h>
#include <HgRenderQueue.h>

#include <HgShader.h>

#include <Gravity.h>
#include <HgInput.h>
#include <Projectile.h>
#include <HgModel.h>

#include <FileWatch.h>

#include <HgTimer.h>
#include <RenderBackend.h>
#include <HgUtils.h>


#include <math/vector.h>

float projection[16];

extern viewport view_port[];
HgCamera camera[2];

HANDLE endOfRenderFrame = NULL;

volatile int8_t needRender = 1;

#define EYE_DISTANCE -0.07f

//uint8_t KeyDownMap[512];

MercuryWindow* window = NULL;

#define USE_RENDER_THREAD 0

void StartWindowSystem() {
	window = MercuryWindow::MakeWindow();
	Renderer::InitOpenGL();
}

void BeginFrame() {
	window->PumpMessages();
	RENDERER->Clear();
	RENDERER->BeginFrame();
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
		printf("UPS %u e_count %d %d\n", itrctr, scene.usedCount(), scene.chunkCount());
		itrctr = 0;
		CheckFilesForChange();
		Sleep(1000);
	}
}

#define ANI_TRIS 400

void fire(HgScene* scene) {
	HgElement* element = NULL;

	create_element("basic_projectile", scene, &element);

	Projectile *pd = dynamic_cast<Projectile*>(&element->logic());
	pd->direction = camera->projectRay();
	element->rotation(camera->rotation.conjugate());
	element->position(camera->position);
}

void(*submit_for_render)(uint8_t viewport_idx, HgCamera* camera, HgElement* e);

void submit_for_render_threaded(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx) {
	if (s == NULL) {
		hgRenderQueue_push(create_render_packet(NULL, 0, NULL, NULL, 0));
		return;
	}
	HgElement* e = s->get_element(idx);
	hgRenderQueue_push(create_render_packet(e, viewport_idx, camera + 0, s, idx)); //submit to renderer
}

void quick_render(uint8_t viewport_idx, HgCamera* camera, HgElement* e) {
	RenderData* rd = e->renderData();
	//	if (rd->shader) VCALL(rd->shader, enable);
	RENDERER->Viewport(viewport_idx);

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated

	rd->shader->setGlobalUniforms(*camera);
	//	setLocalUniforms(&e->rotation, &e->position, e->scale);

	rd->render();
}

void vertex_print(const vertex* v) {
	printf("%f %f %f\n", v->raw[0], v->raw[1], v->raw[2]);
}

int main()
{
	ENGINE::EnumberateSymbols();

	stereo_view = 0;

	StartWindowSystem();

	if (stereo_view) {
		RENDERER->setup_viewports(1280, 480);
		//		Perspective2(60, 1280.0 / 480.0, 0.1f, 100.0f, projection);
	}
	else {
		RENDERER->setup_viewports(640, 480);
	}

	Perspective2(60, 640.0 / 480.0, 0.1f, 100.0f, projection);

	uint8_t s = sizeof(HgElement);
	printf("element size %d\n", s);
	printf("vertex size %zd\n", sizeof(vertex));
	printf("render_packet size %zd\n", sizeof(render_packet));
	printf("render data size %zd\n", sizeof(RenderData));
	//printf("oglrd size %zd\n", sizeof(OGLRenderData));
//	printf("vbo type size %zd\n", sizeof(VBO_TYPE));
	printf("HgTexture size %zd\n", sizeof(HgTexture));

	//	Perspective(60, 640.0 / 480.0, 0.1f, 100.0f, projection);
	//	Perspective2(60, 640.0/480.0, 0.1f, 100.0f,projection);
	//	Perspective2(60, 320.0 / 480.0, 0.1f, 100.0f, projection);
	using namespace HgMath;
	camera[0].position.z(1.5f);
	camera[0].position.y(2.0f);
	camera[0].SetRotation(quaternion::fromEuler(angle::deg(15), angle::ZERO, angle::ZERO) );

	camera[1] = camera[0];
	camera[1].position.x(camera[1].position.x() + EYE_DISTANCE);

	//	MatrixMultiply4f(projection, camera, view);

	print_matrix(projection);
	printf("\n");

	scene.init();
	//	uint32_t tris[ANI_TRIS];
	HgElement* tris[ANI_TRIS];

	GravityField gravity = { 0 };
	allocate_space(&gravity, ANI_TRIS);
	gravity.scene = &scene;
	gravity.vector.y(-1);
	uint32_t i;
	{
		HgElement* element = NULL;

		if (create_element("triangle", &scene, &element) > 0) {
			auto tmp = element->position();
			tmp.x(1.5f);
			tmp.z(1.0f);
			element->position(tmp);
			//	toQuaternion2(0, 0, 90, &element->rotation);
		}

		if (create_element("triangle", &scene, &element) > 0) {
			auto tmp = element->position();
			tmp.x(0.0f);
			tmp.z(-2.0f);
			element->position(tmp);
			//	toQuaternion2(45,0,0,&element->rotation);
		}


		for (i = 0; i < ANI_TRIS; i++) {
			if (create_element("cube", &scene, &element) > 0) {
				tris[i] = element;
				//			gravity.indices[i] = tris[i];
				//		shape_create_triangle(element);
				float x = (i % 20)*1.1f;
				float z = (i / 20)*1.1f;
				;
				element->position(point(-10.0f + x, 5.0f, -2.0f - z));
				element->scale(0.3f);
				//			element->m_renderData->shader = HGShader_acquire("test_vertex2.glsl", "test_frag2.glsl");
				element->renderData()->shader = HgShader::acquire("basic_light1_v.glsl", "basic_light1_f.glsl");
			}
		}

		scene.getNewElement(&element);
		model_data::load_ini(element, "teapot.ini");
//		element->position.

		if (create_element("voxelGrid", &scene, &element) > 0) {
			//		element->scale = 100.0f;
			element->position().z(-10);
			//		toQuaternion2(0, -90, 0, &element->rotation);
			element->renderData()->shader = HgShader::acquire("basic_light1_v.glsl", "basic_light1_f.glsl");
		}

		if (create_element("square", &scene, &element) > 0) {
			element->scale(100.0f);
			element->position().z(-4);
			element->rotation(quaternion::fromEuler(angle::deg(-90), angle::ZERO, angle::ZERO));
			element->renderData()->shader = HgShader::acquire("grid_vertex.glsl", "grid_frag.glsl");
			element->renderData()->blendMode = BLEND_ADDITIVE;
			//	model_data d = LoadModel("test.hgmdl");
		}
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
	//submit_for_render = submit_for_render_serial;
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

	HgTimer gameTimer;
	HgTime last_time;

	int8_t do_render = 1;
	uint8_t did_change = 0;

	int16_t mouse_x, mouse_y;
	mouse_x = mouse_y = 0;

	gameTimer.start();
	while (1 && !window->m_close) {
		HgTime time = gameTimer.getElasped();
		HgTime dtime = time - last_time;
		last_time = time;

//		if (dtime > 17) {
//			printf("time %d\n", dtime);
//		}
#if (!USE_RENDER_THREAD)
		BeginFrame();
#endif

		if (dtime.msec() > 0) {
			vector3 v;

			if (KeyDownMap['w']) v.z(v.z() - 1.0f);
			if (KeyDownMap['s']) v.z(v.z() + 1.0f);
			if (KeyDownMap['a']) v.x(v.x() - 1.0f);
			if (KeyDownMap['d']) v.x(v.x() + 1.0f);
			if (KeyDownMap['r']) {
				mouse_x = 0; mouse_y = -42;
			}

			if (KeyDownMap[' ']) {
				//				printf("fire!\n");
				fire(&scene);
			}

			//			if (v.components.z > 0) DebugBreak();

			v = v.normal().rotate(camera->rotation.conjugate());
			float scale = (1.0f / 1000.0f) * dtime.msec();
			v = v.normal().scale(scale);
			camera->Move(v);

			mouse_x = (MOUSE_INPUT.dx + mouse_x) % 2000;
			mouse_y = (MOUSE_INPUT.dy + mouse_y) % 2000;

			using namespace HgMath;
			camera->FreeRotate(angle::deg((-MOUSE_INPUT.dx / 2000.0f) * 360), angle::deg((-MOUSE_INPUT.dy / 2000.0f) * 360));

			MOUSE_INPUT.dx = 0;
			MOUSE_INPUT.dy = 0;
		}

		if (needRender > 0) {
			do_render = needRender;
			needRender = 0;
		}

		//if (time > 10000) { // && did_change==0) {
		//	did_change = 1;

		//	//			if (dtime>0) gravity_update(&gravity, dtime);
		//	/*
		//	for (i = 0; i < ANI_TRIS; i++) {
		//	change_to_triangle(tris[i]);
		//	}
		//	*/
		//}

		//		printf("dtime: %d\n", dtime);

		{
			HgElement* element = tris[0];
			element->rotation(quaternion::fromEuler(angle::ZERO,angle::deg((time.msec() % 10000) / 27.777777777777777777777777777778), angle::ZERO));

			for (i = 0; i < ANI_TRIS; i++) {
				tris[i]->rotation(element->rotation());
			}
		}

		camera[1] = camera[0];
		camera[1].position.x(camera[1].position.x()+EYE_DISTANCE);

		scene.update(dtime);

		Renderer::opaqueElements.clear();
		Renderer::transparentElements.clear();

		uint32_t maxCount = scene.maxItems();
		for (uint32_t i = 0; i < maxCount; ++i) {
			if (!scene.isUsed(i)) continue;
			HgElement* e = scene.get_element(i);
			if (!e->flags.hidden) {
				Renderer::Enqueue(*e);
			}
		}

		Engine::EnqueueForRender(Engine::collections());

		//render below
		Renderer::Render(stereo_view, camera+0);
		if (stereo_view>0) Renderer::Render(2, camera+1);

		window->SwapBuffers();

		InterlockedAdd(&itrctr, 1);

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

