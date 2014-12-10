// example code to show how linux workqueue worked
#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>

static struct workqueue_struct *queue = NULL;
static struct work_struct work;

static void work_handler(struct work_struct *data)
{
  printk(KERN_ALERT "work handler for work_item in queue helloworkqueue\n");
  // workqueue 中的每个工作完成之后就被移除 workqueue.
  // 下面的语句会造成"死机", 原因可能是该 workqueue 占据了所有的 CPU 时间.
  // queue_work(queue, &work);
}

static int __init test_init(void)
{
  queue = create_singlethread_workqueue("helloworkqueue");
  if (!queue)
  {   
    goto err;
  }   
  INIT_WORK(&work, work_handler);
  queue_work(queue, &work);

  return 0;
err:
  return -1; 
}
static void __exit test_exit(void)
{
  destroy_workqueue(queue);
}

MODULE_LICENSE("GPL");
module_init(test_init);
module_exit(test_exit);
