#include <cstdio>
#include <cstdint>
#include <vector>
#include <exception>
#include <jpeglib.h>

struct ARGB_LE {
    //uint8_t  [   3  ][   2  ][   1  ][   0  ]
    //bit      [31..24][23..16][15..08][07..00]
    //channel  AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB
    uint8_t b, g, r, a;
};
union Pixel {
    ARGB_LE  channel;
    uint32_t hex; //0xAARRGGBB
};
struct RawImage {
    int w, h;
    std::vector<Pixel> data;
};

RawImage loadRawJpeg(const std::string&);

int main()
{
    RawImage img = loadRawJpeg("5x13.jpg");

    for (int i = 0; i < img.h; ++i)
    {
        for (int j = 0; j < img.w; ++j)
        {
            Pixel pix = img.data[i*img.w + j];
            printf("(%03d,%03d,%03d) ", pix.channel.r, pix.channel.g, pix.channel.b);
        }
        printf("\n");
    }

    printf("\n");
    for (int i = 0; i < img.h; ++i)
    {
        for (int j = 0; j < img.w; ++j)
        {
            Pixel pix = img.data[i*img.w + j];
            printf("0x%08x ", pix.hex);
        }
        printf("\n");
    }
}


RawImage loadRawJpeg(const std::string& filename)
{
    RawImage img;
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    FILE* infile = NULL;
    JSAMPARRAY buffer;
    int row_stride;

    if (!(infile = fopen(filename.c_str(), "rb")))
        throw std::runtime_error("Can't open " + filename);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    img.w = cinfo.output_width;
    img.h = cinfo.output_height;
    img.data.reserve(img.w*img.h);

    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for (int x = 0; x < img.w; ++x) {
            Pixel pix;
            pix.channel.a = 0; // alpha value is not supported on jpg
            pix.channel.r = buffer[0][cinfo.output_components*x];
            if (cinfo.output_components > 2) {
                pix.channel.g = buffer[0][cinfo.output_components*x + 1];
                pix.channel.b = buffer[0][cinfo.output_components*x + 2];
            } else {
                pix.channel.g = pix.channel.b = pix.channel.r;
            }
            img.data.push_back(pix);
        }
    }
    fclose(infile);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return img;
}