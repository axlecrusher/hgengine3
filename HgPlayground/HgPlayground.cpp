// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <HgEngine.h>
#include <Win32Window.h>

#include <stdio.h>
#include <stdlib.h>

#include <HgEntity.h>
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

#include <HgSoundDriver.h>
#include <IRenderTarget.h>
#include <WindowRenderTarget.h>

#include <core/HgScene2.h>

float projection[16];

HgCamera camera;

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

//DWORD WINAPI StartRenderThread(LPVOID lpParam) {
//	ENGINE::StartWindowSystem();
//	return RenderThreadLoop();
//}

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

//void fire(HgScene* scene) {
//	HgEntity* entity = NULL;
//
//	create_entity("basic_projectile", scene, &entity);
//
//	Projectile *pd = dynamic_cast<Projectile*>(&entity->logic());
//	pd->direction = camera.getForward();
//	entity->orientation(camera.getWorldSpaceOrientation().conjugate());
//	entity->position(camera.getWorldSpacePosition());
//}

void(*submit_for_render)(uint8_t viewport_idx, HgCamera* camera, HgEntity* e);

//void submit_for_render_threaded(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx) {
//	if (s == NULL) {
//		hgRenderQueue_push(create_render_packet(NULL, 0, NULL, NULL, 0));
//		return;
//	}
//	HgEntity* e = s->get_entity(idx);
//	hgRenderQueue_push(create_render_packet(e, viewport_idx, camera + 0, s, idx)); //submit to renderer
//}

void vertex_print(const vertex* v) {
	printf("%f %f %f\n", v->raw[0], v->raw[1], v->raw[2]);
}

