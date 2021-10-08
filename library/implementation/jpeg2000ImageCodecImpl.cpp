/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpeg2000ImageCodec.cpp
    \brief Implementation of the class jpeg2000ImageCodec.

*/

#ifdef JPEG2000

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryStreamImpl.h"
#include "huffmanTableImpl.h"
#include "jpeg2000ImageCodecImpl.h"
#include "dataSetImpl.h"
#include "imageImpl.h"
#include "dataHandlerNumericImpl.h"
#include "codecFactoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include "jpeg2000Helper.h"


namespace imebra {

    namespace implementation {

        namespace codecs {
            class JPEG2000Internals {
            public:
                JPEG2000Internals()
                        : nNumberOfThreadsForDecompression(2) {
                    memset(&coder_param, 0, sizeof(coder_param));
                    opj_set_default_encoder_parameters(&coder_param);
                    coder_param.cp_fixed_quality = 1;
                    coder_param.cp_disto_alloc = 1;
                }

                opj_cparameters coder_param;
                int nNumberOfThreadsForDecompression;
            };


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the specified transfer
//  syntax
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
            bool jpeg2000ImageCodec::canHandleTransferSyntax(const std::string &transferSyntax) const {
                IMEBRA_FUNCTION_START() ;

                    return (transferSyntax == uidJPEG2000ImageCompressionLosslessOnly_1_2_840_10008_1_2_4_90 ||
                            transferSyntax == uidJPEG2000ImageCompression_1_2_840_10008_1_2_4_91);

                IMEBRA_FUNCTION_END();
            }


            bool jpeg2000ImageCodec::defaultInterleaved() const {
                return false;
            }

            jpeg2000ImageCodec::jpeg2000ImageCodec() {
                Internals = new JPEG2000Internals;
            }

