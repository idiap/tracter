# Tracter: A lightweight dataflow framework

As of March 2010, the build system is based on CMake.  To build, you
should only need to do the following:
```sh
 cd build
 cp Configure.sh.example Configure.sh
 (edit Configure.sh to suit your environment)
 ./Configure.sh
 make
 make install
```
You must specifiy a path to `kissfft`; other dependencies are optional.
The executables will appear in `build/src`

Documentation is provided in `doxygen` form.  Go to the build
directory (the directory containing `Doxyfile`) and type
```sh
 doxygen
```
You can then point a browser at `html/index.html`.  Also see the
[project page](http://juicer.amiproject.org/tracter/) for a mailing
list and the like.

Some technical aspects of tracter are written up in this paper:
```
@InProceedings{Garner2010b,
  author =       "Garner, Philip N. and Dines, John",
  title =        "Tracter: A Lightweight Dataflow Framework",
  booktitle =    "Proceedings of Interspeech",
  year =         2010,
  address =      "Makuhari, Japan",
  month =        "September"
}
```
and there is a [downloadable pdf](http://publications.idiap.ch/downloads/papers/2010/Garner_INTERSPEECH_2010.pdf).

[Phil Garner](http://www.idiap.ch/~pgarner)

December 2007

January 2009
