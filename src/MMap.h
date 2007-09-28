#ifndef MMAP_H
#define MMAP_H

/**
 * Wraps the usual unix mmap into a class
 */
class MMap
{
public:
    MMap();
    ~MMap();
    void* Map(const char* iFileName);
    size_t GetSize();

private:
    int mFD;    // File descriptor
    void* mMap; // Mapped memory location
    size_t mSize;
};

#endif /* MMAP_H */
