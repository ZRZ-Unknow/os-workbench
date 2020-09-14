#include <devices.h>

static int sd_init(device_t *dev) {
  sd_t *sd = dev->ptr;
  _DEV_STORAGE_INFO_t info;
  if (_io_read(_DEV_STORAGE, _DEVREG_STORAGE_INFO, &info, sizeof(info)) <= 0) {
    dev->ptr = NULL;
  } else {
    sd->blkcnt = info.blkcnt;
    sd->blksz  = info.blksz;
    sd->buf    = pmm->alloc(sd->blksz);
  }
  return 0;
}

static void blk_read(void *buf, int blkno, int blkcnt) {
  _DEV_STORAGE_RDCTRL_t ctl;
  ctl.buf    = buf;
  ctl.blkno  = blkno;
  ctl.blkcnt = blkcnt;
  _io_write(_DEV_STORAGE, _DEVREG_STORAGE_RDCTRL, &ctl, sizeof(ctl));
}

static void blk_write(void *buf, int blkno, int blkcnt) {
  _DEV_STORAGE_WRCTRL_t ctl;
  ctl.buf    = buf;
  ctl.blkno  = blkno;
  ctl.blkcnt = blkcnt;
  _io_write(_DEV_STORAGE, _DEVREG_STORAGE_WRCTRL, &ctl, sizeof(ctl));
}

static ssize_t sd_read(device_t *dev, off_t offset, void *buf, size_t count) {
  sd_t *sd = dev->ptr;
  panic_on(!sd, "no disk");
  uint32_t pos = 0;
  for (uint32_t st = ROUNDDOWN(offset, sd->blksz); pos < count; st = offset) {
    uint32_t n = sd->blksz - (offset - st);
    if (n > count - pos) n = count - pos;
    blk_read(sd->buf, st / sd->blksz, 1);
    memcpy((char *)buf + pos, sd->buf + offset - st, n);
    pos   += n;
    offset = st + sd->blksz;
  }
  return pos;
}

static ssize_t sd_write(device_t *dev, off_t offset, const void *buf, size_t count) {
  sd_t *sd = dev->ptr;
  panic_on(!sd, "no disk");
  uint32_t pos = 0;
  for (uint32_t st = ROUNDDOWN(offset, sd->blksz); pos < count; st = offset) {
    uint32_t n = sd->blksz - (offset - st);
    if (n > count - pos) n = count - pos;
    if (n < sd->blksz) {
      blk_read(sd->buf, st / sd->blksz, 1);
    }
    memcpy(sd->buf + offset - st, (char *)buf + pos, n);
    blk_write(sd->buf, st / sd->blksz, 1);
    pos   += n;
    offset = st + sd->blksz;
  }
  return pos;
}

devops_t sd_ops = {
  .init  = sd_init,
  .read  = sd_read,
  .write = sd_write,
};
