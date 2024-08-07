// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <HgEngine.h>
#include <Win32Window.h>
#include <Logging.h>

#include <stdio.h>
#include <stdlib.h>

#include <HgEntity.h>
//#include <HgScene.h>
//#include <shapes.h>

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

#include <triangle.h>
#include <TBNVisualization.h>
#include <PointCloud.h>

#include <OpenVr.h>
#include <OpenVRRenderTarget.h>
#include <Player.h>

#include <HgFreetype.h>

#include <triangle.h>
#include <cube.h>
#include <MeshMath.h>

float projection[16];

HgCamera camera;

HANDLE endOfRenderFrame = NULL;

#define USE_RENDER_THREAD 0

int32_t RenderThreadLoop() {
	MercuryWindow* w = MercuryWindow::GetCurrentWindow();

	uint8_t stop_frame = 0;
	while (1) {
		//BeginFrame();

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
//HgScene scene;

DWORD WINAPI PrintCtr(LPVOID lpParam) {
	while (1) {
		printf("UPS %u e_count %d\n", itrctr, EntityIdTable::Singleton().numberOfEntitiesExisting());
		itrctr = 0;
		CheckFilesForChange();
		Sleep(1000);
	}
}

#define CUBE_COUNT 4000
//#define CUBE_COUNT 150000

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

	OpenVrProxy openVr;
	bool inVr = false;
	//inVr = openVr.Init();

	std::unique_ptr<IRenderTarget> renderTarget;

	if (inVr)
	{
		renderTarget = std::make_unique<OpenVRRenderTarget>(&openVr);
	}
	else
	{
		renderTarget = std::make_unique<WindowRenderTarget>();
	}
	renderTarget->Init();

	auto window = MercuryWindow::GetCurrentWindow();

	SOUND = HgSound::Create();
	if (SOUND->init())
	{
		SOUND->start();
	}

	uint8_t s = sizeof(HgEntity);
	printf("entity size %d\n", s);
	printf("entity SpacialData %d\n", sizeof(SpacialData));
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

	//scene.init();

	Player PLAYER1;
	PLAYER1.entity().init();

	PLAYER1.setPosition({ 0, 1.76784, 0 });

	EventSystem::Register<Events::HMDPoseUpdated>(&PLAYER1, [&PLAYER1](const Events::HMDPoseUpdated& evt) {
		PLAYER1.setOrientation(evt.orientation);
		PLAYER1.setPosition(evt.position);
	});

	Engine::HgScene scene2;
	Engine::HgScene scene_2d;

	//constexpr auto pathCount = 20;
	//vertex3f polyBoardLine[pathCount];
	//auto pb = scene2.create_entity<HgModel>();

	//{
	//	for (int i = 0; i < pathCount; i++)
	//	{
	//		polyBoardLine[i].x(i*0.25);
	//		polyBoardLine[i].y(i*0.4);
	//		polyBoardLine[i].z(-i*i*0.125);
	//	}

	//	constexpr auto vertexCount = MeshMath::computePolyboardVertexCount(pathCount);
	//	vertex3f polyBoardMesh[vertexCount];

	//	constexpr auto indexCount = MeshMath::computePolyboardIndexCount(pathCount);
	//	uint32_t polyBoardIndices[indexCount];
	//	MeshMath::generatePolyboard(polyBoardLine, pathCount, PLAYER1.position(), 1, polyBoardMesh, polyBoardIndices);

	//	auto rd = RenderData::Create();
	//	rd->VertexVboRecord( HgVbo::GenerateUniqueFrom(polyBoardMesh, vertexCount) );
	//	rd->indexVboRecord( HgVbo::GenerateUniqueFrom(polyBoardIndices, indexCount) );
	//	//rd->renderFlags.BACKFACE_CULLING = false;

	//	pb->getEntity().setRenderData(rd);
	//}

	EntityIdType teapotId;
	{
		auto idList = EntityHelpers::createContiguous(1);
		scene2.addEntityIDs(idList);
		teapotId = idList[0];
		auto teapot = EntityTable::Singleton().getPtr(teapotId);
		model_data::load_ini(teapot, "teapot.ini");
		teapot->origin(teapot->origin().x(2).z(3).y(1));
	}

	//{
	//	//tbn doesn't really work on the teapot because it doesn't have UV
	//	auto tbn = scene2.create_entity<TBNVisualization::TBNVisualization>();
	//	tbn->getEntity().setParent(teapot);
	//	tbn->buildFromVertexData(teapot->getRenderDataPtr()->VertexVboRecord());
	//}


	//HgEntity* gun = NULL;
	//scene.getNewEntity(&gun);
	//model_data::load_ini(gun, "gun.ini");

	//{
	//	auto tbn = scene2.create_entity<TBNVisualization::TBNVisualization>();
	//	tbn->buildFromVertexData(gun->getRenderDataPtr()->VertexVboRecord());
	//	tbn->getEntity().setParent(gun);
	//	tbn->getEntity().setHidden(true);
	//}
	//
	//{
	//	auto pointCloud = scene2.create_entity<PointCloud::PointCloud>();
	//	pointCloud->buildFromVertexData(gun->getRenderDataPtr()->VertexVboRecord());
	//	pointCloud->getEntity().setParent(gun);
	//	pointCloud->getEntity().setHidden(true);
	//}
	{
		auto textIDs = EntityHelpers::createContiguous(4);
		{
			auto textEntity = EntityTable::Singleton().getPtr(textIDs[0]);
			scene2.addEntityID(textIDs[0]);

			HgText::Text t;
			t.setText("HgEngine 3 -- Now with text! VV");
			textEntity->setRenderData(t.CreateRenderData());
			EntityHelpers::setDrawOrder(textIDs[0], 110);
			textEntity->position(point(1, 0, 0));
			textEntity->scale(10.0);
			textEntity->getRenderDataPtr()->renderFlags.BACKFACE_CULLING = false;
		}

		{
			auto textEntity = EntityTable::Singleton().getPtr(textIDs[1]);
			scene2.addEntityID(textIDs[1]);

			HgText::Text t;
			t.setText("Woooooooooo!");
			textEntity->setRenderData(t.CreateRenderData());
			EntityHelpers::setDrawOrder(textIDs[1], 110);
			textEntity->position(point(1, 2, 0));
			textEntity->scale(10.0);
			textEntity->getRenderDataPtr()->renderFlags.BACKFACE_CULLING = false;
			RotatingCube::initRotatingCube(textIDs[1]);
		}

		{
			auto textEntity = EntityTable::Singleton().getPtr(textIDs[2]);
			scene_2d.addEntityID(textIDs[2]);

			HgText::Text t;
			t.setText("Mercury Engine 3");
			textEntity->setRenderData(t.CreateRenderData());
			textEntity->position(point(-0.99, 0.95, 0));
			//text.getEntity().getRenderDataPtr()->renderFlags.BACKFACE_CULLING = false;
		}

		{
			auto textEntity = EntityTable::Singleton().getPtr(textIDs[3]);
			scene_2d.addEntityID(textIDs[3]);

			HgText::Text t;
			t.setText("https://github.com/axlecrusher/hgengine3");
			textEntity->setRenderData(t.CreateRenderData());
			textEntity->position(point(-0.99, -0.98, 0));
			textEntity->scale(0.75);
		}
	}

	//{
	//	auto statueList = EntityHelpers::createContiguous(4);
	//	scene2.addEntityIDs(statueList);
	//	auto statueSource = EntityTable::Singleton().getPtr(statueList[0]);
	//	model_data::load_ini(statueSource, "statue.ini");
	//	for (int i = 0; i < 4; ++i)
	//	{
	//		auto statue = EntityTable::Singleton().getPtr(statueList[i]);
	//		statueSource->clone(statue);
	//		statue->origin(statue->origin().x(2).z(3).y(1));
	//		statue->position(statue->position().x(i));
	//	}
	//}

	//{
	//	auto eratoList = EntityHelpers::createContiguous(1);
	//	scene2.addEntityIDs(eratoList);
	//	auto statueSource = EntityTable::Singleton().getPtr(eratoList[0]);
	//	model_data::load_ini(statueSource, "erato.ini");
	//	for (int i = 0; i < 1; ++i)
	//	{
	//		auto statue = EntityTable::Singleton().getPtr(eratoList[i]);
	//		statueSource->clone(statue);
	//		statue->origin(statue->origin().x(2).z(3).y(1));
	//		statue->position(statue->position().x(i));
	//	}
	//}

	//for (int i = 0; i < 4; ++i) {
	//	HgEntity* statue = NULL;
	//	scene.getNewEntity(&statue);
	//	model_data::load_ini(statue, "statue.ini");
	//	statue->origin(statue->origin().x(2).z(3).y(1));
	//	statue->position(statue->position().x(i));
	//}

	//EntityIdType cubeId;
	//uint32_t i;
	{
		HgEntity* entity = NULL;

		{
			//Create a couple of simple triangle entites
			auto triangleList = EntityHelpers::createContiguous(2);
			scene2.addEntityIDs(triangleList);
			Triangle::init(triangleList);

			{
				auto entity = EntityTable::Singleton().getPtr(triangleList[0]);
				const auto tmp = entity->position();
				entity->position(tmp.x(1.5f).z(-1.0f));
			}


			{
				auto entity = EntityTable::Singleton().getPtr(triangleList[1]);
				const auto tmp = entity->position();
				entity->position(tmp.x(0.0f).z(-2.5f));
			}
		}

		//Create a lot of rotating cubes
		auto idList = EntityHelpers::createContiguous(CUBE_COUNT);
		scene2.addEntityIDs(idList);
		Cube::changeToCube(idList);
		RotatingCube::initRotatingCubes(idList);

		uint32_t i = 0;
		EntityIdType cubeId;
		for (auto id : idList)
		{
			float x = (i % 40)*1.1f;
			float z = (i / 40)*1.1f;

			auto cube = EntityTable::Singleton().getPtr(id);

			//if (entity)
			{
				cube->position(point(-20.0f + x, 5.0f, 30.0f - z));
				cube->scale(0.3f);

				if (!EntityIdTable::Singleton().exists(cubeId))
				{
					auto rd = cube->getRenderDataPtr();
					rd->getMaterial().setShader(HgShader::acquire("basic_light2_v_instance.glsl", "basic_light2_f2.glsl"));
				}
				else
				{
					auto r = HgEntity::Find(cubeId);
					if (r.isValid())
					{
						auto rd = r.entity->getRenderDataPtr();
						cube->setRenderData(rd); //instance it
					}
				}
				cubeId = id;
			}

			i++;
		}

//
////		entity->position.
//
//		if (Engine::create_entity("voxelGrid", &scene, &entity) > 0) {
//			//		entity->scale = 100.0f;
//			entity->position().z(-10);
//			//		toQuaternion2(0, -90, 0, &entity->orientation);
//			entity->getRenderDataPtr()->getMaterial().setShader(HgShader::acquire("basic_light1_v.glsl", "basic_light1_f.glsl"));
//		}
	}

	//Create a lot of rotating cubes
	auto idList = EntityHelpers::createContiguous(1);
	EntityIdType gridId = idList[0];

	if (Engine::create_entity("square", gridId))
	{
		using namespace HgMath;
		scene2.addEntityIDs(idList);
		HgEntity* grid = EntityTable::Singleton().getPtr(gridId);
		grid->scale(100.0f);
		grid->position().z(-4);
		grid->orientation(quaternion::fromEuler(angle::deg(-90), angle::ZERO, angle::ZERO));
		grid->getRenderDataPtr()->getMaterial().setShader(HgShader::acquire("assets/shaders/grid.vert", "assets/shaders/grid.frag"));
		grid->getRenderDataPtr()->getMaterial().setBlendMode(BLEND_ADDITIVE);
		grid->getRenderDataPtr()->getMaterial().setTransparent(true);
		grid->getRenderDataPtr()->renderFlags.DEPTH_WRITE = false;
		grid->setName("Grid");
		EntityHelpers::setDrawOrder(grid->getEntityId(), 100);
		//grid->flags.deptj
		//	model_data d = LoadModel("test.hgmdl");
	}

	{
		auto testSound = HgSound::SoundAsset::acquire("assets/music/crush_mono.ogg");
		auto snd = testSound->newPlayingInstance();
		snd->setVolume(0.75);

		{
			auto idList = EntityHelpers::createContiguous(2);
			scene2.addEntityIDs(idList);

			auto& et = EntityTable::Singleton();

			HgEntity* redCube = et.getPtr(idList[0]);
			change_to_cube(redCube);

			redCube->setRenderData(RenderData::Create(redCube->getRenderDataPtr()));
			redCube->position(point(2, 2, -2));
			redCube->scale(1.3f);
			redCube->getRenderDataPtr()->getMaterial().setShader(HgShader::acquire("basic_light2_v_instance.glsl", "basic_light2_f_red.glsl"));
			//entity->getRenderDataPtr()->shader = HgShader::acquire("test_matrix_v.glsl", "test_matrix_f.glsl");

			HgEntity* teapotSquare = EntityTable::Singleton().getPtr(idList[1]);
			change_to_cube(teapotSquare);

			teapotSquare->setRenderData(RenderData::Create(teapotSquare->getRenderDataPtr()));
			teapotSquare->scale(0.3);
			teapotSquare->position(point(0, 3, 0));

			auto flags = et.getFlags(idList[1]);
			flags.inheritParentScale = false;
			et.setFlags(idList[1], flags);

			teapotSquare->setParent(teapotId);
			teapotSquare->getRenderDataPtr()->getMaterial().setShader(HgShader::acquire("basic_light2_v_instance.glsl", "basic_light2_f_blue.glsl"));
			//teapotSquare->setHidden(true);
			SOUND->play3d(snd, snd->EmitFromEntity(teapotSquare));
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

	//HgTimer gameTimer;
	HgTime last_time;

	int8_t do_render = 1;
	uint8_t did_change = 0;

	int16_t mouse_x, mouse_y;
	mouse_x = mouse_y = 0;

	HgTime accumulatedTime;
	const HgTime timeStep = HgTime::msec(8); // about 120 ups

	//Instantiate render queues here.
	//If they persist outside the while running loop we can reuse their data when the scene has not changed.
	RenderQueue renderQueue;
	RenderQueue renderQueue2d;
	//gameTimer.start();

	bool sceneUpdated = false;

	HgTime time;
	HgTimer realTime;
	realTime.start();
	
	accumulatedTime += timeStep;
	
	while (!window->m_close) {
		if (accumulatedTime.msec() > 100) {
			accumulatedTime = HgTime::msec(100);
		}

		HgTime elaspedTime;

		//update loop
		while (accumulatedTime >= timeStep) {
			time += timeStep;
			accumulatedTime -= timeStep;
			elaspedTime += timeStep;

			if (timeStep.msec() > 0) {
				vector3 v;

				if (KeyDownMap[KeyCodes::KEY_W]) v.z(v.z() - 1.0f);
				if (KeyDownMap[KeyCodes::KEY_S]) v.z(v.z() + 1.0f);
				if (KeyDownMap[KeyCodes::KEY_A]) v.x(v.x() - 1.0f);
				if (KeyDownMap[KeyCodes::KEY_D]) v.x(v.x() + 1.0f);


				if (KeyDownMap[KeyCodes::KEY_SPACE]) {
					//snd->stop();
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
					point zero(0, 1, 0);
					camera.setWorldSpacePosition(zero);
				}
				MOUSE_INPUT.dx = 0;
				MOUSE_INPUT.dy = 0;
			}

			{
				HgEntity* grid = EntityTable::Singleton().getPtr(gridId);
				grid->position(point(camera.getWorldSpacePosition()).y(0));
			}

			const auto orientation = quaternion::fromEuler(angle::ZERO, angle::deg(std::fmod(time.msec(),10000) / 27.777777777777777777777777777778), angle::ZERO);
			{
				auto teapot = EntityTable::Singleton().getPtr(teapotId);
				teapot->orientation(orientation.conjugate());
			}

			//{
			//	//Update polyboard when camera moves
			//	constexpr auto vertexCount = MeshMath::computePolyboardVertexCount(pathCount);
			//	vertex3f polyBoardMesh[vertexCount];

			//	constexpr auto indexCount = MeshMath::computePolyboardIndexCount(pathCount);
			//	uint32_t polyBoardIndices[indexCount];
			//	MeshMath::generatePolyboard(polyBoardLine, pathCount, camera.getWorldSpacePosition(), 1, polyBoardMesh, polyBoardIndices);

			//	pb->getEntity().renderData()->VertexVboRecord(
			//		HgVbo::GenerateUniqueFrom(polyBoardMesh, vertexCount)
			//	);
			//}

			//scene.update(timeStep);
			scene2.update(timeStep);
			scene_2d.update(timeStep);

			RotatingCube::updateRotatingCubes(timeStep);

			Engine::updateCollections(timeStep);
			sceneUpdated = true;
		}


		HgTime remainTime = accumulatedTime;

		SOUND->update();

		//set camera to player
		if (openVr.isValid())
		{
			camera.setWorldSpacePosition(PLAYER1.position());
			camera.setWorldSpaceOrientation(PLAYER1.orientation());
		}

		HgSound::Listener listener;
		listener.setPosition(camera.getWorldSpacePosition());
		listener.setForward(camera.getForward());
		listener.setUp(camera.getUp());
		SOUND->setListener(listener);

		{
			ENGINE::INPUT::PumpMessages();

			if (sceneUpdated)
			{
				//if the scene was not updated, don't rebuild the render queue
				renderQueue.Clear();
				renderQueue2d.Clear();

				//scene.EnqueueForRender(&renderQueue, elaspedTime);
				scene2.EnqueueForRender(&renderQueue, elaspedTime);

				scene_2d.EnqueueForRender(&renderQueue2d, elaspedTime);
				//Engine::EnqueueForRender(Engine::collections(), &renderQueue);

				renderQueue.Finalize(remainTime);
				renderQueue2d.Finalize(remainTime);
			}

			HgCamera cam2d;
			PerspectiveProjection perspective;
			OrthographicProjection orthographic;

			RenderParamsList rpl;
			rpl.emplace_back(&camera, &perspective, &renderQueue);
			rpl.emplace_back(&cam2d, &orthographic, &renderQueue2d);


			renderTarget->Render(rpl);
			//renderTarget->Render(&cam2d, &renderQueue2d, projection2d);

			window->SwapBuffers();
			InterlockedAdd(&itrctr, 1);
			sceneUpdated = false;
		}

		HgTime realTimeDt;

		//how much realtime was taken for this game loop
		if (openVr.isValid())
		{
			OpenVRRenderTarget* vrTarget = (OpenVRRenderTarget*)renderTarget.get();
			vrTarget->updateHMD();
			realTimeDt = vrTarget->deltaTime();
			openVr.HandleInput();
		}
		else
		{
			realTimeDt = realTime.getElaspedAndRestart();
		}

		//printf("frame time %f\n", realTimeDt.msec());

		//slow down if we are going really fast
		if (realTimeDt.msec() < 1.0)
		{
			//Sleep(1);
		}

		accumulatedTime += realTimeDt;

	}//main loop

	return 0;
}

