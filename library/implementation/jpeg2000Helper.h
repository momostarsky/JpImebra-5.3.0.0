//
// Created by dhz on 2021/7/30.
//

#ifndef IMEBRA_JPEG2000HELPER_H
#define IMEBRA_JPEG2000HELPER_H

#include "gdcmSwapper.h"
#include <openjpeg-2.3/openjpeg.h>

#define  gdcmErrorMacro(msg) {}
#define  gdcmWarningMacro(msg) {}
#define  gdcmDebugMacro(msg){}
namespace imebra {

    namespace implementation {

        namespace codecs {
            extern "C"
            {



            void opj_error_callback(const char */*msg*/, void */*usr*/) {
                //Dicom::Debug::Log->Error("OpenJPEG: {0}", gcnew String(msg));
            }

            void opj_warning_callback(const char */*msg*/, void *) {
                //Dicom::Debug::Log->Warn("OpenJPEG: {0}", gcnew String(msg));
            }

            void opj_info_callback(const char */*msg*/, void *) {
                //Dicom::Debug::Log->Info("OpenJPEG: {0}", gcnew String(msg));
            }

            }



// Represents a Jpeg2000 memory stream information
            typedef struct {
                OPJ_UINT8 *pData; //Our data.
                OPJ_SIZE_T dataSize; //How big is our data.
                OPJ_SIZE_T offset; //Where are we currently in our data.
            } opj_memory_stream;


// Read from memory stream
            static OPJ_SIZE_T opj_memory_stream_read(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user_data) {
                opj_memory_stream *l_memory_stream = (opj_memory_stream *) p_user_data;//Our data.
                OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;//Amount to move to buffer.

                //Check if the current offset is outside our data buffer.
                if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T) - 1;

                //Check if we are reading more than we have.
                if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset)) {
                    l_nb_bytes_read = l_memory_stream->dataSize - l_memory_stream->offset;//Read all we have.
                }

                //Copy the data to the internal buffer.
                memcpy(p_buffer, &(l_memory_stream->pData[l_memory_stream->offset]), l_nb_bytes_read);
                l_memory_stream->offset += l_nb_bytes_read;//Update the pointer to the new location.
                return l_nb_bytes_read;
            }

//This will write from the buffer to our memory.
            static OPJ_SIZE_T opj_memory_stream_write(void *p_buffer, OPJ_SIZE_T p_nb_bytes, void *p_user_data) {
                opj_memory_stream *l_memory_stream = (opj_memory_stream *) p_user_data;//Our data.
                OPJ_SIZE_T l_nb_bytes_write = p_nb_bytes;//Amount to move to buffer.

                //Check if the current offset is outside our data buffer.
                if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T) - 1;

                //Check if we are write more than we have space for.
                if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset)) {
                    l_nb_bytes_write = l_memory_stream->dataSize - l_memory_stream->offset;//Write the remaining space.
                }

                //Copy the data from the internal buffer.
                memcpy(&(l_memory_stream->pData[l_memory_stream->offset]), p_buffer, l_nb_bytes_write);
                l_memory_stream->offset += l_nb_bytes_write;//Update the pointer to the new location.
                return l_nb_bytes_write;
            }

//Moves the pointer forward, but never more than we have.
            static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void *p_user_data) {
                opj_memory_stream *l_memory_stream = (opj_memory_stream *) p_user_data;
                OPJ_SIZE_T l_nb_bytes;

                if (p_nb_bytes < 0) return -1;//No skipping backwards.
                {
                    l_nb_bytes = (OPJ_SIZE_T) p_nb_bytes;//Allowed because it is positive.
                }

                // Do not allow jumping past the end.
                if (l_nb_bytes > l_memory_stream->dataSize - l_memory_stream->offset) {
                    l_nb_bytes = l_memory_stream->dataSize - l_memory_stream->offset;//Jump the max.
                }

                //Make the jump.
                l_memory_stream->offset += l_nb_bytes;

                //Returm how far we jumped.
                return l_nb_bytes;
            }

//Sets the pointer to anywhere in the memory.
            static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void *p_user_data) {
                opj_memory_stream *l_memory_stream = (opj_memory_stream *) p_user_data;

                if (p_nb_bytes < 0) {
                    return OPJ_FALSE;
                }

                if (p_nb_bytes > (OPJ_OFF_T) l_memory_stream->dataSize) {
                    return OPJ_FALSE;
                }

                l_memory_stream->offset = (OPJ_SIZE_T) p_nb_bytes;//Move to new position.
                return OPJ_TRUE;
            }

//The system needs a routine to do when finished, the name tells you what I want it to do.
            static void opj_memory_stream_do_nothing(void * /* p_user_data */) {
            }

