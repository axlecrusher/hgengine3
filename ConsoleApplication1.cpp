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

#include <oglShaders.h>
#include <HgMath.h>
#include <HgTypes.h>
}

#include <HgRenderQueue.h>

#define M_PI 3.14159265358979323846

float projection[16];

viewport view_port[2];
HgCamera camera;

HANDLE endOfRenderFrame;

void gluPerspective(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M)
{
	double top = tan(fov*0.5 * RADIANS) * znear;
	double bottom = -top;
	double right = aspect*top;
	double left = -right;

	M[0] = (2*znear) / (right-left);
	M[2] = (right+left) / (right - left);
	M[6] = (top+bottom) / (top - bottom);
	M[5] = (2 * znear) / (top-bottom);
	M[10] = (zfar + znear) / (zfar-znear);
	M[11] = (2 * zfar*znear) / (zfar-znear);
	M[14] = -1.0f;
}

void gluPerspective2(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M)
{
	fov *= RADIANS;

	double f = 1.0 / tan(fov*0.5);
	memset(M, 0, 16 * sizeof* M);


	M[0] = f / aspect;
	M[5] = f;
	M[10] = (zfar + znear) / (znear - zfar);
	M[11] = (2*zfar*znear) / (znear - zfar);
	M[14] = -1.0f;
}



/*
// set the OpenGL perspective projection matrix
void glFrustum(
	const float &bottom, const float &top, const float &left, const float &right,
	const float &zNear, const float &zFar,
	float *M)
{
	//Should go in projection matrix
	float near2 = 2 * zNear;
	float rml = right - left;
	float tmb = top - bottom;
	float fmn = zFar - zNear;

	float A = (right + left) / rml;
	float B = (top + bottom) / tmb;
	float C = -(zFar + zNear) / fmn;
	float D = -(near2*zFar*zNear) / fmn;

	memset(M, 0, 16 * sizeof* M);

	//row major
	M[0] = near2 / rml;
	M[2] = A;
	M[5] = near2 / tmb;
	M[6] = B;
	M[10] = C;
	M[11] = D;
	M[14] = -1;
}
*/


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

	uint8_t stop = 0;
	while (1) {
		w->PumpMessages();
		
		w->Clear();

//		glUniformMatrix4fv(4, 1, GL_TRUE, view);
		glUniformMatrix4fv(5, 1, GL_TRUE, projection);

		glUniform4f(6, camera.rotation.x, camera.rotation.y, camera.rotation.z, camera.rotation.w);
		glUniform3f(7, camera.position.components.x, camera.position.components.y, camera.position.components.z);

		while (stop == 0) {
			HgRenderQueue* x = hgRenderQueue_pop();
			while (x == NULL) {
				Sleep(1);
				x = hgRenderQueue_pop();
			}

			stop = !x->rp->render();
/*
			if (x->rp->element == NULL) {
				stop = 1;
			}
			else {
				glUniform3f(7, x->rp->cam_position[0], x->rp->cam_position[1], x->rp->cam_position[2]);
//				glViewport(0, 0, 1280*0.5, 480);
				glViewport(1280 * 0.5 * x->rp->eye,0,1280*0.5,480);
				x->rp->element->m_renderData->renderFunc(x->rp->element);

				//change perspective, and re-render
				//currently, it seems faster to render all of one view, followed by all of another view.
				//this could change with scene complexity that woudl require more shader switching
//				glUniform3f(7, camera_position[0]+0.07, camera_position[1], camera_position[2]);
//				glViewport(1280 * 0.5,0,1280*0.5,480);
//				glDrawArrays(GL_TRIANGLES, 0, 3);

//				x->rp->element->m_renderData->renderFunc(x->rp->element);
			}
			*/
//			delete(x);
			delete(x->rp);
			free(x);
		}

		//signal main game thread that we are done rendering frame.
		//swap buffers is slow, calling this before swapping allows the main thread to begin processing the next frame step
		if (!SetEvent(endOfRenderFrame))
		{
			printf("SetEvent failed endOfRenderFrame (%d)\n", GetLastError());
			return 1;
		}

		stop = 0;
//		HgElement* e = (HgElement*)render_thing;
//		if (e != NULL) {
//			e->m_renderData->renderFunc(e);
//		}
//		glFlush();
		w->SwapBuffers();
		
//		Sleep(10);
	}
}

volatile LONG itrctr;

DWORD WINAPI PrintCtr(LPVOID lpParam) {
	while (1) {
		printf("FPS %u\n", itrctr);
		itrctr = 0;
		Sleep(1000);
	}
}

uint8_t eye = 0;

