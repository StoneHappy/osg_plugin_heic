#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/ImageUtils>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <sstream>
#include <iostream>
#include <fstream>
#include <libheif/heif.h>

#if defined(_MSC_VER) && defined(OSG_DISABLE_MSVC_WARNINGS)
// disable "structure was padded due to __declspec(align())
#pragma warning( disable : 4324 )
#endif

/****************************************************************************
 *
 * Follows is code extracted from the simage library.  Original Authors:
 *
 *      Systems in Motion,
 *      <URL:http://www.sim.no>
 *
 *      Peder Blekken <pederb@sim.no>
 *      Morten Eriksen <mortene@sim.no>
 *      Marius Bugge Monsen <mariusbu@sim.no>
 *
 * The original COPYING notice
 *
 *      All files in this library are public domain, except simage_rgb.cpp which is
 *      Copyright (c) Mark J Kilgard <mjk@nvidia.com>. I will contact Mark
 *      very soon to hear if this source also can become public domain.
 *
 *      Please send patches for bugs and new features to: <pederb@sim.no>.
 *
 *      Peder Blekken
 *
 *
 * Ported into the OSG as a plugin, Robert Osfield December 2000.
 * Note, reference above to license of simage_rgb is not relevant to the OSG
 * as the OSG does not use it.  Also for patches, bugs and new features
 * please send them direct to the OSG dev team rather than address above.
 *
 **********************************************************************/

 /*
  * Based on example code found in the libjpeg archive
  *
  */

#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#if defined(_MSC_VER) && defined(OSG_DISABLE_MSVC_WARNINGS)
#pragma warning( disable : 4611 )
#endif
#define LIBHEIF_TEST_ERROR(func, ...) if ((func) != heif_error_code::heif_error_Ok) {std::cout << __VA_ARGS__ << std::endl;}
class ReaderWriterHEIC : public osgDB::ReaderWriter
{
    WriteResult::WriteStatus write_HEIC_file(std::ostream& fout, const osg::Image& img, int quality = 100) const
    {
        std::cout << "write heic!" << std::endl;
        return WriteResult::WriteStatus::FILE_SAVED;
    }
    int getQuality(const osgDB::ReaderWriter::Options* options) const {
        return 100;
    }
public:

    ReaderWriterHEIC()
    {
        supportsExtension("heic", "HEIC image format");
    }

    virtual const char* className() const { return "heic Image Reader/Writer"; }

    ReadResult readHEICStream(std::istream& fin) const
    {
        std::cout << "read heic!" << std::endl;
        std::ofstream out("temp.heic", std::ios::binary);
        out << fin.rdbuf();
        out.close();
        osg::ref_ptr<osg::Image> pOsgImage = new osg::Image;
        return readImage("./temp.heic", nullptr);
    }

    virtual ReadResult readObject(std::istream& fin, const osgDB::ReaderWriter::Options* options = NULL) const
    {
        return readImage(fin, options);
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options = NULL) const
    {
        return readImage(file, options);
    }

    virtual ReadResult readImage(std::istream& fin, const osgDB::ReaderWriter::Options* = NULL) const
    {
        return readHEICStream(fin);
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        /*std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file, options);
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
        if (!istream) return ReadResult::ERROR_IN_READING_FILE;
        ReadResult rr = readHEICStream(istream);
        if (rr.validImage()) rr.getImage()->setFileName(file);*/
        heif_context* ctx = heif_context_alloc();
        LIBHEIF_TEST_ERROR(heif_context_read_from_file(ctx, file.c_str(), nullptr).code, "File read error!")

            // get a handle to the primary image
            heif_image_handle* handle;
        LIBHEIF_TEST_ERROR(heif_context_get_primary_image_handle(ctx, &handle).code, "Get image handle error")

            // decode the image and convert colorspace to RGB, saved as 24bit interleaved
            heif_image* img;
        LIBHEIF_TEST_ERROR(heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr).code, "Decode image error")

            auto width = heif_image_get_primary_width(img);
        auto height = heif_image_get_primary_height(img);

        std::cout << "Get image info: " << std::endl;
        std::cout << "width: " << width << std::endl;
        std::cout << "height: " << height << std::endl;

        int stride;
        uint8_t* data = heif_image_get_plane(img, heif_channel_interleaved, &stride);
        osg::ref_ptr<osg::Image> osgimage = new osg::Image;

        osgimage->setImage(width, height, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE);
        osgimage->flipVertical();
        osgimage->setFileName(file);
        return osgimage.release();
    }

    virtual WriteResult writeImage(const osg::Image& img, std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
    {
        std::cout << "writeImage fout!" << std::endl;
        osg::ref_ptr<osg::Image> tmp_img = new osg::Image(img);
        tmp_img->flipVertical();
        WriteResult::WriteStatus ws = write_HEIC_file(fout, *(tmp_img.get()), getQuality(options));
        return ws;
    }

    virtual WriteResult writeImage(const osg::Image& img, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::cout << "writeImage filename!" << std::endl;
        std::string ext = osgDB::getFileExtension(fileName);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
        if (!fout) return WriteResult::ERROR_IN_WRITING_FILE;

        return writeImage(img, fout, options);
    }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(heic, ReaderWriterHEIC)