int main()
{
	using namespace ENGINE::INPUT;

	ENGINE::InitEngine();

	std::unique_ptr<IRenderTarget> renderTarget;
	renderTarget = std::make_unique<WindowRenderTarget>();
	renderTarget->Init();

	auto window = MercuryWindow::GetCurrentWindow();

	SOUND = HgSound::Create();
	if (SOUND->init())
	{
		SOUND->start();
	}

	uint8_t s = sizeof(HgEntity);
	printf("entity size %d\n", s);
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
	const auto position = camera.getWorldSpacePosition().z(1.5f).y(2.0f);
	camera.setWorldSpacePosition(position);
	camera.setWorldSpaceOrientation(quaternion::fromEuler(angle::deg(15), angle::ZERO, angle::ZERO) );

	print_matrix(projection);
	printf("\n");

	scene.init();

	Engine::HgScene scene2;

	GravityField gravity = { 0 };
	allocate_space(&gravity, ANI_TRIS);
	gravity.scene = &scene;
	gravity.vector.y(-1);
	
	HgEntity* teapot = NULL;
	scene.getNewEntity(&teapot);
	model_data::load_ini(teapot, "teapot.ini");
	teapot->origin(teapot->origin().x(2).z(3).y(1));
	
	//for (int i = 0; i < 4; ++i) {
	//	HgEntity* statue = NULL;
	//	scene.getNewEntity(&statue);
	//	model_data::load_ini(statue, "statue.ini");
	//	statue->origin(statue->origin().x(2).z(3).y(1));
	//	statue->position(statue->position().x(i));
	//}

	EntityIdType cubeId;
	uint32_t i;
	{
		HgEntity* entity = NULL;

		{
			HgEntity* entity = scene2.create_entity("triangle");
			if (entity) {
				const auto tmp = entity->position();
				entity->position(tmp.x(1.5f).z(1.0f));
				entity->renderData()->getMaterial().setShader(HgShader::acquire("test_vertex_instanced.glsl", "test_frag_instanced.glsl"));
				//	toQuaternion2(0, 0, 90, &entity->orientation);
			}

			entity = scene2.create_entity("triangle");
			if (entity) {
				const auto tmp = entity->position();
				entity->position(tmp.x(0.0f).z(-2.0f));
				entity->renderData()->getMaterial().setShader(HgShader::acquire("test_vertex_instanced.glsl", "test_frag_instanced.glsl"));
				//	toQuaternion2(45,0,0,&entity->orientation);
			}
		}

		for (i = 0; i < ANI_TRIS; i++) {
			float x = (i % 20)*1.1f;
			float z = (i / 20)*1.1f;

			HgEntity* entity = scene2.create_entity("rotating_cube");
			if (entity)
			{
				entity->position(point(-10.0f + x, 5.0f, -2.0f - z));
				entity->scale(0.3f);
				auto rd = entity->renderData();
				rd->getMaterial().setShader(HgShader::acquire("basic_light2_v_instance.glsl", "basic_light2_f2.glsl"));
				cubeId = entity->getEntityId();
			}
		}

		auto redCube = scene2.create_entity("cube");
		if (redCube) {
			redCube->position(point(2,2,-2));
			redCube->scale(1.3f);
			redCube->renderData()->getMaterial().setShader(HgShader::acquire("basic_light2_v_instance.glsl", "basic_light2_f_red.glsl"));
			//entity->renderData()->shader = HgShader::acquire("test_matrix_v.glsl", "test_matrix_f.glsl");
		}

//		entity->position.

		if (Engine::create_entity("voxelGrid", &scene, &entity) > 0) {
			//		entity->scale = 100.0f;
			entity->position().z(-10);
			//		toQuaternion2(0, -90, 0, &entity->orientation);
			entity->renderData()->getMaterial().setShader(HgShader::acquire("basic_light1_v.glsl", "basic_light1_f.glsl"));
		}
	}

	HgEntity* grid = nullptr;
	if (Engine::create_entity("square", &scene, &grid) > 0) {
		using namespace HgMath;
		grid->scale(100.0f);
		grid->position().z(-4);
		grid->orientation(quaternion::fromEuler(angle::deg(-90), angle::ZERO, angle::ZERO));
		grid->renderData()->getMaterial().setShader(HgShader::acquire("grid_vertex.glsl", "grid_frag.glsl"));
		grid->renderData()->getMaterial().setBlendMode(BLEND_ADDITIVE);
		grid->renderData()->getMaterial().setTransparent(true);
		grid->renderData()->renderFlags.DEPTH_WRITE = false;
		grid->setName("Grid");
		grid->setDrawOrder(100);
		//grid->flags.deptj
		//	model_data d = LoadModel("test.hgmdl");
	}

	auto testSound = HgSound::SoundAsset::acquire("tone.wav");
	auto snd = testSound->newPlayingInstance();
	snd->setVolume(0.5);

	HgEntity* teapotSquare = scene2.create_entity("cube");
	if (teapotSquare > 0) {
		teapotSquare->scale(0.3);
		teapotSquare->position(point(0, 3, 0));
		teapotSquare->setInheritParentScale(false);
		teapotSquare->setParent(teapot);
		teapotSquare->renderData()->getMaterial().setShader(HgShader::acquire("basic_light2_v_instance.glsl", "basic_light2_f_blue.glsl"));
		//teapotSquare->setHidden(true);
		SOUND->play3d(snd, snd->EmitFromEntity(teapotSquare));
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

	RenderQueue renderQueue;

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


				if (KeyDownMap[KeyCodes::KEY_SPACE]) {
					snd->stop();
				}

				//			if (v.components.z > 0) DebugBreak();

				v = v.normal().rotate(camera.getWorldSpaceOrientation());
				float scale = (1.0f / 1000.0f) * timeStep.msec();
				v = v.normal().scale(scale);
				camera.move(v);

				mouse_x = (MOUSE_INPUT.dx + mouse_x) % 2000;
				mouse_y = (MOUSE_INPUT.dy + mouse_y) % 2000;

				using namespace HgMath;
				camera.FreeRotate(angle::deg((MOUSE_INPUT.dx / 2000.0f) * 360), angle::deg((MOUSE_INPUT.dy / 2000.0f) * 360));

				if (KeyDownMap[KeyCodes::KEY_R]) {
					auto q = quaternion::fromEuler(angle::deg(0), angle::deg(0), angle::deg(0));
					camera.setWorldSpaceOrientation(q);
					point zero(0,1,0);
					camera.setWorldSpacePosition(zero);
				}
				MOUSE_INPUT.dx = 0;
				MOUSE_INPUT.dy = 0;
			}

			grid->position(point(camera.getWorldSpacePosition()).y(0));

			const auto orientation = quaternion::fromEuler(angle::ZERO, angle::deg((time.msec() % 10000) / 27.777777777777777777777777777778), angle::ZERO);
			teapot->orientation(orientation.conjugate());

			scene.update(timeStep);
			scene2.update(timeStep);
			Engine::updateCollections(timeStep);
		}

		SOUND->update();

		doRender = true;
		if (doRender) {
			BeginFrame();

			HgSound::Listener listener;
			listener.setPosition(camera.getWorldSpacePosition());
			listener.setForward(camera.getForward());
			listener.setUp(camera.getUp());
			SOUND->setListener(listener);

			renderQueue.Clear();
			scene.EnqueueForRender(&renderQueue);
			scene2.EnqueueForRender(&renderQueue);
			//Engine::EnqueueForRender(Engine::collections(), &renderQueue);
			renderQueue.Finalize();

			renderTarget->Render(&camera, &renderQueue);

			window->SwapBuffers();
			InterlockedAdd(&itrctr, 1);
			doRender = false;
		}
	}

	return 0;
}

