#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <semaphore.h> 

#define POSIX_IPC_NAME_PREFIX "/yang-"
#define EMPTY_SEM_NAME POSIX_IPC_NAME_PREFIX "nemptysem"
#define FULL_SEM_NAME POSIX_IPC_NAME_PREFIX "nfullsem"

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Yang Liu");    ///< The author -- visible when you use modinfo


// static int my_open(struct miscdevice * my_misc_device, struct file *)
// static ssize_t my_read(  struct file *file,
//   char __user * out,
//   size_t size,
//   loff_t * off);
// static int my_close(struct miscdevice* my_misc_device, struct file *filep);

static int fildes[2];
static int pipe_size;
static sem_t *sem_notempty;
static sem_t *sem_notfull;

static ssize_t my_read(
  struct file *file,
  char __user * out,
  size_t size,
  loff_t * off) {
    if(sem_wait(sem_notempty)<0)return -1;
    char buf[1];
    read(fildes[0],buf, 1);
    put_user(buf, out);
    if(sem_post(sem_notfull)<0)return -1;
    return 1;
}

ssize_t my_write (struct file *file, const char __user *in, size_t, size, loff_t *off){
  if(sem_wait(sem_notfull)<0)return -1;
    write(fildes[1], in, 1);
    if(sem_post(sem_notempty)<0)return -1;
    return 1;
}

static int my_open (struct inode * id, struct file * filep){
  printk(KERN_ALERT "Char Device has been opened.\n");
  return 0;
}

static int my_close (struct file *, fl_owner_t id); {
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
  sem_notempty= sem_open(NEMPTY_SEM_NAME,O_RDWR|O_CREAT,ALL_RW_PERMS, 0);
  sem_notfull= sem_open(NFULL_SEM_NAME,O_RDWR|O_CREAT,ALL_RW_PERMS, pipe_size);  
  if(pipe(fildes)==-1){
    return -1;
  }
  misc_register(&charpipe);
  return 0;
}

static void __exit my_exit(void) {
  misc_deregister(&charpipe);
  sem_close(sem_notempty);
  sem_close(sem_notfull);
  close(fildes[0]);
  close(fildes[1]);
  printk(KERN_ALERT "Exit Char Pipe.\n");
}



module_init(my_init);
module_exit(my_exit);
module_param(pipe_size, int, 0000);
