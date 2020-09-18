/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FILEPATH_H
#define FILEPATH_H

#include <string>
#include <vector>

namespace Tracter
{
    /**
     * Class to manage files and paths.  Given a full path to a file,
     * separates it into path, basename and extension.  e.g,
     * '/path/to/file.ext' is represented internally as '/path/to', 'file'
     * and 'ext'.  The idea is that it is then easy to operate on any of
     * the entities separately.
     */
    class FilePath
    {
    public:
        void setName(const char* iFilePath);
        void dump();
        void makePath();

    private:
        std::string mPath;
        std::string mBase;
        std::string mExtension;
        std::vector<char> mTmp;
    };
}

#endif /* FILEPATH_H */
