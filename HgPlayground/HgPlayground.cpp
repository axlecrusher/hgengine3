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
#include <InstancedCollection.h>

float projection[16];

extern viewport view_port[];
HgCamera camera[3];

HANDLE endOfRenderFrame = NULL;

#define EYE_DISTANCE -0.07f

#define USE_RENDER_THREAD 0

void BeginFrame() {
	ENGINE::INPUT::PumpMessages();
	RENDERER()->Clear();
	RENDERER()->BeginFrame();
}

int32_t RenderThreadLoop() {
	MercuryWindow* w = MercuryWindow::GetCurrentWindow();

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
	ENGINE::StartWindowSystem();
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
	element->orientation(camera->orientation.conjugate());
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
	float wsm[16];
	RenderData* rd = e->renderData();
	//	if (rd->shader) VCALL(rd->shader, enable);
	RENDERER()->Viewport(viewport_idx);

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated
	e->computeWorldSpaceMatrix().store(wsm);
	rd->shader->uploadMatrices(wsm, Renderer::ProjectionMatrix, Renderer::ViewMatrix);
	//	setLocalUniforms(&e->orientation, &e->position, e->scale);

	rd->render();
}

void vertex_print(const vertex* v) {
	printf("%f %f %f\n", v->raw[0], v->raw[1], v->raw[2]);
}

