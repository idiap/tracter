#ifndef SOURCE_H
#define SOURCE_H

/**
 * Interface for a source plugin.  As a source plugin has basically
 * the same interface as any other plugin, this is designed to be
 * multiply inherited.  i.e., a source plugin should inherit this
 * *and* a typed plugin.
 */
class Source
{
public:
    /** Open a source with the given name */
    virtual void Open(const char* iName) = 0;
    virtual ~Source() {}
protected:
    char* mName;
};

#endif /* SOURCE_H */
