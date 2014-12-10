// how linux kernel thread works
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>

struct task_struct* ts;

static int thread_func(void)
{
  while (1)
  {
    printk("I am a kernel thread!\n");

    msleep(100);
    if (kthread_should_stop())
      break;
  }

  return 0;
}

static int __init kthread_test_init(void)
{
  printk(KERN_INFO "kthread_test kthread_test_init called\n");
  ts = kthread_run(thread_func, 0, "kthread");
  return 0;
}

static void __exit kthread_test_exit(void)
{
  printk(KERN_INFO "kthread_test kthread_test_exit called\n");
  kthread_stop(ts);
}

MODULE_LICENSE("GPL");
module_init(kthread_test_init);
module_exit(kthread_test_exit);
