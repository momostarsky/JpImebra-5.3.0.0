/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef GDCMSWAPPER_H
#define GDCMSWAPPER_H

namespace imebra {

    namespace implementation {

        namespace codecs {
            class SwapperNoOp {
            public:
                template<typename T>
                static T Swap(T val) { return val; }

                template<typename T>
                static void SwapArray(T *, size_t) {}
            };

            class SwapperDoOp {
            public:
                template<typename T>
                static T Swap(T val);

                template<typename T>
                static void SwapArray(T *array, size_t n) {

                    for (size_t i = 0; i < n; ++i) {
                        array[i] = Swap<T>(array[i]);
                    }
                }
            };
        }
    }

} // end namespace gdcm

#include "gdcmSwapper.cpp"

#endif //GDCMSWAPPER_H
