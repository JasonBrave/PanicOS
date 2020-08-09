/*
 * about program
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <array>
#include <cstdlib>
#include <iostream>
#include <libcpu/cpuinfo.h>
#include <libgui/button.hpp>
#include <libgui/imagebox.hpp>
#include <libgui/label.hpp>
#include <libgui/window.hpp>
#include <string>

class AboutWindow : public GUI::Window {
	GUI::Label about_label = "About PanicOS";
	GUI::Label version_label = "alpha version";
	GUI::Label license_label = "This is free software released under ";
	GUI::Label license_label2 = "GNU General Public License version 3.";
	GUI::Label date_label;
	GUI::Label compiler_label;
	GUI::Label cpuname_label;
	GUI::ImageBox gplv3_logo;
	GUI::Button close_button;
	std::string cpuname;

public:
	AboutWindow() : GUI::Window(400, 400) {
		set_title("About PanicOS\n");

		about_label.x = 16;
		about_label.y = 25;
		add_control(about_label);

		version_label.x = 16;
		version_label.y = 50;
		add_control(version_label);

		license_label.x = 16;
		license_label.y = 75;
		add_control(license_label);
		license_label2.x = 16;
		license_label2.y = 100;
		add_control(license_label2);

		date_label.x = 16;
		date_label.y = 150;
		date_label.set_text("Built on " __DATE__ " " __TIME__);
		add_control(date_label);

		compiler_label.x = 16;
		compiler_label.y = 175;
		compiler_label.set_text("Compiled with GCC " __VERSION__);
		add_control(compiler_label);

		std::array<char, 50> brand;
		cpuname = cpu_get_brand_string(brand.data());
		cpuname_label.x = 16;
		cpuname_label.y = 200;
		cpuname_label.set_text("CPU: " + cpuname);
		add_control(cpuname_label);

		gplv3_logo.x = 100;
		gplv3_logo.y = 230;
		gplv3_logo.load_image("/share/gplv3.bmp");
		add_control(gplv3_logo);

		close_button.x = 150;
		close_button.y = 350;
		close_button.width = 80;
		close_button.height = 30;
		close_button.set_text("Close");
		close_button.onclick.connect(this, &AboutWindow::close_button_onclick);
		add_control(close_button);
	}

	void close_button_onclick() {
		std::exit(0);
	}
};

int main() {
	std::cout << "PanicOS alpha version\n";
	std::cout << "This is free software released under GNU General Public License "
				 "version 3.\n";
	std::cout << "Built on " __DATE__ " " __TIME__ << '\n';
	std::cout << "Compiled with GCC " __VERSION__ << '\n';
	std::array<char, 50> cpu_brand;
	std::cout << "CPU: " << cpu_get_brand_string(cpu_brand.data()) << '\n';

	AboutWindow win;
	GUI::start_application(win);
	return 0;
}
