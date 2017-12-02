#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include </linux/wait.h>
#include <linux/semaphore.h> 

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Yang Liu");    ///< The author -- visible when you use modinfo


// static int my_open(struct miscdevice * my_misc_device, struct file *)
// static ssize_t my_read(  struct file *file,
//   char __user * out,
//   size_t size,
//   loff_t * off);
// static int my_close(struct miscdevice* my_misc_device, struct file *filep);

static int pipe_size;
static int *myqueue;
static int start;
static int end;
static int buf;
static int error=0;
static struct semaphore sem_notempty;
static struct semaphore sem_notfull;
static struct semaphore mutex;

static void pushQueue(const int num){
  myqueue[end] = num; end = (end+1)%pipe_size;
}
static int popQueue(void){
  int cur = myqueue[start];
  start = (start+1)%pipe_size;
  return cur;
}

static ssize_t my_read(struct file *file, char __user * out, size_t size, loff_t * off) {
    int myerr = 0;
    myerr = down_interruptible(&sem_notempty);
    if(myerr<0)return -1;
    myerr = 0;
    myerr = down_interruptible(&mutex);
    if(myerr<0){
      up(sem_notempty);
      return -1;
    }
    buf = myqueue[start];
    error = copy_to_user(out, &buf, (unsigned long)(sizeof(int)));
    if(error<0){
      up(&mutex);
      return -1;
    }
    buf = popQueue();
    up(&mutex);
    up(&sem_notfull);
    return sizeof(int);
}

static ssize_t my_write (struct file *file, const char __user *in, size_t size, loff_t *off){
    int myerr = 0;
    myerr = down_interruptible(&sem_notfull);
    if(myerr<0)return -1;
    myerr = 0;
    myerr = down_interruptible(&mutex);
    if(myerr<0){
      up(sem_notfull);
      return -1;
    }
    error = copy_from_user(&buf, in, sizeof(int));
    if(error<0){ 
      up(&mutex);return -1;
    }
    pushQueue(buf);
    up(&mutex);
    up(&sem_notempty);
    return sizeof(int);
}

static int my_open (struct inode * id, struct file * filep){
  printk(KERN_ALERT "Char Device has been opened.\n");
  return 0;
}

static int my_close (struct file * filep, fl_owner_t id){
  printk(KERN_ALERT "Char Device successfully closed.\n");
  return 0;
}

static struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .flush = my_close,
  .read = my_read,
  .write = my_write,
};

static struct miscdevice charpipe = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "charpipe",
  .fops = &my_fops
};

static int __init my_init(void) {
  printk(KERN_ALERT "Init Char Pipe sucessfully.\n");
  sema_init(&sem_notempty, 0);
  sema_init(&sem_notfull, pipe_size); 
  sema_init(&mutex, 1);
  myqueue = (int *)kmalloc(pipe_size*sizeof(int), GFP_KERNEL);
  start = 0; end=0;
  misc_register(&charpipe);
  return 0;
}

static void __exit my_exit(void) {
  misc_deregister(&charpipe);
  kfree(myqueue);
  printk(KERN_ALERT "Exit Char Pipe.\n");
}



module_init(my_init);
module_exit(my_exit);
module_param(pipe_size, int, 0000);
