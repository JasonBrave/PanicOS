#ifndef _LIBGUI_IMAGELOADER_BMP_H
#define _LIBGUI_IMAGELOADER_BMP_H

#include <cstdio>

namespace GUI {

	class BMPLoader {

		std::FILE* bmpfile;
		int width, height, bpp, img_offset;

		void pix24_to_colour(void* dest, const void* src, int num);

	public:
		BMPLoader() = delete;
		BMPLoader(const char* filename);
		~BMPLoader();

		void load(void* dest);
		int get_bpp(void) const;
		int get_width(void) const;
		int get_height(void) const;
	};

} // namespace GUI

#endif
