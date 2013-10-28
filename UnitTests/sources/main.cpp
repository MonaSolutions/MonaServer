/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License received along this program for more
	details (or else see http://www.gnu.org/licenses/).

	This file is a part of Mona.
*/

#include "Mona/Application.h"
#include "Mona/Logs.h"
#include "Test.h"

using namespace Mona;
using namespace std;

class TestApp : public Application  {
private:
	void defineOptions(Exception& ex, Options& options) 
		/// \brief define options of TestApp
	{

		options.add(ex, "mod", "m", "Specify the module to run.")
			.argument("module")
			.handler([this](const string& value) { _module = value; });

		// defines here your options applications
		Application::defineOptions(ex,options);
	}

	void runSelectedModule() 
		/// \brief Selection Module function
	{

		vector<const std::string> lTests;
		PoolTest::_PoolTestInstance.getListTests(lTests);

		// Print The list
		int index = 0;
		string tmp;
		cout << String::Format(tmp, "a - Run all tests") << endl;
		for(const string& test : lTests) {
			cout << String::Format(tmp, index++, " - ", test) << endl;
		}

		// Ask for the index
		cout << endl << "Choose the index of the test to run : ";
		char val[255];
		cin >> val;
		Exception ex;
		int number = String::ToNumber<int>(ex, val);
		
		if (*val == 'a') {

			PoolTest::_PoolTestInstance.runAll();
		} else if (!ex && (number >= 0) && (number < lTests.size())) {
			
			PoolTest::_PoolTestInstance.run(lTests.at(number));
		}
	}

///// MAIN
	int main() {

		try {

			if (_module.empty()) {
				runSelectedModule();
			}
			else if (_module=="all")
				PoolTest::_PoolTestInstance.runAll();
			else
				PoolTest::_PoolTestInstance.run(_module);
		}
		catch (exception ex) {
			FATAL(ex.what())
		}
		NOTE("END OF TESTS");
		system("pause");

		return EXIT_OK;
	}

private:
	string _module;
};

PoolTest PoolTest::_PoolTestInstance;

int main(int argc, char* argv[]) {
	return TestApp().run(argc, argv);
}