//Create a stream to use memory as the input or output.
            opj_stream_t *
            opj_stream_create_default_memory_stream(opj_memory_stream *p_memoryStream, OPJ_BOOL p_is_read_stream) {
                opj_stream_t *l_stream;

                if (!(l_stream = opj_stream_default_create(p_is_read_stream))) {
                    return (NULL);
                }

                //Set how to work with the frame buffer.
                if (p_is_read_stream) {
                    opj_stream_set_read_function(l_stream, opj_memory_stream_read);
                } else {
                    opj_stream_set_write_function(l_stream, opj_memory_stream_write);
                }

                opj_stream_set_seek_function(l_stream, opj_memory_stream_seek);
                opj_stream_set_skip_function(l_stream, opj_memory_stream_skip);
                opj_stream_set_user_data(l_stream, p_memoryStream, opj_memory_stream_do_nothing);
                opj_stream_set_user_data_length(l_stream, p_memoryStream->dataSize);
                return l_stream;
            }





            /* Part 1  Table A.2 List of markers and marker segments */
            typedef enum {
                FF30 = 0xFF30,
                FF31 = 0xFF31,
                FF32 = 0xFF32,
                FF33 = 0xFF33,
                FF34 = 0xFF34,
                FF35 = 0xFF35,
                FF36 = 0xFF36,
                FF37 = 0xFF37,
                FF38 = 0xFF38,
                FF39 = 0xFF39,
                FF3A = 0xFF3A,
                FF3B = 0xFF3B,
                FF3C = 0xFF3C,
                FF3D = 0xFF3D,
                FF3E = 0xFF3E,
                FF3F = 0xFF3F,
                SOC = 0xFF4F,
                CAP = 0xFF50,
                SIZ = 0xFF51,
                COD = 0xFF52,
                COC = 0xFF53,
                TLM = 0xFF55,
                PLM = 0XFF57,
                PLT = 0XFF58,
                QCD = 0xFF5C,
                QCC = 0xFF5D,
                RGN = 0xFF5E,
                POC = 0xFF5F,
                PPM = 0XFF60,
                PPT = 0XFF61,
                CRG = 0xFF63,
                COM = 0xFF64,
                SOT = 0xFF90,
                SOP = 0xFF91,
                EPH = 0XFF92,
                SOD = 0xFF93,
                EOC = 0XFFD9  /* EOI in old jpeg */
            } MarkerType;

            typedef enum {
                JP = 0x6a502020,
                FTYP = 0x66747970,
                JP2H = 0x6a703268,
                JP2C = 0x6a703263,
                JP2 = 0x6a703220,
                IHDR = 0x69686472,
                COLR = 0x636f6c72,
                XML = 0x786d6c20,
                CDEF = 0x63646566,
                CMAP = 0x636D6170,
                PCLR = 0x70636c72,
                RES = 0x72657320
            } OtherType;

            static inline bool hasnolength(uint_fast16_t marker) {
                switch (marker) {
                    case FF30:
                    case FF31:
                    case FF32:
                    case FF33:
                    case FF34:
                    case FF35:
                    case FF36:
                    case FF37:
                    case FF38:
                    case FF39:
                    case FF3A:
                    case FF3B:
                    case FF3C:
                    case FF3D:
                    case FF3E:
                    case FF3F:
                    case SOC:
                    case SOD:
                    case EOC:
                    case EPH:
                        return true;
                }
                return false;
            }


            static inline bool read16(const char **input, size_t *len, uint16_t *ret) {
                if (*len >= 2) {
                    union {
                        uint16_t v;
                        char bytes[2];
                    } u;
                    memcpy(u.bytes, *input, 2);
                    *ret = SwapperDoOp::Swap(u.v);
                    *input += 2;
                    *len -= 2;
                    return true;
                }
                return false;
            }


            static inline bool read32(const char **input, size_t *len, uint32_t *ret) {
                if (*len >= 4) {
                    union {
                        uint32_t v;
                        char bytes[4];
                    } u;
                    memcpy(u.bytes, *input, 4);
                    *ret = SwapperDoOp::Swap(u.v);
                    *input += 4;
                    *len -= 4;
                    return true;
                }
                return false;
            }

            static inline bool read64(const char **input, size_t *len, uint64_t *ret) {
                if (*len >= 8) {
                    union {
                        uint64_t v;
                        char bytes[8];
                    } u;
                    memcpy(u.bytes, *input, 8);
                    *ret = SwapperDoOp::Swap(u.v);
                    *input += 8;
                    *len -= 8;
                    return true;
                }
                return false;
            }


            static bool parsej2k_imp(const char *const stream, const size_t file_size, bool *lossless, bool *mct) {
                uint16_t marker;
                size_t lenmarker;
                const char *cur = stream;
                size_t cur_size = file_size;
                *lossless = false; // default init
                while (read16(&cur, &cur_size, &marker)) {
                    if (!hasnolength(marker)) {
                        uint16_t l;
                        bool r = read16(&cur, &cur_size, &l);
                        if (!r || l < 2)
                            break;
                        lenmarker = (size_t) l - 2;

                        if (marker == COD) {
                            const uint8_t MCTransformation = *(cur + 4);
                            if (MCTransformation == 0x0) *mct = false;
                            else if (MCTransformation == 0x1) *mct = true;
                            else return false;
                            const uint8_t Transformation = *(cur + 9);
                            if (Transformation == 0x0) {
                                *lossless = false;
                                return true;
                            } else if (Transformation == 0x1) *lossless = true;
                            else return false;
                        }
                        cur += lenmarker;
                        cur_size -= lenmarker;
                    } else if (marker == SOD)
                        return true;
                }
                return false;
            }

            __attribute__((unused)) __attribute__((unused)) __attribute__((unused)) static bool parsejp2_imp(const char *const stream, const size_t file_size, bool *lossless, bool *mct) {
                uint32_t marker;
                uint64_t len64; /* ref */
                uint32_t len32; /* local 32bits op */
                const char *cur = stream;
                size_t cur_size = file_size;

                while (read32(&cur, &cur_size, &len32)) {
                    bool b0 = read32(&cur, &cur_size, &marker);
                    if (!b0) break;
                    len64 = len32;
                    if (len32 == 1) /* 64bits ? */
                    {
                        bool b = read64(&cur, &cur_size, &len64);
                        assert(b);
                        (void) b;
                        len64 -= 8;
                    }
                    if (marker == JP2C) {
                        const size_t start = cur - stream;
                        if (!len64) {
                            len64 = (size_t) (file_size - start + 8);
                        }
                        assert(len64 >= 8);
                        return parsej2k_imp(cur, (size_t) (len64 - 8), lossless, mct);
                    }
                    const size_t lenmarker = (size_t) (len64 - 8);
                    cur += lenmarker;
                }

                return false;
            }


            /**
            sample error callback expecting a FILE* client object
            */
            void error_callback(const char *msg, void *) {
                (void) msg;
                gdcmErrorMacro("Error in gdcmopenjpeg" << msg);
            }

            /**
            sample warning callback expecting a FILE* client object
            */
            void warning_callback(const char *msg, void *) {
                (void) msg;
                gdcmWarningMacro("Warning in gdcmopenjpeg" << msg);
            }

            /**
            sample debug callback expecting no client object
            */
            void info_callback(const char *msg, void *) {
                (void) msg;
                gdcmDebugMacro("Info in gdcmopenjpeg" << msg);
            }

