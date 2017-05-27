#include <HgRenderQueue.h>
#include <stdlib.h>

#include <Windows.h>

static volatile HgRenderQueue* queue_head = NULL;
static volatile HgRenderQueue* queue_tail = NULL;
static volatile uint32_t count;

volatile LONG wait = 0;

volatile uint32_t hgRenderQueue_length() {
	return count;
}

void hgRenderQueue_push(RenderPacket* p) {
//	HgRenderQueue* x = calloc(1, sizeof *x);
	HgRenderQueue* x = new HgRenderQueue();
	x->next = NULL;
	x->rp = p;

	while (InterlockedCompareExchange(&wait, 1, 0)>0);

	if (queue_head == NULL) queue_head = x;

	if (queue_tail == NULL) {
		queue_tail = x;
	}
	else {
		queue_tail->next = x;
		queue_tail = x;
	}

	count++;
	InterlockedDecrement(&wait);
}

HgRenderQueue* hgRenderQueue_pop() {
	HgRenderQueue* x = NULL;
	while (InterlockedCompareExchange(&wait, 1, 0)>0);

	if (queue_head != NULL) {
		x = const_cast<HgRenderQueue*>(queue_head);

		if (queue_head == queue_tail) {
			queue_head = queue_tail = NULL;
		}
		else {
			queue_head = x->next;
		}

		count--;
	}

	InterlockedDecrement(&wait);

	return x;
}

RenderPacket* create_render_packet(HgElement* e, uint8_t viewport_idx, HgCamera* camera) {
	//	render_packet* rp = (render_packet*)calloc(1, sizeof* rp);
	RenderElement* rp = new RenderElement();

	rp->camera = *camera;
	rp->viewport_idx = viewport_idx;

	rp->element = e;
	return rp;
}