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
#include <ostream>
#include <cstdio>



namespace imebra {

    namespace implementation {

        namespace codecs {

            extern  "C"
            {
                #include  <openjpeg-2.1/openjpeg.h>
            }

            typedef struct
            {
                OPJ_UINT8* pData; //Our data.
                OPJ_SIZE_T dataSize; //How big is our data.
                OPJ_SIZE_T offset; //Where are we currently in our data.
            } opj_memory_stream;


// Read from memory stream
            static OPJ_SIZE_T opj_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
            {
                opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.
                OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;//Amount to move to buffer.

                //Check if the current offset is outside our data buffer.
                if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T)-1;

                //Check if we are reading more than we have.
                if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
                {
                    l_nb_bytes_read = l_memory_stream->dataSize - l_memory_stream->offset;//Read all we have.
                }

                //Copy the data to the internal buffer.
                memcpy(p_buffer, &(l_memory_stream->pData[l_memory_stream->offset]), l_nb_bytes_read);
                l_memory_stream->offset += l_nb_bytes_read;//Update the pointer to the new location.
                return l_nb_bytes_read;
            }

//This will write from the buffer to our memory.
            static OPJ_SIZE_T opj_memory_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
            {
                opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.
                OPJ_SIZE_T l_nb_bytes_write = p_nb_bytes;//Amount to move to buffer.

                //Check if the current offset is outside our data buffer.
                if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T)-1;

                //Check if we are write more than we have space for.
                if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
                {
                    l_nb_bytes_write = l_memory_stream->dataSize - l_memory_stream->offset;//Write the remaining space.
                }

                //Copy the data from the internal buffer.
                memcpy(&(l_memory_stream->pData[l_memory_stream->offset]), p_buffer, l_nb_bytes_write);
                l_memory_stream->offset += l_nb_bytes_write;//Update the pointer to the new location.
                return l_nb_bytes_write;
            }

//Moves the pointer forward, but never more than we have.
            static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
            {
                opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;
                OPJ_SIZE_T l_nb_bytes;

                if (p_nb_bytes < 0) return -1;//No skipping backwards.
                {
                    l_nb_bytes = (OPJ_SIZE_T)p_nb_bytes;//Allowed because it is positive.
                }

                // Do not allow jumping past the end.
                if (l_nb_bytes >l_memory_stream->dataSize - l_memory_stream->offset)
                {
                    l_nb_bytes = l_memory_stream->dataSize - l_memory_stream->offset;//Jump the max.
                }

                //Make the jump.
                l_memory_stream->offset += l_nb_bytes;

                //Returm how far we jumped.
                return l_nb_bytes;
            }

//Sets the pointer to anywhere in the memory.
            static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data)
            {
                opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;

                if (p_nb_bytes < 0)
                {
                    return OPJ_FALSE;
                }

                if (p_nb_bytes >(OPJ_OFF_T)l_memory_stream->dataSize)
                {
                    return OPJ_FALSE;
                }

                l_memory_stream->offset = (OPJ_SIZE_T)p_nb_bytes;//Move to new position.
                return OPJ_TRUE;
            }

//The system needs a routine to do when finished, the name tells you what I want it to do.
            static void opj_memory_stream_do_nothing(void * /* p_user_data */)
            {
            }

