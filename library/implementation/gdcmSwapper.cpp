/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef GDCMSWAPPER_TXX
#define GDCMSWAPPER_TXX
#include <bits/byteswap.h>
#include "assert.h"
/* The following definitions must all be macros since otherwise some
   of the possible optimizations are not possible.  */

/* Return a value with all bytes in the 16 bit argument swapped.  */
#define bswap_16(x) __bswap_16 (x)

/* Return a value with all bytes in the 32 bit argument swapped.  */
#define bswap_32(x) __bswap_32 (x)

/* Return a value with all bytes in the 64 bit argument swapped.  */
#define bswap_64(x) __bswap_64 (x)

namespace imebra {

    namespace implementation {

        namespace codecs {
            template<>
            inline uint16_t SwapperDoOp::Swap<uint16_t>(uint16_t val) {
                return bswap_16(val);
            }

            template<>
            inline int16_t SwapperDoOp::Swap<int16_t>(int16_t val) {
                return Swap((uint16_t) val);
            }

            template<>
            inline uint32_t SwapperDoOp::Swap<uint32_t>(uint32_t val) {
                return bswap_32(val);
            }

            template<>
            inline int32_t SwapperDoOp::Swap<int32_t>(int32_t val) {
                return Swap((uint32_t) val);
            }

            template<>
            inline float SwapperDoOp::Swap<float>(float val) {
                return static_cast<float>(Swap((uint32_t) val));
            }

            template<>
            inline uint64_t SwapperDoOp::Swap<uint64_t>(uint64_t val) {
                return bswap_64(val);
            }

            template<>
            inline int64_t SwapperDoOp::Swap<int64_t>(int64_t val) {
                return Swap((uint64_t) val);
            }

            template<>
            inline double SwapperDoOp::Swap<double>(double val) {
                return static_cast<double>(Swap((uint64_t) val));
            }


            template<>
            inline void SwapperDoOp::SwapArray(uint8_t *, size_t) {}

            template<>
            inline void SwapperDoOp::SwapArray(float *array, size_t n) {
                switch (sizeof(float)) {
                    case 4:
                        SwapperDoOp::SwapArray<uint32_t>((uint32_t *) array, n);
                        break;
                    default:
                        assert(0);
                }
            }

            template<>
            inline void SwapperDoOp::SwapArray(double *array, size_t n) {
                switch (sizeof(double)) {
                    case 8:
                        SwapperDoOp::SwapArray<uint64_t>((uint64_t *) array, n);
                        break;
                    default:
                        assert(0);
                }
            }

        } // end namespace gdcm

    }
}
#endif
