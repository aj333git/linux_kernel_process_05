#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Sysfs + Linked List + Uevent Demo");
MODULE_VERSION("2.1");

static struct kobject *kobj;

/* Linked list node */
struct value_node {
    int value;
    struct list_head list;
};

static LIST_HEAD(value_list);
static DEFINE_MUTEX(list_lock);

/* ---------- list show ---------- */
static ssize_t list_show(struct kobject *kobj,
                         struct kobj_attribute *attr, char *buf)
{
    struct value_node *node;
    ssize_t len = 0;

    mutex_lock(&list_lock);
    list_for_each_entry(node, &value_list, list) {
        len += scnprintf(buf + len, PAGE_SIZE - len,
                         "%d\n", node->value);
    }
    mutex_unlock(&list_lock);

    return len;
}

/* ---------- count show (NEW FEATURE) ---------- */
static ssize_t count_show(struct kobject *kobj,
                          struct kobj_attribute *attr, char *buf)
{
    struct value_node *node;
    int count = 0;

    mutex_lock(&list_lock);
    list_for_each_entry(node, &value_list, list) {
        count++;
    }
    mutex_unlock(&list_lock);

    return scnprintf(buf, PAGE_SIZE, "%d\n", count);
}

/* ---------- add value ---------- */
static ssize_t add_store(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    struct value_node *node;
    int val;

    if (kstrtoint(buf, 10, &val))
        return -EINVAL;

    node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node)
        return -ENOMEM;

    node->value = val;
    INIT_LIST_HEAD(&node->list);

    mutex_lock(&list_lock);
    list_add_tail(&node->list, &value_list);
    mutex_unlock(&list_lock);

    pr_info("sysfs_list: added %d\n", val);
    kobject_uevent(kobj, KOBJ_CHANGE);

    return count;
}

/* ---------- clear list ---------- */
static ssize_t clear_store(struct kobject *kobj,
                           struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
    struct value_node *node, *tmp;

    mutex_lock(&list_lock);
    list_for_each_entry_safe(node, tmp, &value_list, list) {
        list_del(&node->list);
        kfree(node);
    }
    mutex_unlock(&list_lock);

    pr_info("sysfs_list: cleared all entries\n");
    kobject_uevent(kobj, KOBJ_CHANGE);

    return count;
}

/* Sysfs attributes */
static struct kobj_attribute list_attr  = __ATTR_RO(list);
static struct kobj_attribute count_attr = __ATTR_RO(count);   /* NEW */
static struct kobj_attribute add_attr   = __ATTR_WO(add);
static struct kobj_attribute clear_attr = __ATTR_WO(clear);

static struct attribute *attrs[] = {
    &list_attr.attr,
    &count_attr.attr,   /* NEW */
    &add_attr.attr,
    &clear_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

/* ---------- init ---------- */
static int __init sysfs_list_init(void)
{
    kobj = kobject_create_and_add("sysfs_ueventnotification", kernel_kobj);
    if (!kobj)
        return -ENOMEM;

    if (sysfs_create_group(kobj, &attr_group)) {
        kobject_put(kobj);
        return -ENOMEM;
    }

    pr_info("sysfs_list: module loaded\n");
    return 0;
}

/* ---------- exit ---------- */
static void __exit sysfs_list_exit(void)
{
    struct value_node *node, *tmp;

    mutex_lock(&list_lock);
    list_for_each_entry_safe(node, tmp, &value_list, list) {
        list_del(&node->list);
        kfree(node);
    }
    mutex_unlock(&list_lock);

    kobject_put(kobj);
    pr_info("sysfs_list: module unloaded\n");
}

module_init(sysfs_list_init);
module_exit(sysfs_list_exit);

