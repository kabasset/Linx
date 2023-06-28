# Project Overview

<br/>![Linx logo](doc/diagrams/logo_square.svg)

# Purpose

This project is a work in progress to gather ND array related features
of [EleFits](https://cnes.github.io/EleFits/)' `Raster` -- an extensible ND data storage class with pixel-wise operations --
and PhiFun's operations -- e.g. Fourier transforms, linear filtering, and interpolation.

The target is a header-only library focused on ease of use
and which interfaces seamlessly with the standard C++ library.
At some point, the ND array implementation of EleFits should be droped in favor of Linx.

The image processing feature set is expected to grow steadily on demand.
Currently supported operations are:

* Pixel-wise operations (`+`, `*`, `exp()`, `apply()`...) on `Raster`s and on `Raster` regions;
* Discrete Fourier transforms (FFTW wrapper) through the `DftPlan` class;
* Linear filtering through the `Kernel` class;
* Morphology and rank-order filtering through the `StructuringElement` class;
* Interpolation and extrapolation (`NearestNeighbor`, `Linear`, `OutOfBoundsConstant`...);
* Affine transformations.

# License

[LGPL-3.0-or-later](LICENSE.md)

# Installation

The simplest way to use Linx, which is a header-only library, is to copy the deepest `Linx` folder into your include directory:

```sh
git clone https://github.com/kabasset/Linx.git
cp -r Linx/Linx/Linx /usr/include/
```

# Alternatives

The following libraries offer features similar to Linx.
Most of the libraries which support ND arrays are focused on linear algebra,
while most image processing libraries limit the number of axes.
ITK (and SimpleITK) is the notable exception.
Linx aims at being simpler, less verbose and more extensible,
although with a more limited feature set.

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

Here is a quick comparison of ITK, CImg, Linx and NumPy/SciKit of the same use case:
read an image, dilate it with an L2-ball structuring element, and write the output.

ITK:

```cpp
using PixelType = unsigned char;
constexpr unsigned int Dimension = 2;

using ImageType = itk::Image<PixelType, Dimension>;
using ReaderType = itk::ImageFileReader<ImageType>;
ReaderType::Pointer reader = ReaderType::New();
reader->SetFileName(input);

using StructuringElementType = itk::FlatStructuringElement<Dimension>;
StructuringElementType::RadiusType strelRadius;
strelRadius.Fill(radius);
StructuringElementType ball = StructuringElementType::Ball(strelRadius);
using BinaryDilateImageFilterType = itk::BinaryDilateImageFilter<ImageType, ImageType, StructuringElementType>;
BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
dilateFilter->SetInput(reader->GetOutput());
dilateFilter->SetKernel(ball);

itk::ImageFileWriter<ImageType>::Pointer writer;
writer = itk::ImageFileWriter<ImageType>::New();
writer->SetInput(dilateFilter->GetOutput());
writer->SetFileName(output);
writer->Update();
```

CImg:

```cpp
using PixelType = unsigned char;

const auto raw = cimg::CImg<PixelType>().load(input);

cimg::CImg<bool> ball(2 * radius + 1, 2 * radius + 1, 1, 1, false);
bool[1] color = {true};
ball.draw_disk(radius, radius, radius, color);
const auto dilated = raw.get_dilate(ball);

dilated.write(output);
```

Linx:


```cpp
using PixelType = unsigned char;
constexpr Linx::Index Dimension = 2;

const auto raw = Linx::read<PixelType, Dimension>(input);

const auto ball = Linx::Mask<Dimension>::ball<2>(radius);
const auto dilated = dilate(ball) * raw;

Linx::write(dilated, output);
```

NumPy/SciKit:

```python
raw = np.load(input)

ball = skimage.morphology.disk(radius)
dilated = skimage.morphology.binary_dilation(raw, ball)

np.save(output, dilated)
```