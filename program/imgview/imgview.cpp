#include <iostream>
#include <libgui/imagebox.hpp>
#include <libgui/window.hpp>
#include <string>

class ImgView : public GUI::Window {
	GUI::ImageBox img;

public:
	ImgView(const std::string& filename) : GUI::Window(600, 400) {
		set_title("Image viewer");

		img.x = 0;
		img.y = 0;
		img.load_image(filename.c_str());
		add_control(img);
	}
};

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cout << "Usage: imgview filename" << std::endl;
		return 1;
	}
	ImgView imgview(argv[1]);
	GUI::start_application(imgview);
	return 0;
}
