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

#include <HgSoundDriver.h>

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
	HgEntity* entity = NULL;

	create_entity("basic_projectile", scene, &entity);

	Projectile *pd = dynamic_cast<Projectile*>(&entity->logic());
	pd->direction = camera->getForward();
	entity->orientation(camera->getWorldSpaceOrientation().conjugate());
	entity->position(camera->getWorldSpacePosition());
}

void(*submit_for_render)(uint8_t viewport_idx, HgCamera* camera, HgEntity* e);

void submit_for_render_threaded(uint8_t viewport_idx, HgCamera* camera, HgScene *s, uint32_t idx) {
	if (s == NULL) {
		hgRenderQueue_push(create_render_packet(NULL, 0, NULL, NULL, 0));
		return;
	}
	HgEntity* e = s->get_entity(idx);
	hgRenderQueue_push(create_render_packet(e, viewport_idx, camera + 0, s, idx)); //submit to renderer
}

void quick_render(uint8_t viewport_idx, HgCamera* camera, HgEntity* e) {
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
	const auto position = camera[0].getWorldSpacePosition().z(1.5f).y(2.0f);
	camera[0].setWorldSpacePosition(position);
	camera[0].setWorldSpaceOrientation(quaternion::fromEuler(angle::deg(15), angle::ZERO, angle::ZERO) );

	{
		camera[1] = camera[0];
		auto tmp = camera[1].getWorldSpacePosition();
		tmp.x(tmp.x() + EYE_DISTANCE); //i don't think this is correct. I think that you have to rotate a (1,0,0) vector by the orientation and then scale by eye distance and add to position
		camera[1].setWorldSpacePosition(tmp);
	}

	//	MatrixMultiply4f(projection, camera, view);

	print_matrix(projection);
	printf("\n");

	scene.init();
	//	uint32_t tris[ANI_TRIS];
	HgEntity* tris[ANI_TRIS];

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

	uint32_t i;
	{
		HgEntity* entity = NULL;

		if (create_entity("triangle", &scene, &entity) > 0) {
			const auto tmp = entity->position();
			entity->position(tmp.x(1.5f).z(1.0f));
			//	toQuaternion2(0, 0, 90, &entity->orientation);
		}

		if (create_entity("triangle", &scene, &entity) > 0) {
			const auto tmp = entity->position();
			entity->position(tmp.x(0.0f).z(-2.0f));
			//	toQuaternion2(45,0,0,&entity->orientation);
		}

		for (i = 0; i < ANI_TRIS; i++) {
			if (create_entity("cube", &scene, &entity) > 0) {
				tris[i] = entity;
				//			gravity.indices[i] = tris[i];
				//		shape_create_triangle(element);
				float x = (i % 20)*1.1f;
				float z = (i / 20)*1.1f;
				;
				entity->position(point(-10.0f + x, 5.0f, -2.0f - z));
				entity->scale(0.3f); 
				entity->renderData()->shader = HgShader::acquire("basic_light2_v.glsl", "basic_light2_f.glsl");
				//entity->renderData()->shader = HgShader::acquire("test_matrix_v.glsl", "test_matrix_f.glsl");
			}
		}

		if (create_entity("cube", &scene, &entity) > 0) {
			entity->position(point(2,2,-2));
			entity->scale(0.3f);
			entity->renderData()->shader = HgShader::acquire("basic_light2_v.glsl", "basic_light2_f.glsl");
			//entity->renderData()->shader = HgShader::acquire("test_matrix_v.glsl", "test_matrix_f.glsl");
		}

//		entity->position.

		if (create_entity("voxelGrid", &scene, &entity) > 0) {
			//		entity->scale = 100.0f;
			entity->position().z(-10);
			//		toQuaternion2(0, -90, 0, &entity->orientation);
			entity->renderData()->shader = HgShader::acquire("basic_light1_v.glsl", "basic_light1_f.glsl");
		}
	}

	HgEntity* grid = NULL;
	if (create_entity("square", &scene, &grid) > 0) {
		grid->scale(100.0f);
		//entity->position().z(-4);
		grid->orientation(quaternion::fromEuler(angle::deg(-90), angle::ZERO, angle::ZERO));
		grid->renderData()->shader = HgShader::acquire("grid_vertex.glsl", "grid_frag.glsl");
		grid->renderData()->blendMode = BLEND_ADDITIVE;
		//	model_data d = LoadModel("test.hgmdl");
	}


	HgEntity* teapotSquare = NULL;
	if (create_entity("cube", &scene, &teapotSquare) > 0) {
		teapotSquare->scale(0.3);
		teapotSquare->position(point(0, 3, 0));
		teapotSquare->inheritParentScale(false);
		teapotSquare->setParent(teapot);
	}

	auto testSound = HgSound::SoundAsset::acquire("tone.wav");
	auto snd = testSound->newPlayingInstance();
	snd->setVolume(0.5);
	HgSound::Emitter emitter;
	emitter.setPosition(teapotSquare->position());
	SOUND->play3d(snd, emitter);

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


				if (KeyDownMap[KeyCodes::KEY_SPACE]) {
					snd->stop();
				}

				//			if (v.components.z > 0) DebugBreak();

				v = v.normal().rotate(camera->getWorldSpaceOrientation());
				float scale = (1.0f / 1000.0f) * timeStep.msec();
				v = v.normal().scale(scale);
				camera[0].move(v);

				mouse_x = (MOUSE_INPUT.dx + mouse_x) % 2000;
				mouse_y = (MOUSE_INPUT.dy + mouse_y) % 2000;

				using namespace HgMath;
				camera->FreeRotate(angle::deg((MOUSE_INPUT.dx / 2000.0f) * 360), angle::deg((MOUSE_INPUT.dy / 2000.0f) * 360));

				if (KeyDownMap[KeyCodes::KEY_R]) {
					auto q = quaternion::fromEuler(angle::deg(0), angle::deg(0), angle::deg(0));
					camera->setWorldSpaceOrientation(q);
					point zero(0,1,0);
					camera->setWorldSpacePosition(zero);
				}
				MOUSE_INPUT.dx = 0;
				MOUSE_INPUT.dy = 0;
			}

			grid->position(point(camera->getWorldSpacePosition()).y(0));

			{
				HgEntity* entity = tris[0];
				const auto orientation = quaternion::fromEuler(angle::ZERO, angle::deg((time.msec() % 10000) / 27.777777777777777777777777777778), angle::ZERO);
				entity->orientation(orientation);
				teapot->orientation(orientation.conjugate());

				HgSound::Emitter emitter = snd->getEmitter();
				emitter.setPosition(teapotSquare->computeWorldSpacePosition());
				snd->setEmitter(emitter);


				for (i = 0; i < ANI_TRIS; i++) {
					tris[i]->orientation(entity->orientation());
				}
			}

			scene.update(timeStep);

		}

		doRender = true;
		if (doRender) {
			BeginFrame();

			camera[1] = camera[0];
//			camera[1].position.x(camera[1].position.x() - EYE_DISTANCE * 0.5f);
			camera[2] = camera[0];
//			camera[2].position.x(camera[1].position.x() + EYE_DISTANCE * 0.5f);

			HgSound::Listener listener;
			listener.setPosition(camera[0].getWorldSpacePosition());
			listener.setForward(camera[0].getForward());
			listener.setUp(camera[0].getUp());
			SOUND->setListener(listener);

			Renderer::opaqueEntities.clear();
			Renderer::transparentEntities.clear();

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

