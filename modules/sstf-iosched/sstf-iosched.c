/*
 * SSTF IO Scheduler
 *
 * For Kernel 4.13.9
 */

#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
sector_t cur_sect = (sector_t) 0;
/* SSTF data structure. */
//https://stackoverflow.com/questions/16230524/explain-list-for-each-entry-and-list-for-each-entry-safe
//https://kernelnewbies.org/FAQ/LinkedLists
struct sstf_data {
	struct list_head queue;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

int is_better_request(struct request* rq, struct request* current_best ){
	if(!current_best){
		return 1;
	}
	sector_t s1, s2, delta1, delta2;
	s1 = blk_rq_pos(rq);
	s2 = blk_rq_pos(current_best);
	delta1 = s1 - cur_sect<0 ? (-1)*(s1 - cur_sect):(s1 - cur_sect);
	delta2 = s2 - cur_sect<0 ? (-1)*(s2 - cur_sect):(s2 - cur_sect);
	return delta1<delta2?1:0;
}

/* Esta função despacha o próximo bloco a ser lido. */
static int sstf_dispatch(struct request_queue *q, int force){
	struct sstf_data* nd = q->elevator->elevator_data;
	struct request* rq;
	struct request* best_request = NULL;
	list_for_each_entry(rq, &nd->queue, queuelist) {
		if (is_better_request(rq, best_request)==1) {
            best_request = rq;
        }
	}
	if (best_request) {
		cur_sect = blk_rq_pos(best_request);
        list_del_init(&best_request->queuelist);
        elv_dispatch_sort(q, best_request);
		printk(KERN_EMERG "[SSTF] best request: %llu\n",cur_sect);
        return 1;
    }
	return 0;
}
static void sstf_add_request(struct request_queue *q, struct request *rq){
	struct sstf_data *nd = q->elevator->elevator_data;
	char direction = 'R';
	list_add_tail(&rq->queuelist, &nd->queue);
	printk(KERN_EMERG "[SSTF] add %c %llu\n", direction, blk_rq_pos(rq));
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e){
	struct sstf_data *nd;
	struct elevator_queue *eq;

	/* Implementação da inicialização da fila (queue).
	 *
	 * Use como exemplo a inicialização da fila no driver noop-iosched.c
	 *
	 */

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	/* Implementação da finalização da fila (queue).
	 *
	 * Use como exemplo o driver noop-iosched.c
	 *
	 */
	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

/* Infrastrutura dos drivers de IO Scheduling. */
static struct elevator_type elevator_sstf = {
	.ops.sq = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

/* Inicialização do driver. */
static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

/* Finalização do driver. */
static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("Miguel Xavier");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