#define J2K_CFMT 0
#define JP2_CFMT 1
#define JPT_CFMT 2

#define PXM_DFMT 10
#define PGX_DFMT 11
#define BMP_DFMT 12
#define YUV_DFMT 13
#define TIF_DFMT 14
#define RAW_DFMT 15
#define TGA_DFMT 16
#define PNG_DFMT 17
#define CODEC_JP2 OPJ_CODEC_JP2
#define CODEC_J2K OPJ_CODEC_J2K
#define CLRSPC_GRAY OPJ_CLRSPC_GRAY
#define CLRSPC_SRGB OPJ_CLRSPC_SRGB

            struct myfile {
                char *mem;
                char *cur;
                size_t len;
            };

            void gdcm_error_callback(const char *msg, void *) {
                fprintf(stderr, "%s", msg);
            }


            OPJ_SIZE_T opj_read_from_memory(void *p_buffer, OPJ_SIZE_T p_nb_bytes, myfile *p_file) {
                //OPJ_UINT32 l_nb_read = fread(p_buffer,1,p_nb_bytes,p_file);
                OPJ_SIZE_T l_nb_read;
                if (p_file->cur + p_nb_bytes <= p_file->mem + p_file->len) {
                    l_nb_read = 1 * p_nb_bytes;
                } else {
                    l_nb_read = (OPJ_SIZE_T) (p_file->mem + p_file->len - p_file->cur);
                    assert(l_nb_read < p_nb_bytes);
                }
                memcpy(p_buffer, p_file->cur, l_nb_read);
                p_file->cur += l_nb_read;
                assert(p_file->cur <= p_file->mem + p_file->len);
                //std::cout << "l_nb_read: " << l_nb_read << std::endl;
                return l_nb_read ? l_nb_read : ((OPJ_SIZE_T) -1);
            }

            OPJ_SIZE_T opj_write_from_memory(void *p_buffer, OPJ_SIZE_T p_nb_bytes, myfile *p_file) {
                //return fwrite(p_buffer,1,p_nb_bytes,p_file);
                OPJ_SIZE_T l_nb_write;
                //if( p_file->cur + p_nb_bytes < p_file->mem + p_file->len )
                //  {
                l_nb_write = 1 * p_nb_bytes;
                //  }
                //else
                //  {
                //  l_nb_write = p_file->mem + p_file->len - p_file->cur;
                //  assert( l_nb_write < p_nb_bytes );
                //  }
                memcpy(p_file->cur, p_buffer, l_nb_write);
                p_file->cur += l_nb_write;
                p_file->len += l_nb_write;
                //assert( p_file->cur < p_file->mem + p_file->len );
                return l_nb_write;
                //return p_nb_bytes;
            }

            OPJ_OFF_T opj_skip_from_memory(OPJ_OFF_T p_nb_bytes, myfile *p_file) {
                //if (fseek(p_user_data,p_nb_bytes,SEEK_CUR))
                //  {
                //  return -1;
                //  }
                if (p_file->cur + p_nb_bytes <= p_file->mem + p_file->len) {
                    p_file->cur += p_nb_bytes;
                    return p_nb_bytes;
                }

                p_file->cur = p_file->mem + p_file->len;
                return -1;
            }

            OPJ_BOOL opj_seek_from_memory(OPJ_OFF_T p_nb_bytes, myfile *p_file) {
                //if (fseek(p_user_data,p_nb_bytes,SEEK_SET))
                //  {
                //  return false;
                //  }
                //return true;
                assert(p_nb_bytes >= 0);
                if ((size_t) p_nb_bytes <= p_file->len) {
                    p_file->cur = p_file->mem + p_nb_bytes;
                    return OPJ_TRUE;
                }

                p_file->cur = p_file->mem + p_file->len;
                return OPJ_FALSE;
            }

            opj_stream_t *
            OPJ_CALLCONV opj_stream_create_memory_stream(myfile *p_mem, OPJ_SIZE_T p_size, bool p_is_read_stream) {
                opj_stream_t *l_stream = nullptr;
                if
                        (!p_mem) {
                    return nullptr;
                }
                l_stream = opj_stream_create(p_size, p_is_read_stream);
                if
                        (!l_stream) {
                    return nullptr;
                }
                opj_stream_set_user_data(l_stream, p_mem, nullptr);
                opj_stream_set_read_function(l_stream, (opj_stream_read_fn) opj_read_from_memory);
                opj_stream_set_write_function(l_stream, (opj_stream_write_fn) opj_write_from_memory);
                opj_stream_set_skip_function(l_stream, (opj_stream_skip_fn) opj_skip_from_memory);
                opj_stream_set_seek_function(l_stream, (opj_stream_seek_fn) opj_seek_from_memory);
                opj_stream_set_user_data_length(l_stream, p_mem->len /* p_size*/); /* important to avoid an assert() */
                return l_stream;
            }


            /*
             * Divide an integer by a power of 2 and round upwards.
             *
             * a divided by 2^b
             */
            inline int int_ceildivpow2(int a, int b) {
                return (a + (1 << b) - 1) >> b;
            }

            static inline bool check_comp_valid(opj_image_t *image) {
                int compno = 0;
                opj_image_comp_t *comp = &image->comps[compno];
                if (comp->prec > 32) // I doubt openjpeg will reach here.
                    return false;

                bool invalid = false;
                if (image->numcomps == 3) {
                    opj_image_comp_t *comp1 = &image->comps[1];
                    opj_image_comp_t *comp2 = &image->comps[2];
                    if (comp->prec != comp1->prec) invalid = true;
                    if (comp->prec != comp2->prec) invalid = true;
                    if (comp->sgnd != comp1->sgnd) invalid = true;
                    if (comp->sgnd != comp2->sgnd) invalid = true;
                    if (comp->h != comp1->h) invalid = true;
                    if (comp->h != comp2->h) invalid = true;
                    if (comp->w != comp1->w) invalid = true;
                    if (comp->w != comp2->w) invalid = true;
                }
                return !invalid;
            }


            std::pair<char *, size_t> decodeByStreamsCommon(char *dummy_buffer, size_t buf_size,

                    //                                                            int16_t samplesPerPixel,
                    //                                                            int16_t numberOfChannels,
                                                            uint32_t imageWidth,
                                                            uint32_t imageHeight,
                                                            std::uint8_t bitsAllocated/*,
                                                            std::uint8_t storedBits,
                                                            std::uint8_t highBit,
                                                            bool pixelRepresentation,
                                                            const char *colorSpace*/
            ) {
                opj_dparameters_t parameters;  /* decompression parameters */
                opj_codec_t *dinfo = nullptr;  /* handle to a decompressor */
                opj_stream_t *cio = nullptr;
                opj_image_t *image = nullptr;

                unsigned char *src = (unsigned char *) dummy_buffer;
                uint32_t file_length = (uint32_t) buf_size; // 32bits truncation should be ok since DICOM cannot have larger than 2Gb image

                // WARNING: OpenJPEG is very picky when there is a trailing 00 at the end of the JPC
                // so we need to make sure to remove it:
                // See for example: DX_J2K_0Padding.dcm
                //             and D_CLUNIE_CT1_J2KR.dcm
                //  Marker 0xffd9 EOI End of Image (JPEG 2000 EOC End of codestream)
                // gdcmData/D_CLUNIE_CT1_J2KR.dcm contains a trailing 0xFF which apparently is ok...
                while (file_length > 0 && src[file_length - 1] != 0xd9) {
                    file_length--;
                }
                // what if 0xd9 is never found ?
                assert(file_length > 0 && src[file_length - 1] == 0xd9);

                /* set decoding parameters to default values */
                opj_set_default_decoder_parameters(&parameters);

                const char jp2magic[] = "\x00\x00\x00\x0C\x6A\x50\x20\x20\x0D\x0A\x87\x0A";
                if (memcmp(src, jp2magic, sizeof(jp2magic)) == 0) {
                    /* JPEG-2000 compressed image data ... sigh */
                    // gdcmData/ELSCINT1_JP2vsJ2K.dcm
                    // gdcmData/MAROTECH_CT_JP2Lossy.dcm
                    gdcmWarningMacro("J2K start like JPEG-2000 compressed image data instead of codestream");
                    parameters.decod_format = JP2_CFMT;
                    assert(parameters.decod_format == JP2_CFMT);
                } else {
                    /* JPEG-2000 codestream */
                    parameters.decod_format = J2K_CFMT;
                    assert(parameters.decod_format == J2K_CFMT);
                }
                parameters.cod_format = PGX_DFMT;
                assert(parameters.cod_format == PGX_DFMT);

                /* get a decoder handle */
                switch (parameters.decod_format) {
                    case J2K_CFMT:
                        dinfo = opj_create_decompress(CODEC_J2K);
                        break;
                    case JP2_CFMT:
                        dinfo = opj_create_decompress(CODEC_JP2);
                        break;
                    default: gdcmErrorMacro("Impossible happen");
                        return std::make_pair<char *, size_t>(0, 0);
                }
#if ((OPJ_VERSION_MAJOR == 2 && OPJ_VERSION_MINOR >= 3) || (OPJ_VERSION_MAJOR > 2))
                opj_codec_set_threads(dinfo, 4);
#endif

                //                int reversible;
                myfile mysrc;
                myfile *fsrc = &mysrc;
                fsrc->mem = fsrc->cur = (char *) src;
                fsrc->len = file_length;

                OPJ_UINT32 *s[2];
                // the following hack is used for the file: DX_J2K_0Padding.dcm
                // see the function j2k_read_sot in openjpeg (line: 5946)
                // to deal with zero length Psot
                OPJ_UINT32 fl = file_length - 100;
                s[0] = &fl;
                s[1] = nullptr;
                opj_set_error_handler(dinfo, gdcm_error_callback, s);

                cio = opj_stream_create_memory_stream(fsrc, OPJ_J2K_STREAM_CHUNK_SIZE, true);

                /* set up the decoder decoding parameters using user parameters */
                OPJ_BOOL bResult;
                bResult = opj_setup_decoder(dinfo, &parameters);
                if (!bResult) {
                    opj_destroy_codec(dinfo);
                    opj_stream_destroy(cio);
                    gdcmErrorMacro("opj_setup_decoder failure");
                    return std::make_pair<char *, size_t>(0, 0);
                }

                bResult = opj_read_header(
                        cio,
                        dinfo,
                        &image);
                if (!bResult) {
                    opj_destroy_codec(dinfo);
                    opj_stream_destroy(cio);
                    gdcmErrorMacro("opj_setup_decoder failure");
                    return std::make_pair<char *, size_t>(0, 0);
                }


                bResult = opj_decode(dinfo, cio, image);
                if (!bResult) {
                    opj_destroy_codec(dinfo);
                    opj_stream_destroy(cio);
                    gdcmErrorMacro("opj_decode failed");
                    return std::make_pair<char *, size_t>(0, 0);
                }
                bResult = bResult && (image != nullptr);
                bResult = bResult && opj_end_decompress(dinfo, cio);
                if (!image || !check_comp_valid(image)) {
                    opj_destroy_codec(dinfo);
                    opj_stream_destroy(cio);
                    gdcmErrorMacro("opj_decode failed");
                    return std::make_pair<char *, size_t>(0, 0);
                }


                //                bool b = false;
                //                bool lossless;
                //                bool mct;
                //                if (parameters.decod_format == JP2_CFMT)
                //                    b = parsejp2_imp(dummy_buffer, buf_size, &lossless, &mct);
                //                else if (parameters.decod_format == J2K_CFMT)
                //                    b = parsej2k_imp(dummy_buffer, buf_size, &lossless, &mct);

                //                reversible = 0;
                //                if (b) {
                //                    reversible = lossless;
                //                }
                //    LossyFlag = !reversible;

                // assert(image->numcomps == this->GetPixelFormat().GetSamplesPerPixel());
                /// assert(image->numcomps = samplesPerPixel);


                /* close the byte stream */
                opj_stream_destroy(cio);

                // Copy buffer
                //    unsigned long len = Dimensions[0] * Dimensions[1] * (PF.GetBitsAllocated() / 8) * image->numcomps;
                unsigned long len = imageWidth * imageHeight * (bitsAllocated / 8) * image->numcomps;
                char *raw = new char[len];
                //assert( len == fsrc->len );
                for (unsigned int compno = 0; compno < (unsigned int) image->numcomps; compno++) {
                    opj_image_comp_t *comp = &image->comps[compno];

                    int w = image->comps[compno].w;
                    int wr = int_ceildivpow2(image->comps[compno].w, image->comps[compno].factor);

                    //int h = image.comps[compno].h;
                    int hr = int_ceildivpow2(image->comps[compno].h, image->comps[compno].factor);
                    //assert(  wr * hr * 1 * image->numcomps * (comp->prec/8) == len );

                    // ELSCINT1_JP2vsJ2K.dcm
                    // -> prec = 12, bpp = 0, sgnd = 0
                    //assert( wr == Dimensions[0] );
                    //assert( hr == Dimensions[1] );
                    //        if (comp->sgnd != PF.GetPixelRepresentation()) {
                    //            PF.SetPixelRepresentation((uint16_t) comp->sgnd);
                    //        }
#ifndef GDCM_SUPPORT_BROKEN_IMPLEMENTATION
//     assert(comp->prec == storedBits); // D_CLUNIE_RG3_JPLY.dcm
//      assert(comp->prec - 1 == highBit);
#endif
//assert( comp->prec >= PF.GetBitsStored());
//        if (comp->prec != PF.GetBitsStored()) {
//            if (comp->prec <= 8)
//                PF.SetBitsAllocated(8);
//            else if (comp->prec <= 16)
//                PF.SetBitsAllocated(16);
//            else if (comp->prec <= 32)
//                PF.SetBitsAllocated(32);
//            PF.SetBitsStored((unsigned short) comp->prec);
//            PF.SetHighBit((unsigned short) (comp->prec - 1)); // ??
//        }
//        assert(PF.IsValid());
//        assert(comp->prec <= 32);

                    if (comp->prec <= 8) {
                        uint8_t *data8 = (uint8_t *) raw + compno;
                        for (int i = 0; i < wr * hr; i++) {
                            int v = image->comps[compno].data[i / wr * w + i % wr];
                            *data8 = (uint8_t) v;
                            data8 += image->numcomps;
                        }
                    } else if (comp->prec <= 16) {
                        // ELSCINT1_JP2vsJ2K.dcm is a 12bits image
                        uint16_t *data16 = (uint16_t *) (void *) raw + compno;
                        for (int i = 0; i < wr * hr; i++) {
                            int v = image->comps[compno].data[i / wr * w + i % wr];
                            *data16 = (uint16_t) v;
                            data16 += image->numcomps;
                        }
                    } else {
                        uint32_t *data32 = (uint32_t *) (void *) raw + compno;
                        for (int i = 0; i < wr * hr; i++) {
                            int v = image->comps[compno].data[i / wr * w + i % wr];
                            *data32 = (uint32_t) v;
                            data32 += image->numcomps;
                        }
                    }
                }

                /* free remaining structures */
                if (dinfo) {
                    opj_destroy_codec(dinfo);
                }

                /* free image data structure */
                opj_image_destroy(image);

                return std::make_pair(raw, len);
            }


            ///////////////////////////////////////////////
            ///Encode
            ///////////////////////////////////////////////


            template<typename T>
            void rawtoimage_fill2(const T *inputbuffer, int w, int h, int numcomps, opj_image_t *image, int pc,
                                  int bitsallocated, int bitsstored, int highbit, int sign) {
                uint16_t pmask = 0xffff;
                pmask = (uint16_t) (pmask >> (bitsallocated - bitsstored));

                const T *p = inputbuffer;
                if (sign) {
                    // smask : to check the 'sign' when BitsStored != BitsAllocated
                    uint16_t smask = 0x0001;
                    smask = (uint16_t) (
                            smask << (16 - (bitsallocated - bitsstored + 1)));
                    // nmask : to propagate sign a bit on negative values
                    int16_t nmask = (int16_t) 0x8000;
                    nmask = (int16_t) (nmask >> (bitsallocated - bitsstored - 1));
                    if (pc) {
                        for (int compno = 0; compno < numcomps; compno++) {
                            for (int i = 0; i < w * h; i++) {
                                /* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
                                uint16_t c = *p;
                                c = (uint16_t) (c >> (bitsstored - highbit - 1));
                                if (c & smask) {
                                    c = (uint16_t) (c | nmask);
                                } else {
                                    c = c & pmask;
                                }
                                int16_t fix;
                                memcpy(&fix, &c, sizeof fix);
                                image->comps[compno].data[i] = fix;
                                ++p;
                            }
                        }
                    } else {
                        for (int i = 0; i < w * h; i++) {
                            for (int compno = 0; compno < numcomps; compno++) {
                                /* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
                                uint16_t c = *p;
                                c = (uint16_t) (c >> (bitsstored - highbit - 1));
                                if (c & smask) {
                                    c = (uint16_t) (c | nmask);
                                } else {
                                    c = c & pmask;
                                }
                                int16_t fix;
                                memcpy(&fix, &c, sizeof fix);
                                image->comps[compno].data[i] = fix;
                                ++p;
                            }
                        }
                    }
                } else {
                    if (pc) {
                        for (int compno = 0; compno < numcomps; compno++) {
                            for (int i = 0; i < w * h; i++) {
                                /* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
                                uint16_t c = *p;
                                c = (uint16_t) (
                                        (c >> (bitsstored - highbit - 1)) & pmask);
                                image->comps[compno].data[i] = c;
                                ++p;
                            }
                        }
                    } else {
                        for (int i = 0; i < w * h; i++) {
                            for (int compno = 0; compno < numcomps; compno++) {
                                /* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
                                uint16_t c = *p;
                                c = (uint16_t) (
                                        (c >> (bitsstored - highbit - 1)) & pmask);
                                image->comps[compno].data[i] = c;
                                ++p;
                            }
                        }
                    }
                }
            }

            template<typename T>
            void rawtoimage_fill(const T *inputbuffer, int w, int h, int numcomps, opj_image_t *image, int pc) {
                const T *p = inputbuffer;
                if (pc) {
                    for (int compno = 0; compno < numcomps; compno++) {
                        for (int i = 0; i < w * h; i++) {
                            /* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
                            image->comps[compno].data[i] = *p;
                            ++p;
                        }
                    }
                } else {
                    for (int i = 0; i < w * h; i++) {
                        for (int compno = 0; compno < numcomps; compno++) {
                            /* compno : 0 = GREY, (0, 1, 2) = (R, G, B) */
                            image->comps[compno].data[i] = *p;
                            ++p;
                        }
                    }
                }
            }

            opj_image_t *rawtoimage(const char *inputbuffer8, opj_cparameters_t *parameters,
                                    size_t fragment_size, int image_width, int image_height, int sample_pixel,
                                    int bitsallocated, int bitsstored, int highbit, int sign, int quality, int pc) {
                (void) quality;
                (void) fragment_size;
                int w, h;
                int numcomps;
                OPJ_COLOR_SPACE color_space;
                opj_image_cmptparm_t cmptparm[3]; /* maximum of 3 components */
                opj_image_t *image = nullptr;
                const void *inputbuffer = inputbuffer8;

                assert(sample_pixel == 1 || sample_pixel == 3);
                if (sample_pixel == 1) {
                    numcomps = 1;
                    color_space = CLRSPC_GRAY;
                } else // sample_pixel == 3
                {
                    numcomps = 3;
                    color_space = CLRSPC_SRGB;
                    /* Does OpenJPEg support: CLRSPC_SYCC ?? */
                }
                if (bitsallocated % 8 != 0) {
                    gdcmDebugMacro("BitsAllocated is not % 8");
                    return nullptr;
                }
                assert(bitsallocated % 8 == 0);
                // eg. fragment_size == 63532 and 181 * 117 * 3 * 8 == 63531 ...
                assert(((fragment_size + 1) / 2) * 2 ==
                       (((size_t) image_height * image_width * numcomps * (bitsallocated / 8) + 1) / 2) * 2);
                int subsampling_dx = parameters->subsampling_dx;
                int subsampling_dy = parameters->subsampling_dy;


                w = image_width;
                h = image_height;

                /* initialize image components */
                memset(&cmptparm[0], 0, 3 * sizeof(opj_image_cmptparm_t));
                //assert( bitsallocated == 8 );
                for (int i = 0; i < numcomps; i++) {
                    cmptparm[i].prec = bitsstored;
                    cmptparm[i].prec = bitsallocated;
                    cmptparm[i].bpp = bitsallocated;
                    cmptparm[i].sgnd = sign;
                    cmptparm[i].dx = subsampling_dx;
                    cmptparm[i].dy = subsampling_dy;
                    cmptparm[i].w = w;
                    cmptparm[i].h = h;
                }

                /* create the image */
                image = opj_image_create(numcomps, &cmptparm[0], color_space);
                if (!image) {
                    return nullptr;
                }
                /* set image offset and reference grid */
                image->x0 = parameters->image_offset_x0;
                image->y0 = parameters->image_offset_y0;
                image->x1 = parameters->image_offset_x0 + (w - 1) * subsampling_dx + 1;
                image->y1 = parameters->image_offset_y0 + (h - 1) * subsampling_dy + 1;

                /* set image data */

                //assert( fragment_size == numcomps*w*h*(bitsallocated/8) );
                if (bitsallocated <= 8) {
                    if (sign) {
                        rawtoimage_fill<int8_t>((const int8_t *) inputbuffer, w, h, numcomps, image, pc);
                    } else {
                        rawtoimage_fill<uint8_t>((const uint8_t *) inputbuffer, w, h, numcomps, image, pc);
                    }
                } else if (bitsallocated <= 16) {
                    if (bitsallocated != bitsstored) {
                        if (sign) {
                            rawtoimage_fill2<int16_t>((const int16_t *) inputbuffer, w, h, numcomps, image, pc,
                                                      bitsallocated, bitsstored, highbit, sign);
                        } else {
                            rawtoimage_fill2<uint16_t>((const uint16_t *) inputbuffer, w, h, numcomps, image, pc,
                                                       bitsallocated, bitsstored, highbit, sign);
                        }
                    } else {
                        if (sign) {
                            rawtoimage_fill<int16_t>((const int16_t *) inputbuffer, w, h, numcomps, image, pc);
                        } else {
                            rawtoimage_fill<uint16_t>((const uint16_t *) inputbuffer, w, h, numcomps, image, pc);
                        }
                    }
                } else if (bitsallocated <= 32) {
                    if (sign) {
                        rawtoimage_fill<int32_t>((const int32_t *) inputbuffer, w, h, numcomps, image, pc);
                    } else {
                        rawtoimage_fill<uint32_t>((const uint32_t *) inputbuffer, w, h, numcomps, image, pc);
                    }
                } else // dead branch ?
                {
                    opj_image_destroy(image);
                    return nullptr;
                }

                return image;
            }


            bool codeFrameIntoBuffer(char *outdata, size_t outlen, size_t &complen, const char *inputdata,
                                     size_t inputlength,

                                     int image_width,
                                     int image_height,
                                     int sample_pixel,
                                     int bitsallocated,
                                     int bitsstored,
                                     int highbit,
                                     int sign,
                                     opj_cparameters  Internalscoder_param,
                                     int  pPlanarConfiguration
            ) {
                complen = 0; // default init

                int numZ = 0; //dims[2];


                int quality = 100;

                //// input_buffer is ONE image
                //// fragment_size is the size of this image (fragment)
                (void) numZ;
                bool bSuccess;
                //bool delete_comment = true;
                opj_cparameters_t parameters;  /* compression parameters */
                opj_image_t *image = nullptr;
                //quality = 100;

                /* set encoding parameters to default values */
                //memset(&parameters, 0, sizeof(parameters));
                //opj_set_default_encoder_parameters(&parameters);

                memcpy(&parameters, &Internalscoder_param, sizeof(parameters));

                if ((parameters.cp_disto_alloc || parameters.cp_fixed_alloc || parameters.cp_fixed_quality)
                    && (!(parameters.cp_disto_alloc ^ parameters.cp_fixed_alloc ^ parameters.cp_fixed_quality))) {
                    gdcmErrorMacro("Error: options -r -q and -f cannot be used together.");
                    return false;
                }        /* mod fixed_quality */

                /* if no rate entered, lossless by default */
                if (parameters.tcp_numlayers == 0) {
                    parameters.tcp_rates[0] = 0;
                    parameters.tcp_numlayers = 1;
                    parameters.cp_disto_alloc = 1;
                }

                if (parameters.cp_comment == nullptr) {
                    const char comment[] = "Created by GDCM/OpenJPEG version %s";
                    const char *vers = opj_version();
                    parameters.cp_comment = (char *) malloc(strlen(comment) + 10);
                    snprintf(parameters.cp_comment, strlen(comment) + 10, comment, vers);
                    /* no need to delete parameters.cp_comment on exit */
                    //delete_comment = false;
                }

                // Compute the proper number of resolutions to use.
                // This is mostly done for images smaller than 64 pixels
                // along any dimension.
                unsigned int numberOfResolutions = 0;

                unsigned int tw = image_width >> 1;
                unsigned int th = image_height >> 1;

                while (tw && th) {
                    numberOfResolutions++;
                    tw >>= 1;
                    th >>= 1;
                }

                // Clamp the number of resolutions to 6.
                if (numberOfResolutions > 6) {
                    numberOfResolutions = 6;
                }

                parameters.numresolution = numberOfResolutions;


                /* decode the source image */
                /* ----------------------- */

                image = rawtoimage((const char *) inputdata, &parameters,
                                   inputlength,
                                   image_width, image_height,
                                   sample_pixel, bitsallocated, bitsstored, highbit, sign, quality,
                                   pPlanarConfiguration);
                if (!image) {
                    return false;
                }

                /* encode the destination image */
                /* ---------------------------- */
                parameters.cod_format = J2K_CFMT; /* J2K format output */
                size_t codestream_length;
                opj_codec_t *cinfo = nullptr;
                opj_stream_t *cio = nullptr;

                /* get a J2K compressor handle */
                cinfo = opj_create_compress(CODEC_J2K);


                /* set up the encoder parameters using the current image and using user parameters */
                opj_setup_encoder(cinfo, &parameters, image);

                myfile mysrc;
                myfile *fsrc = &mysrc;
                char *buffer_j2k = new char[inputlength]; // overallocated
                fsrc->mem = fsrc->cur = buffer_j2k;
                fsrc->len = 0; //inputlength;

                /* open a byte stream for writing */
                /* allocate memory for all tiles */
                cio = opj_stream_create_memory_stream(fsrc, OPJ_J2K_STREAM_CHUNK_SIZE, false);
                if (!cio) {
                    return false;
                }
                /* encode the image */
                /*if (*indexfilename)          // If need to extract codestream information
                  bSuccess = opj_encode_with_info(cinfo, cio, image, &cstr_info);
                  else*/
                bSuccess = opj_start_compress(cinfo, image, cio) ? true : false;
                bSuccess = bSuccess && opj_encode(cinfo, cio);
                bSuccess = bSuccess && opj_end_compress(cinfo, cio);

                if (!bSuccess) {
                    opj_stream_destroy(cio);
                    return false;
                }
                codestream_length = mysrc.len;


                bool success = false;
                if (codestream_length <= outlen) {
                    success = true;
                    memcpy(outdata, (char *) (mysrc.mem), codestream_length);
                }
                delete[] buffer_j2k;

                /* close and free the byte stream */
                opj_stream_destroy(cio);

                /* free remaining compression structures */
                opj_destroy_codec(cinfo);
                complen = codestream_length;

                /* free user parameters structure */
                if (parameters.cp_comment) free(parameters.cp_comment);
                if (parameters.cp_matrice) free(parameters.cp_matrice);

                /* free image data */
                opj_image_destroy(image);

                return success;
            }

            // Compress into JPEG



        }
    }
}

#endif //IMEBRA_JPEG2000HELPER_H
