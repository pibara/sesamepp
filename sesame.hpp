#include <sys/types.h>
#include "openfilecollection.hpp"
#include "simplefile.hpp"
namespace sesame {
  namespace impl {
    static  openfilecollection<sesame::simplefile,960,65536> collection;
  }
  uint64_t open(const char *pathname, int flags);
  uint64_t open(const char *pathname, int flags, mode_t mode);
  uint64_t creat(const char *pathname, mode_t mode);
  ssize_t read(uint64_t fd, void *buf, size_t count);
  ssize_t readv(uint64_t fd, const struct iovec *iov, int iovcnt);
  ssize_t pread(uint64_t fd, void *buf, size_t count, off_t offset);  
  ssize_t preadv(uint64_t fd, const struct iovec *iov, int iovcnt,off_t offset);
  ssize_t write(uint64_t fd, const void *buf, size_t count);  
  ssize_t writev(uint64_t fd, const struct iovec *iov, int iovcnt);  
  ssize_t pwrite(uint64_t fd, const void *buf, size_t count, off_t offset);
  ssize_t pwritev(uint64_t fd, const struct iovec *iov, int iovcnt, off_t offset);
  ssize_t sendfile(uint64_t out_fd, int in_fd, off_t *offset, size_t count);
  off_t lseek(uint64_t fd, off_t offset, int whence);
  int fstat(uint64_t fd, struct stat *buf); 
  int fchmod(uint64_t fd, mode_t mode);
  int fchown(uint64_t fd, uid_t owner, gid_t group); 
  int ftruncate(uint64_t fd, off_t length);
  int fsync(uint64_t fd);
  int fdatasync(uint64_t fd);
  int fstatfs(uint64_t fd, struct statfs *buf);
  int close(uint64_t fd);
  uint64_t dup(uint64_t oldfd);
  uint64_t dup2(uint64_t oldfd, uint64_t newfd);
  uint64_t dup3(uint64_t oldfd, uint64_t newfd, int flags);
  uint64_t fcntl(uint64_t fd, int cmd, ... /* arg */ );
}
