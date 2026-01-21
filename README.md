# Linux Kernel Process Module 05  
## Sysfs + Uevent Notification Demo

This repository contains a Linux kernel module demonstrating:

- Sysfs interface creation using `kobject`
- Kernel linked lists (`list_head`)
- Mutex-based synchronization
- Runtime value management from user space
- `kobject_uevent()` notifications (`KOBJ_CHANGE`)

## Sysfs Path

/sys/kernel/sysfs_ueventnotification/


## Attributes

| Attribute | Access | Description |
|---------|--------|-------------|
| list    | RO     | Displays stored integer values |
| count   | RO     | Number of stored values |
| add     | WO     | Add an integer to the kernel list |
| clear   | WO     | Clear all stored values |

## Build

```bash
make

Load Module
sudo insmod sysfs_ueventnotification.ko

Usage
echo 10 | sudo tee add
echo 20 | sudo tee add
cat list
cat count
echo 1 | sudo tee clear

Monitor Uevents
udevadm monitor --kernel

Unload
sudo rmmod sysfs_ueventnotification

License
GPL v2


Save and exit.

---

## 2️⃣ Create LICENSE file (matches kernel GPL)

```bash
nano LICENSE

