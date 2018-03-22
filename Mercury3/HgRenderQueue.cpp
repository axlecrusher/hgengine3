#include <HgRenderQueue.h>
#include <stdlib.h>

#include <Windows.h>

//this structure is pretty compact. It may be good to allocate as an array for better caching
typedef struct HgRenderQueue {
	render_packet* rp;
	struct HgRenderQueue* next;
} HgRenderQueue;

typedef struct render_queue {
	volatile HgRenderQueue* queue_head;
	volatile HgRenderQueue* queue_tail;
	volatile uint32_t count;
	volatile LONG wait;
} render_queue;

static render_queue to_draw;
static render_queue unused;
/*
static volatile HgRenderQueue* queue_head = NULL;
static volatile HgRenderQueue* queue_tail = NULL;
static volatile uint32_t count;

volatile LONG wait = 0;
*/

volatile uint32_t hgRenderQueue_length() {
	return to_draw.count;
}

static void _push(render_queue* q, HgRenderQueue* x) {
	x->next = NULL;

	while (InterlockedCompareExchange(&q->wait, 1, 0)>0);

	if (q->queue_head == NULL) q->queue_head = x;

	if (q->queue_tail == NULL) {
		q->queue_tail = x;
	}
	else {
		q->queue_tail->next = x;
		q->queue_tail = x;
	}

	q->count++;
	InterlockedDecrement(&q->wait);
}

static HgRenderQueue* _pop(render_queue* q) {
	HgRenderQueue* x = NULL;
	while (InterlockedCompareExchange(&q->wait, 1, 0)>0);

	if (q->queue_head != NULL) {
		x = (HgRenderQueue*)q->queue_head;

		if (q->queue_head == q->queue_tail) {
			q->queue_head = q->queue_tail = NULL;
		}
		else {
			q->queue_head = x->next;
		}

		q->count--;
	}

	InterlockedDecrement(&q->wait);

	return x;
}

static void HgRenderQueue_init(HgRenderQueue* x) {
	x->next = NULL;
	x->rp = NULL;
};

void hgRenderQueue_push(render_packet* p) {
	HgRenderQueue* x = _pop(&unused);
	if (x == NULL) {
//		printf("ALLOCATE\n");
		x = new HgRenderQueue();
	}

	x->next = NULL;
	x->rp = p;

	_push(&to_draw, x);
}

render_packet* hgRenderQueue_pop() {
	HgRenderQueue* x = _pop(&to_draw);
	if (x == NULL) return NULL;

	render_packet* rp = x->rp;

	HgRenderQueue_init(x);
	_push(&unused,x);

	return rp;
}

render_packet* create_render_packet(HgElement* e, uint8_t viewport_idx, HgCamera* camera, HgScene* scene, uint16_t idx) {
	render_packet* rp = new render_packet();
//	RenderElement* rp = new RenderElement();

	rp->renderData = NULL;
	if (scene == NULL) return rp;

	if (e) {
		rp->position = e->position;
		rp->rotation = e->rotation;
		rp->scale = e->scale;
		rp->renderData = e->renderData();
	}
	rp->camera = *camera;
	rp->viewport_idx = viewport_idx;

//	rp->element_idx = idx;
//	rp->scene = scene;
//	rp->element = e;
	return rp;
}