            jpeg2000ImageCodec::~jpeg2000ImageCodec() {
                delete Internals;
            }


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the transfer syntax has to be
//  encapsulated
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
            bool jpeg2000ImageCodec::encapsulated(const std::string &transferSyntax) const {
                IMEBRA_FUNCTION_START() ;

                    if (!canHandleTransferSyntax(transferSyntax)) {
                        IMEBRA_THROW(CodecWrongTransferSyntaxError, "Cannot handle the transfer syntax");
                    }
                    return true;

                IMEBRA_FUNCTION_END();
            }


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the suggested allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
            std::uint32_t jpeg2000ImageCodec::suggestAllocatedBits(const std::string &transferSyntax,
                                                                   std::uint32_t highBit) const {
                IMEBRA_FUNCTION_START() ;


                    std::uint32_t hb = highBit + 1;
                    if (canHandleTransferSyntax(transferSyntax)) {
                        if (hb <= 8) {
                            return 8;
                        } else if (hb <= 16) {
                            return 16;
                        } else if (hb <= 32) {
                            return 32;
                        } else {
                            IMEBRA_THROW(DataSetUnknownTransferSyntaxError, "highBit  value out of range ");
                        }

                    } else {
                        IMEBRA_THROW(DataSetUnknownTransferSyntaxError,
                                     "Only Support 1.2.840.10008.1.2.4.90  and 1.2.840.10008.1.2.4.91");

                    }

                IMEBRA_FUNCTION_END();
            }


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Get a jpeg image from a Dicom dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
            std::shared_ptr<image> jpeg2000ImageCodec::getImage(const std::string &/*transferSyntax*/,
                                                                const std::string &imageColorSpace,
                                                                std::uint32_t /*channelsNumber*/,
                                                                std::uint32_t imageWidth,
                                                                std::uint32_t imageHeight,
                                                                bool /*bSubsampledX*/,
                                                                bool /*bSubsampledY*/,
                                                                bool /*bInterleaved*/,
                                                                bool b2Complement,
                                                                std::uint8_t /*allocatedBits*/,
                                                                std::uint8_t /*storedBits*/,
                                                                std::uint8_t highBit,
                                                                std::shared_ptr<streamReader> pStream) const {
                IMEBRA_FUNCTION_START() ;

#ifdef JPEG2000_V1
                    CODEC_FORMAT format = (CODEC_FORMAT)(CODEC_J2K);
#else
                    CODEC_FORMAT format = (CODEC_FORMAT) (OPJ_CODEC_J2K);
#endif

                    std::shared_ptr<memory> jpegMemory(std::make_shared<memory>());
                    {
                        std::shared_ptr<memoryStreamOutput> memoryStream(
                                std::make_shared<memoryStreamOutput>(jpegMemory));
                        streamWriter memoryStreamWriter(memoryStream);
                        unsigned char tempBuffer[4096];
                        while (!pStream->endReached()) {
                            size_t readLength = pStream->readSome(tempBuffer, sizeof(tempBuffer));
                            memoryStreamWriter.write(tempBuffer, readLength);
                        }
                    }

                    opj_dparameters_t parameters;
                    opj_set_default_decoder_parameters(&parameters);
#ifdef JPEG2000_V1
                    opj_dinfo_t *dinfo;
#else
                    opj_codec_t *dinfo;
#endif

                    dinfo = opj_create_decompress(format);
                    opj_setup_decoder(dinfo, &parameters);
                    opj_image_t *jp2image(nullptr);

#ifdef JPEG2000_V1
                    opj_cio_t* cio = opj_cio_open((opj_common_ptr)dinfo, jpegMemory->data(), (int)jpegMemory->size());
                    if(cio != 0)
                    {
                        jp2image = opj_decode(dinfo, cio);
                        opj_cio_close(cio);
                    }
#else
                    opj_memory_stream memoryStream;
                    memoryStream.pData = jpegMemory->data();
                    memoryStream.dataSize = jpegMemory->size();
                    memoryStream.offset = 0;

                    opj_stream_t *pJpeg2000Stream = opj_stream_create_default_memory_stream(&memoryStream, true);
                    if (pJpeg2000Stream != nullptr) {
                        if (opj_read_header(pJpeg2000Stream, dinfo, &jp2image)) {
                            if (!opj_decode(dinfo, pJpeg2000Stream, jp2image)) {
                                opj_image_destroy(jp2image);
                                jp2image = nullptr;
                            }
                        }
                        opj_stream_destroy(pJpeg2000Stream);
                    }
                    opj_destroy_codec(dinfo);
#endif
                    if (jp2image == nullptr) {
                        IMEBRA_THROW(CodecCorruptedFileError,  "JPEG2000 Decode Failed!");
                    }

//    std::uint32_t width(sourceDataSet.getUint32(0x0028, 0, 0x0011, 0, 0, 0));
//    std::uint32_t height(sourceDataSet.getUint32(0x0028, 0, 0x0010, 0, 0, 0));
//    std::string colorSpace(sourceDataSet.getString(0x0028, 0, 0x0004, 0, 0, ""));


                    std::uint32_t width(imageWidth);
                    std::uint32_t height(imageHeight);

                    std::string colorSpace(imageColorSpace);

                    bitDepth_t depth;
                    if (b2Complement) {
                        if (highBit >= 16) {
                            depth = bitDepth_t::depthS32;
                        } else if (highBit >= 8) {
                            depth = bitDepth_t::depthS16;
                        } else {
                            depth = bitDepth_t::depthS8;
                        }
                    } else {
                        if (highBit >= 16) {
                            depth = bitDepth_t::depthU32;
                        } else if (highBit >= 8) {
                            depth = bitDepth_t::depthU16;
                        } else {
                            depth = bitDepth_t::depthU8;
                        }
                    }


                    std::shared_ptr<image> returnImage(
                            std::make_shared<image>(
                                    width,
                                    height,
                                    depth,
                                    colorSpace,
                                    highBit));

                    std::shared_ptr<handlers::writingDataHandlerNumericBase> imageHandler(
                            returnImage->getWritingDataHandler());

                    ::memset(imageHandler->getMemory()->data(), 0, imageHandler->getSize());

                    // Copy channels
                    for (unsigned int channelNumber(0); channelNumber != jp2image->numcomps; ++channelNumber) {
                        const opj_image_comp_t &channelData = jp2image->comps[channelNumber];

                        imageHandler->copyFromInt32Interleaved(channelData.data,
                                                               channelData.dx,
                                                               channelData.dy,
                                                               channelData.x0,
                                                               channelData.y0,
                                                               channelData.x0 + channelData.w * channelData.dx,
                                                               channelData.y0 + channelData.h * channelData.dy,
                                                               channelNumber,
                                                               width,
                                                               height,
                                                               jp2image->numcomps);
                    }

                    opj_image_destroy(jp2image);

                    return returnImage;


                IMEBRA_FUNCTION_END();
            }


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write an image into the dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
            void jpeg2000ImageCodec::setImage(
                    std::shared_ptr<streamWriter> pDestStream,
                    std::shared_ptr<const image> pSourceImage,
                    const std::string &,
                    imageQuality_t,
                    std::uint32_t allocatedBits,
                    bool,
                    bool,
                    bool,
                    bool b2Complement) const {
                IMEBRA_FUNCTION_START() ;

                    IMEBRA_THROW(DataSetUnknownTransferSyntaxError,
                                 "None of the codecs support the specified transfer syntax");
                    const uint32_t highBit = pSourceImage->getHighBit();
                    std::shared_ptr<handlers::readingDataHandlerNumericBase> dataReader = pSourceImage->getReadingDataHandler();

                    size_t sz = dataReader->getMemorySize();
                    const std::uint8_t *buffer = dataReader->getMemoryBuffer();
                    uint32_t image_width = 0;
                    uint32_t image_height = 0;
                    pSourceImage->getSize(&image_width, &image_height);

                    std::string colorSpace = pSourceImage->getColorSpace();
                    uint32_t sample_pixel = pSourceImage->getChannelsNumber();
                    uint32_t bitsallocated = allocatedBits;

                    // Most significant bit for pixel sample data. Each sample shall have the same high bit.
                    // High Bit (0028,0102) shall be one less than Bits Stored (0028,0101).
                    // See PS3.5 for further explanation.
                    uint32_t bitsstored = highBit + 1;// this ->suggestAllocatedBits(transferSyntax, highBit) ;
                    // bool b2Complement(getUint32(0x0028, 0x0, 0x0103, 0, 0, 0) != 0);
                    int pixelRepresentation = 0;
                    if (b2Complement) {
                        pixelRepresentation = 1;
                    } else {
                        pixelRepresentation = 0;
                    }

                    int pcx = 0;
                    if (pSourceImage->getColorSpace() == "PALETTE COLOR") {
                        pcx = 1;
                    }


                    std::vector<char> rgbyteCompressed;
                    rgbyteCompressed.resize(image_width * image_height * 4);
                    size_t cbyteCompressed;
                    bool b = codeFrameIntoBuffer((char *) &rgbyteCompressed[0], rgbyteCompressed.size(),
                                                 cbyteCompressed, reinterpret_cast<const char *>(buffer), sz,
                                                 static_cast<int>(image_width),
                                                 static_cast<int>(image_height),
                                                 static_cast<int>(sample_pixel),
                                                 static_cast<int>(bitsallocated),
                                                 static_cast<int>(bitsstored),
                                                 (int) highBit,
                                                 pixelRepresentation,
                                                 Internals->coder_param,
                                                 pcx
                    );
                    if (!b) {
                        IMEBRA_THROW(DataSetUnknownTransferSyntaxError,
                                     " change to  JPEG2000 failed !");
                    }
                    pDestStream->write(reinterpret_cast<const uint8_t *>(&rgbyteCompressed[0]),
                                       (uint32_t) cbyteCompressed);


                IMEBRA_FUNCTION_END();
            }


        } // namespace codecs

    } // namespace implementation

} // namespace imebra

#endif //JPEG2000