int main()
{
	using namespace ENGINE::INPUT;

	stereo_view = false;

	ENGINE::InitEngine();
	ENGINE::StartWindowSystem();

	auto window = MercuryWindow::GetCurrentWindow();

	int width = window->CurrentWidth();
	int height = window->CurrentHeight();

	if (stereo_view) {
		RENDERER()->setup_viewports(width, height);
		double renderWidth = width / 2.0;
		double renderHeight = height;
		Perspective2(60, renderWidth / renderHeight, 0.1f, 100.0f, projection);
	}
	else {
		RENDERER()->setup_viewports(width, height);
		double renderWidth = width;
		double renderHeight = height;
		double aspect = renderWidth / renderHeight;
		Perspective2(60, aspect, 0.1f, 100.0f, projection);
		//auto tmp = HgMath::mat4f::perspective(60*DEG_RAD, aspect, 0.1f, 100.0f);
		//tmp.store(projection);
	}

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
	camera[0].setOrientation(quaternion::fromEuler(angle::deg(15), angle::ZERO, angle::ZERO) );

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
	
	HgElement* teapot = NULL;
	scene.getNewElement(&teapot);
	model_data::load_ini(teapot, "teapot.ini");
	teapot->origin(teapot->origin().x(2).z(3).y(1));
	
	HgElement* statue = NULL;
	scene.getNewElement(&statue);
	model_data::load_ini(statue, "statue.ini");
	statue->origin(statue->origin().x(2).z(3).y(1));

	uint32_t i;
	{
		HgElement* element = NULL;

		if (create_element("triangle", &scene, &element) > 0) {
			auto tmp = element->position();
			tmp.x(1.5f);
			tmp.z(1.0f);
			element->position(tmp);
			//	toQuaternion2(0, 0, 90, &element->orientation);
		}

		if (create_element("triangle", &scene, &element) > 0) {
			auto tmp = element->position();
			tmp.x(0.0f);
			tmp.z(-2.0f);
			element->position(tmp);
			//	toQuaternion2(45,0,0,&element->orientation);
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
				element->renderData()->shader = HgShader::acquire("basic_light2_v.glsl", "basic_light2_f.glsl");
				//element->renderData()->shader = HgShader::acquire("test_matrix_v.glsl", "test_matrix_f.glsl");
			}
		}

//		element->position.

		if (create_element("voxelGrid", &scene, &element) > 0) {
			//		element->scale = 100.0f;
			element->position().z(-10);
			//		toQuaternion2(0, -90, 0, &element->orientation);
			element->renderData()->shader = HgShader::acquire("basic_light1_v.glsl", "basic_light1_f.glsl");
		}
	}

	HgElement* grid = NULL;
	if (create_element("square", &scene, &grid) > 0) {
		grid->scale(100.0f);
		//element->position().z(-4);
		grid->orientation(quaternion::fromEuler(angle::deg(-90), angle::ZERO, angle::ZERO));
		grid->renderData()->shader = HgShader::acquire("grid_vertex.glsl", "grid_frag.glsl");
		grid->renderData()->blendMode = BLEND_ADDITIVE;
		//	model_data d = LoadModel("test.hgmdl");
	}


	{
		HgElement* element = NULL;
		if (create_element("cube", &scene, &element) > 0) {
			element->scale(0.3);
			element->position(point(0, 3, 0));
			element->inheritParentScale(false);
			element->setParent(teapot);
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

	HgTime accumulatedTime;
	const HgTime timeStep = HgTime::msec(8); // about 120 ups

	gameTimer.start();
	while (!window->m_close) {
		const HgTime time = gameTimer.getElasped();
		accumulatedTime += time - last_time;
		last_time = time;

		if (accumulatedTime.msec() > 100) {
			accumulatedTime = HgTime::msec(100);
		}

		bool doRender = false;

		//update loop
		while (accumulatedTime >= timeStep) {
			accumulatedTime -= timeStep;
			doRender = true;

			if (timeStep.msec() > 0) {
				vector3 v;

				if (KeyDownMap[KeyCodes::KEY_W]) v.z(v.z() - 1.0f);
				if (KeyDownMap[KeyCodes::KEY_S]) v.z(v.z() + 1.0f);
				if (KeyDownMap[KeyCodes::KEY_A]) v.x(v.x() - 1.0f);
				if (KeyDownMap[KeyCodes::KEY_D]) v.x(v.x() + 1.0f);
				if (KeyDownMap[KeyCodes::KEY_R]) {
					mouse_x = 0; mouse_y = -42;
				}

				if (KeyDownMap[KeyCodes::KEY_SPACE]) {
					//				printf("fire!\n");
					//fire(&scene);
				}

				//			if (v.components.z > 0) DebugBreak();

				v = v.normal().rotate(camera->orientation.conjugate());
				float scale = (1.0f / 1000.0f) * timeStep.msec();
				v = v.normal().scale(scale);
				camera->move(v);

				mouse_x = (MOUSE_INPUT.dx + mouse_x) % 2000;
				mouse_y = (MOUSE_INPUT.dy + mouse_y) % 2000;

				using namespace HgMath;
				camera->FreeRotate(angle::deg((-MOUSE_INPUT.dx / 2000.0f) * 360), angle::deg((-MOUSE_INPUT.dy / 2000.0f) * 360));

				MOUSE_INPUT.dx = 0;
				MOUSE_INPUT.dy = 0;
			}

			grid->position(point(camera->position).y(0));

			{
				HgElement* element = tris[0];
				const auto orientation = quaternion::fromEuler(angle::ZERO, angle::deg((time.msec() % 10000) / 27.777777777777777777777777777778), angle::ZERO);
				element->orientation(orientation);
				teapot->orientation(orientation.conjugate());

				for (i = 0; i < ANI_TRIS; i++) {
					tris[i]->orientation(element->orientation());
				}
			}

			scene.update(timeStep);

		}

		if (doRender) {
			BeginFrame();

			camera[1] = camera[0];
			camera[1].position.x(camera[1].position.x() - EYE_DISTANCE * 0.5f);
			camera[2] = camera[0];
			camera[2].position.x(camera[1].position.x() + EYE_DISTANCE * 0.5f);

			Renderer::opaqueElements.clear();
			Renderer::transparentElements.clear();

			scene.EnqueueForRender();
			Engine::EnqueueForRender(Engine::collections());

			//render below
			const auto projection_matrix = HgMath::mat4f(projection);

			if (stereo_view) {
				Renderer::Render(1, camera + 1, projection_matrix); //eye 1
				Renderer::Render(2, camera + 2, projection_matrix); //eye 2
			}
			else
			{
				Renderer::Render(0, camera, projection_matrix);
			}

			window->SwapBuffers();
			InterlockedAdd(&itrctr, 1);
			doRender = false;
		}
	}

	return 0;
}

