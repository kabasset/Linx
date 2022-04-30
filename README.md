# Cnes.Raster

This project is a work in progress to gather _n_D array related features
of EleFits' Raster -- an extensible _n_D data storage class with pixel-wise operations --
and PhiFun's operations -- e.g. Fourier transforms, linear filtering, and interpolation.

The target is a header-only library focused on ease of use
and which interfaces seamlessly with the standard C++ library.
The image processing feature set is expected to grow steadily on demand.
Currently supported operations are:

* Pixel-wise operations through `Raster::apply()` and `Raster::generate()`;
* Discrete Fourier transforms through the `Dft` class;
* Linear filtering through the `Kernel` class.

# Alternatives

* Armadillo
* Blitz++
* Boost.MultiArray
* CImg
* Eigen
* ITK, SimpleITK
* ndarray
* OpenCV
* STL's valarray
* XTensor
