#include <HgRenderQueue.h>
#include <stdlib.h>

#include <Windows.h>

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

void _push(render_queue* q, HgRenderQueue* x) {
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

HgRenderQueue* _pop(render_queue* q) {
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

void HgRenderQueue_init(HgRenderQueue* x) {
	x->next = NULL;
	x->rp = NULL;
};

void hgRenderQueue_push(render_packet* p) {
	HgRenderQueue* x = _pop(&unused);
	if (x == NULL) {
		printf("ALLOCATE\n");
		x = calloc(1, sizeof *x);
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

render_packet* create_render_packet(HgElement* e, uint8_t viewport_idx, HgCamera* camera) {
	render_packet* rp = calloc(1, sizeof* rp);
//	RenderElement* rp = new RenderElement();

	rp->camera = *camera;
	rp->viewport_idx = viewport_idx;

	rp->element = e;
	return rp;
}