//Create a stream to use memory as the input or output.
            opj_stream_t* opj_stream_create_default_memory_stream(opj_memory_stream* p_memoryStream, OPJ_BOOL p_is_read_stream)
            {
                opj_stream_t* l_stream;

                if (!(l_stream = opj_stream_default_create(p_is_read_stream)))
                {
                    return (NULL);
                }

                //Set how to work with the frame buffer.
                if(p_is_read_stream)
                {
                    opj_stream_set_read_function(l_stream, opj_memory_stream_read);
                }
                else
                {
                    opj_stream_set_write_function(l_stream, opj_memory_stream_write);
                }

                opj_stream_set_seek_function(l_stream, opj_memory_stream_seek);
                opj_stream_set_skip_function(l_stream, opj_memory_stream_skip);
                opj_stream_set_user_data(l_stream, p_memoryStream, opj_memory_stream_do_nothing);
                opj_stream_set_user_data_length(l_stream, p_memoryStream->dataSize);
                return l_stream;
            }


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


                    return (
                            transferSyntax == uidJPEG2000ImageCompression_1_2_840_10008_1_2_4_91
                            || transferSyntax == uidJPEG2000ImageCompressionLosslessOnly_1_2_840_10008_1_2_4_90
                            || transferSyntax ==
                               uidJPEG2000Part2MulticomponentImageCompressionLosslessOnly_1_2_840_10008_1_2_4_92
                            || transferSyntax == uidJPEG2000Part2MulticomponentImageCompression_1_2_840_10008_1_2_4_93

                    );

                IMEBRA_FUNCTION_END();
            }


            bool jpeg2000ImageCodec::defaultInterleaved() const {
                return false;
            }

            jpeg2000ImageCodec::jpeg2000ImageCodec() {
                jpeg2000Codec = new gdcm::JPEG2000Codec();
            }

            jpeg2000ImageCodec::~jpeg2000ImageCodec() {
                delete jpeg2000Codec;
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
                                                                std::uint8_t allocatedBits,
                                                                std::uint8_t /*storedBits*/,
                                                                std::uint8_t highBit,
                                                                std::shared_ptr<streamReader> pStream) const {
                IMEBRA_FUNCTION_START() ;





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



                    CODEC_FORMAT format = (CODEC_FORMAT)(OPJ_CODEC_J2K);

                    std::shared_ptr<memory> jpegMemory(std::make_shared<memory>());
                    {
                        std::shared_ptr<memoryStreamOutput> memoryStream(std::make_shared<memoryStreamOutput>(jpegMemory));
                        streamWriter memoryStreamWriter(memoryStream);
                        unsigned char tempBuffer[4096];
                        while(!pStream->endReached())
                        {
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
                    opj_image_t* jp2image(0);

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

                    opj_stream_t* pJpeg2000Stream = opj_stream_create_default_memory_stream(&memoryStream, true);
                    if(pJpeg2000Stream != 0)
                    {
                        if(opj_read_header(pJpeg2000Stream, dinfo, &jp2image))
                        {
                            if(!opj_decode(dinfo, pJpeg2000Stream, jp2image))
                            {
                                opj_image_destroy(jp2image);
                                jp2image = 0;
                            }
                        }
                        opj_stream_destroy(pJpeg2000Stream);
                    }
                    opj_destroy_codec(dinfo);
#endif
                    if(jp2image == 0)
                    {
                        IMEBRA_THROW(CodecCorruptedFileError, "Could not decode the jpeg2000 image");
                    }

                    std::uint32_t width(imageWidth);
                    std::uint32_t height(imageHeight);

                    std::string colorSpace(imageColorSpace);

                    std::shared_ptr<image> returnImage(
                            std::make_shared<image>(
                                    width,
                                    height,
                                    depth,
                                    colorSpace,
                                    highBit));

                    std::shared_ptr<handlers::writingDataHandlerNumericBase> imageHandler(returnImage->getWritingDataHandler());

                    ::memset(imageHandler->getMemory()->data(), 0, imageHandler->getSize());

                    // Copy channels
                    for(unsigned int channelNumber(0); channelNumber != jp2image->numcomps; ++channelNumber)
                    {
                        const opj_image_comp_t& channelData = jp2image->comps[channelNumber];

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
                    bool /*bSubSampledX*/,
                    bool /*bSubSampledY*/,
                    bool /*bInterleaved*/,
                    bool b2Complement) const {
                IMEBRA_FUNCTION_START() ;

                    const uint32_t highBit = pSourceImage->getHighBit();
                    std::shared_ptr<handlers::readingDataHandlerNumericBase> dataReader = pSourceImage->getReadingDataHandler();

                    size_t sz = dataReader->getMemorySize();
                    const std::uint8_t *buffer = dataReader->getMemoryBuffer();
                    uint32_t image_width = 0;
                    uint32_t image_height = 0;
                    pSourceImage->getSize(&image_width, &image_height);

                    std::string colorSpace = pSourceImage->getColorSpace();


                    std::vector<char> rgbyteCompressed;
                    rgbyteCompressed.resize(image_width * image_height * 4);
                    size_t cbyteCompressed;
                    bool b =jpeg2000Codec->CodeFrameIntoBuffer((char *) &rgbyteCompressed[0], rgbyteCompressed.size(),
                                                 cbyteCompressed, reinterpret_cast<const char *>(buffer), sz
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
