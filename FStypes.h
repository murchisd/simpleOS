// FStypes.h, 159

#ifndef __FSTYPES_H__
#define __FSTYPES_H__

//////////////////////////////////////////////// phase6 file services
#define FD_NUM Q_SIZE                         // # of avail FD

#define A_REG 0x8000 // REGular file type
#define A_DIR 0x4000 // DIRectory type
#define A_MT  0xF000 // Attribute Identifier Flag Mask Table for mask & compare

#define A_XOTH 00001 // other: execute
#define A_WOTH 00002 // other: write
#define A_ROTH 00004 // other: read
#define A_RWXO 00007 // other: read, write, execute
#define A_RWXG 00070 // group: read, write, execute
#define A_RWXU 00700 // owner: read, write, execute

// macros extract info from "mode" in "attr_t"
// is it a regular file or directory?
#define A_ISREG(mode) (((mode) & A_MT)==A_REG)
#define A_ISDIR(mode) (((mode) & A_MT)==A_DIR)

#define MODE_DIR (0777|A_DIR)
#define MODE_FILE (0666|A_REG)
#define MODE_EXEC (0777|A_REG)
#define MODE_READFILE (0444|A_REG)

#define END_INODE ((unsigned int)(~0)) // end marker of directory content

typedef struct {
   int inode,   // inode on the device
       mode,    // file access mode
       dev,     // PID of file service if any
       nlink,   // # of links to the file
       size;    // file size
   char *data;  // file content
} attr_t;

typedef struct { // directory type
   int inode,
       mode,
       size;
   char *name,
        *data;
} dir_t;

typedef struct {      // file descriptor type
   int owner,         // PID, -1 means not used
       offset;        // can be negative
   dir_t *item,
         *owning_dir; // dir where `item' resides
} fd_t;

#endif // __FSTYPES_H__

