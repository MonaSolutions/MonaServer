
#include "Test.h"
#include "Mona/Options.h"
#include "Mona/Logs.h"
#include <vector>

using namespace Mona;
using namespace std;

Options _Options;

bool GetOption(const string& fullName) { return _Options.get(fullName) ? true : false; }

bool AddOption(const string& fullName, const string& shortName, const string& description, 
		bool required=false, bool repeatable=false, const string& argName="", bool argRequired=false) {

	Exception ex;
	Option& opt = _Options.add(ex, fullName.c_str(), shortName.c_str())
		.description(description)
		.required(required)
		.repeatable(repeatable);
	
	if (!argName.empty())
		opt.argument(argName, argRequired);

	if (ex) {
		DEBUG("Exception : ", ex.error());
		return false;
	}

	return true;
}

bool ProcessArg(const char* arg, const function<void(Exception& ex, const std::string&, const std::string&)>& handler=nullptr) {
    const char* argv[] = {"", arg};
	
	Exception ex;
	return _Options.process(ex, 2, argv, handler) && !ex;
}

ADD_TEST(OptionsTest, TestOption) {

	Option incOpt("include-dir", "I", "specify an include search path");
	incOpt.required(false)
		.repeatable(true)
		.argument("path");
		
	Option libOpt("library-dir", "L", "specify a library search path", false);
	libOpt.repeatable(true)
		.argument("path");
		
	Option outOpt("output", "o", "specify the output file", true);
	outOpt.argument("file", true);

	Option vrbOpt("verbose", "v");
	vrbOpt.description("enable verbose mode")
		.required(false)
		.repeatable(false);
		
	Option optOpt("optimize", "O");
	optOpt.description("enable optimization")
		.required(false)
		.repeatable(false)
		.argument("level", false);
		
	CHECK(incOpt.shortName() == "I");
	CHECK(incOpt.fullName() == "include-dir");
	CHECK(incOpt.repeatable());
	CHECK(!incOpt.required());
	CHECK(incOpt.argumentName() == "path");
	CHECK(incOpt.argumentRequired());
	CHECK(incOpt.takesArgument());
		
	CHECK(libOpt.shortName() == "L");
	CHECK(libOpt.fullName() == "library-dir");
	CHECK(libOpt.repeatable());
	CHECK(!libOpt.required());
	CHECK(libOpt.argumentName() == "path");
	CHECK(libOpt.argumentRequired());
	CHECK(incOpt.takesArgument());

	CHECK(outOpt.shortName() == "o");
	CHECK(outOpt.fullName() == "output");
	CHECK(!outOpt.repeatable());
	CHECK(outOpt.required());
	CHECK(outOpt.argumentName() == "file");
	CHECK(outOpt.argumentRequired());
	CHECK(incOpt.takesArgument());

	CHECK(vrbOpt.shortName() == "v");
	CHECK(vrbOpt.fullName() == "verbose");
	CHECK(!vrbOpt.repeatable());
	CHECK(!vrbOpt.required());
	CHECK(!vrbOpt.argumentRequired());
	CHECK(!vrbOpt.takesArgument());

	CHECK(optOpt.shortName() == "O");
	CHECK(optOpt.fullName() == "optimize");
	CHECK(!optOpt.repeatable());
	CHECK(!optOpt.required());
	CHECK(optOpt.argumentName() == "level");
	CHECK(optOpt.takesArgument());
	CHECK(!optOpt.argumentRequired());
}

ADD_TEST(OptionsTest, TestOptionsAdd) {

	//removeAllOptions();

	CHECK(AddOption("helper", "H", "start helper"));
	CHECK(AddOption("help", "h", "print help text"));
	CHECK(AddOption("include-dir", "I", "specify a search path for locating header files", false, true, "path"));
	CHECK(AddOption("library-dir", "L", "specify a search path for locating library files", false, true, "path"));
	CHECK(AddOption("insert", "it", "insert something", false, true, "path"));
	CHECK(!AddOption("item", "", "insert something", false, true, "path"));
	CHECK(AddOption("include", "J", "specify a search path for locating header files", false, true, "path"));
	CHECK(!AddOption("include", "J", "specify a search path for locating header files"));

	CHECK(GetOption("include"));
	CHECK(GetOption("insert"));
	CHECK(!GetOption("Insert"));
	CHECK(!GetOption("item"));
	CHECK(!GetOption("i"));
	CHECK(!GetOption("he"));
	CHECK(!GetOption("in"));
	CHECK(GetOption("help"));
	CHECK(!GetOption("helpe"));
	CHECK(GetOption("helper"));

	_Options.remove("include-dir");
	CHECK(!GetOption("include-dir"));
}

void TestProcessInclude(Exception& ex, const string& name, const string& value) {
	CHECK(name == "include-dir");
	static int i = 0;
    static const char* res[] = {
		"include",
		"/usr/include",
		"/usr/local/include",
		"/proj/include",
		"/usr/include" };
	CHECK(value == res[i++]);
}

void TestProcessVerbose(Exception& ex, const string& name, const string& value) {
	CHECK(name == "verbose");
	CHECK(value.empty());
}

void TestProcessOptimize(Exception& ex, const string& name, const string& value) {
	CHECK(name == "optimize");
	static int counter = 0;
	if (++counter < 3)
		CHECK(value.empty())
	else if (counter == 3)
		CHECK(value == "1")
	else
		CHECK(value == "2")
}

ADD_TEST(OptionsTest, TestProcess) {
	//_Options.clear();
	Exception ex;
	CHECK(AddOption("include-dir", "I", "specify an include search path", false, true, "path", true));

    const char* arg[] = { "row for path",
					"/I:include",
					"-I=/usr/include",
					"/include-dir:/usr/local/include",
					"-include-dir=/proj/include",
					"/include-dir=/usr/include"};

	CHECK(_Options.process(ex, (sizeof(arg) / sizeof(char *)), arg, TestProcessInclude));

	CHECK(!ProcessArg("/I"));
	
	CHECK(!ProcessArg("/include-dir"));

	CHECK(!ProcessArg("/Llib"));
	
	CHECK(AddOption("verbose", "v", "enable verbose mode", false, false));
	
	CHECK(ProcessArg("/v", TestProcessVerbose));
	
	CHECK(ProcessArg("/verbose", TestProcessVerbose));

	CHECK(!ProcessArg("/v2"));

	// TODO If argument specified but not expected => must be false
	//CHECK(!ProcessArg("/verbose:2"));

	CHECK(AddOption("optimize", "O", "enable optimization", false, false, "level", false));
	
	CHECK(ProcessArg("/O", TestProcessOptimize));
	CHECK(ProcessArg("-optimize=", TestProcessOptimize));

	CHECK(ProcessArg("-optimize:1", TestProcessOptimize));
	CHECK(ProcessArg("/O=2", TestProcessOptimize));
	
}