void send_to_render_queue(HgElement* e) {
//	render_packet* rp = (render_packet*)calloc(1, sizeof* rp);
	RenderElement* rp = new RenderElement();

	memcpy(rp->cam_position, camera.position.array, 3* sizeof* camera.position.array);
	rp->eye = eye;
	rp->vp = view_port + eye;

	rp->element = e;
	if (e != NULL) {
		//	rp->position = e->position;
		//	rp->rotation = e->rotation;
		//	rp->vao = e->m_renderData->renderFunc
	}
	else {
//		printf("null\n");
	}

	hgRenderQueue_push(rp);
}
#define ANI_TRIS 400

int main()
{
//	MercuryWindow* w = MercuryWindow::MakeWindow();
	
//	gen_triangle(&points);

	view_port[0].x = view_port[0].y = 0;
	view_port[0].width = 1280 * 0.5;
	view_port[0].height = 480;


	view_port[1].x = 1280 * 0.5;
	view_port[1].y = 0;
	view_port[1].width = 1280 * 0.5;
	view_port[1].height = 480;
	
	uint8_t s = sizeof(HgElement);
	printf("element size %d\n", s);
	printf("vertex size %d\n", sizeof(vertex));
	printf("oglrd size %d\n", sizeof(OGLRenderData));

//	gluPerspective(60, 640.0 / 480.0, 0.1f, 100.0f, projection);
	gluPerspective2(60, 640.0/480.0, 0.1f, 100.0f,projection);
//	gluPerspective2(60, 320.0 / 480.0, 0.1f, 100.0f, projection);

	quaternion_init(&camera.rotation);
	memset(camera.position.array, 0, sizeof camera.position);
	camera.position.components.z = -1.5f;
	camera.position.components.y = -2.0f;
	toQuaternion2(0, 15, 0, &camera.rotation); //y,x,z

//	MatrixMultiply4f(projection, camera, view);

	print_matrix(projection);
	printf("\n");

	HgScene scene;
	scene_init(&scene, 1000);

	HgElement* element = scene_newElement(&scene);
	shape_create_triangle(element);
	element->position[0] = 1.5f;
	element->position[2] = -1.0f;
//	toQuaternion2(0, 0, 90, &element->rotation);

	element = scene_newElement(&scene);
	shape_create_triangle(element);
	element->position[0] = -0.0f;
	element->position[2] = -2.0f;
//	toQuaternion2(45,0,0,&element->rotation);

	HgElement* tris[ANI_TRIS];

	uint32_t i;
	for (i = 0; i < ANI_TRIS; i++) {
		tris[i] = element = scene_newElement(&scene);
		shape_create_triangle(element);
		float x = (i % 20)*1.1;
		float z = (i / 20)*1.1;
		element->position[0] = -10.0 + x;
		element->position[2] = -2.0f - z;
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

	int16_t r = 0;
	uint32_t stime = GetTickCount();
	uint32_t time = stime;
	uint32_t dtime = time - stime;
	while (1) {
		dtime = GetTickCount() - stime;

		HgScene_iterator itr;
		scene_init_iterator(&itr,&scene);
		HgElement* e = scene_next_element(&itr);

		r++;
		r = r % 3600;

//		y,x,z
		toQuaternion2((dtime%1000)/ 2.7777777777777777777777777777778, 0, 0, &element->rotation);
//		toQuaternion2((dtime % 10000) / 27.777777777777777777777777777778, 0, 0, &camera_rot);

		for (i = 0; i < ANI_TRIS; i++) {
			memcpy(&tris[i]->rotation, &element->rotation, sizeof element->rotation);
		}

		eye = 0;
		while (e != NULL) {
			if (e->updateFunc != NULL) e->updateFunc(e,0);
			if (is_destroyed(e) > 0) scene_delete_element(&itr);
			if (check_flag(e, HGE_HIDDEN) == 0) send_to_render_queue(e); //submit to renderer
			e = scene_next_element(&itr);
		}

		float tmp = camera.position.components.x;
		camera.position.components.x += 0.07;

		scene_init_iterator(&itr, &scene);
		e = scene_next_element(&itr);

		eye = 1;
		while (e != NULL) {
			if (check_flag(e, HGE_HIDDEN) == 0) send_to_render_queue(e); //submit to renderer
			e = scene_next_element(&itr);
		}

//		scene_clearUpdate(&scene);
		camera.position.components.x = tmp;

		InterlockedAdd(&itrctr,1);

		hgRenderQueue_push(new EndOfFrame());
//		send_to_render_queue(NULL); //null element to indicate end of frame

		DWORD dwWaitResult = WaitForSingleObject(
			endOfRenderFrame, // event handle
			INFINITE);    // indefinite wait

		ResetEvent(endOfRenderFrame);
	}

    return 0;
}

