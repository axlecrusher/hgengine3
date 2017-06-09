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

}



#define M_PI 3.14159265358979323846

float projection[16];

extern viewport view_port[];
HgCamera camera[2];

HANDLE endOfRenderFrame;

volatile int8_t needRender = 1;

#define EYE_DISTANCE -0.07

//uint8_t KeyDownMap[512];

DWORD WINAPI StartWindowSystem(LPVOID lpParam) {
	MercuryWindow* w = MercuryWindow::MakeWindow();

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

	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	uint8_t stop_frame = 0;
	while (1) {
		w->PumpMessages();
		
		w->Clear();

//		glUniformMatrix4fv(U_VIEW, 1, GL_TRUE, view);
//		glUniformMatrix4fv(U_PROJECTION, 1, GL_TRUE, projection);
		_projection = projection;

		while (stop_frame == 0) {
			render_packet* x = hgRenderQueue_pop();
			while (x == NULL) {
				Sleep(1);
				x = hgRenderQueue_pop();
			}

			if (x->scene == NULL) {
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

volatile LONG itrctr;

DWORD WINAPI PrintCtr(LPVOID lpParam) {
	while (1) {
		printf("UPS %u e_count %d %d\n", itrctr, scene.size_used, scene._size);
		itrctr = 0;
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

int main()
{
//	MercuryWindow* w = MercuryWindow::MakeWindow();
	
//	gen_triangle(&points);

	_create_shader = HGShader_ogl_create;

//	setup_viewports(1280,480);
	setup_viewports(640, 480);

	uint8_t s = sizeof(HgElement);
	printf("element size %d\n", s);
	printf("vertex size %d\n", sizeof(vertex));
	printf("render_packet size %d\n", sizeof(render_packet));
	printf("oglrd size %d\n", sizeof(OGLRenderData));

//	Perspective(60, 640.0 / 480.0, 0.1f, 100.0f, projection);
	Perspective2(60, 640.0/480.0, 0.1f, 100.0f,projection);
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

	HgScene scene;
	scene_init(&scene, 1000);

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

	uint32_t tris[ANI_TRIS];

	GravityField gravity = { 0 };
	allocate_space(&gravity, ANI_TRIS);
	gravity.scene = &scene;
	gravity.vector.components.y=-1;

	uint32_t i;
	for (i = 0; i < ANI_TRIS; i++) {
		tris[i] = scene_newElement(&scene, &element);
		gravity.indices[i] = tris[i];
//		shape_create_triangle(element);
		shape_create_cube(element);
		float x = (i % 20)*1.1;
		float z = (i / 20)*1.1;
		element->position.components.y = 5.0f;
		element->position.components.x = -10.0 + x;
		element->position.components.z = -2.0f - z;
		element->scale = 0.3f;
	}

	printf("\n%f %f %f %f\n", element->rotation.w, element->rotation.x, element->rotation.y, element->rotation.z);
	
	endOfRenderFrame = CreateEvent(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		NULL  // object name
	);

	HANDLE thread1 = CreateThread(NULL, 0, &PrintCtr, NULL, 0, NULL);
	HANDLE thread = CreateThread(NULL, 0, &StartWindowSystem, NULL, 0, NULL);
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

		if (ddtime > 0) {
			vector3 v;
			vector3_zero(&v);

			if (KeyDownMap['w']) v.components.z -= 1.0f;
			if (KeyDownMap['s']) v.components.z += 1.0f;
			if (KeyDownMap['a']) v.components.x -= 1.0f;
			if (KeyDownMap['d']) v.components.x += 1.0f;
			if (KeyDownMap['r']) {
				mouse_x = 0; mouse_y = -41.66666666666667;
			}

			if (KeyDownMap[' ']) {
//				printf("fire!\n");
				fire(&scene);
			}

//			if (v.components.z > 0) DebugBreak();

			float scale = (1.0 / 1000.0f) * ddtime;
			v = vector3_normalize(&v);
//			v = vector3_scale(&v, -1.0f);

			camera->rotation.w = -camera->rotation.w; //invert coordinate system for vector rotation
			v= vector3_quat_rotate(&v, &camera->rotation);
			camera->rotation.w = -camera->rotation.w; //restore

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

//		y,x,z
		toQuaternion2((dtime%1000)/ 2.7777777777777777777777777777778, 0, 0, &element->rotation);

		for (i = 0; i < ANI_TRIS; i++) {
			HgElement* e = scene.elements + tris[i];
			memcpy(&e->rotation, &element->rotation, sizeof element->rotation);
		}

		for (uint32_t i=0; i<scene._size; ++i) {
//			if (IS_USED(&scene, i) == 0) continue;
			if(is_used(&scene, i) == 0) continue;
			HgElement* e = scene.elements + i;

			if (ddtime > 0) {
				VCALL_IDX(e, updateFunc, ddtime);
			}

			/* FIXME: WARNING!!! if this loop is running async to the render thread, element deletion can cause a crash!*/
			if (CHECK_FLAG(e, HGE_DESTROY) > 0) {
				scene_delete_element(&scene, i);
				continue;
			}
			if ((CHECK_FLAG(e, HGE_HIDDEN) == 0) && (do_render > 0)) hgRenderQueue_push(create_render_packet(e, 0, camera + 0,&scene,i)); //submit to renderer
		}
		/*

		if (do_render > 0) {
			camera[1] = camera[0];
			camera[1].position.components.x += EYE_DISTANCE;
			for (uint32_t i = 0; i < scene._size; ++i) {
				if (is_used(&scene, i) == 0) continue;
				HgElement* e = scene.elements + i;
				if (CHECK_FLAG(e, HGE_HIDDEN) == 0) hgRenderQueue_push(create_render_packet(e, 2, camera + 1)); //submit to renderer
			}
		}
		*/

//		scene_clearUpdate(&scene);

		InterlockedAdd(&itrctr,1);


		if ((do_render>0))hgRenderQueue_push(create_render_packet(NULL, 2, camera + 1, NULL, 0)); //null element to indicate end of frame

		do_render = 0;

//		Sleep(1);
		
		DWORD dwWaitResult = WaitForSingleObject(
			endOfRenderFrame, // event handle
			INFINITE);    // indefinite wait

		ResetEvent(endOfRenderFrame);
	}

    return 0;
}

