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

class ReaderWriterHEIC : public osgDB::ReaderWriter
{
    WriteResult::WriteStatus write_JPEG_file(std::ostream& fout, const osg::Image& img, int quality = 100) const
    {
        std::cout << "write heic!" << std::endl;
    }
    int getQuality(const osgDB::ReaderWriter::Options* options) const {
        return 100;
    }
public:

    ReaderWriterHEIC()
    {
        supportsExtension("heic", "HEIC image format");
    }

    virtual const char* className() const { return "JPEG Image Reader/Writer"; }

    ReadResult readJPGStream(std::istream& fin) const
    {
        std::cout << "read heic!" << std::endl;
        osg::ref_ptr<osg::Image> pOsgImage = new osg::Image;
        return pOsgImage.release();
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
        return readJPGStream(fin);
    }

    virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file, options);
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
        if (!istream) return ReadResult::ERROR_IN_READING_FILE;
        ReadResult rr = readJPGStream(istream);
        if (rr.validImage()) rr.getImage()->setFileName(file);
        return rr;
    }

    virtual WriteResult writeImage(const osg::Image& img, std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
    {
        osg::ref_ptr<osg::Image> tmp_img = new osg::Image(img);
        tmp_img->flipVertical();
        WriteResult::WriteStatus ws = write_JPEG_file(fout, *(tmp_img.get()), getQuality(options));
        return ws;
    }

    virtual WriteResult writeImage(const osg::Image& img, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
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
