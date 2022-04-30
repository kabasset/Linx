# Cnes.Raster

This project is a work in progress to gather n-D array related features
of EleFits' Raster -- an extensible n-D data storage class with pixel-wise operations --
and PhiFun's operations -- e.g. Fourier transforms, linear filtering, and interpolation.

The target is a header-only library focused on ease of use
and which interfaces seamlessly with the standard C++ library.
The image processing feature set is expected to grow steadily on demand.
Currently supported operations are:

* Pixel-wise operations through `Raster::apply()` and `Raster::generate()`;
* Discrete Fourier transforms through the `Dft` class (to be integrated);
* Linear filtering through the `Kernel` class.
