
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
		
	EXPECT_TRUE(incOpt.shortName() == "I");
	EXPECT_TRUE(incOpt.fullName() == "include-dir");
	EXPECT_TRUE(incOpt.repeatable());
	EXPECT_TRUE(!incOpt.required());
	EXPECT_TRUE(incOpt.argumentName() == "path");
	EXPECT_TRUE(incOpt.argumentRequired());
	EXPECT_TRUE(incOpt.takesArgument());
		
	EXPECT_TRUE(libOpt.shortName() == "L");
	EXPECT_TRUE(libOpt.fullName() == "library-dir");
	EXPECT_TRUE(libOpt.repeatable());
	EXPECT_TRUE(!libOpt.required());
	EXPECT_TRUE(libOpt.argumentName() == "path");
	EXPECT_TRUE(libOpt.argumentRequired());
	EXPECT_TRUE(incOpt.takesArgument());

	EXPECT_TRUE(outOpt.shortName() == "o");
	EXPECT_TRUE(outOpt.fullName() == "output");
	EXPECT_TRUE(!outOpt.repeatable());
	EXPECT_TRUE(outOpt.required());
	EXPECT_TRUE(outOpt.argumentName() == "file");
	EXPECT_TRUE(outOpt.argumentRequired());
	EXPECT_TRUE(incOpt.takesArgument());

	EXPECT_TRUE(vrbOpt.shortName() == "v");
	EXPECT_TRUE(vrbOpt.fullName() == "verbose");
	EXPECT_TRUE(!vrbOpt.repeatable());
	EXPECT_TRUE(!vrbOpt.required());
	EXPECT_TRUE(!vrbOpt.argumentRequired());
	EXPECT_TRUE(!vrbOpt.takesArgument());

	EXPECT_TRUE(optOpt.shortName() == "O");
	EXPECT_TRUE(optOpt.fullName() == "optimize");
	EXPECT_TRUE(!optOpt.repeatable());
	EXPECT_TRUE(!optOpt.required());
	EXPECT_TRUE(optOpt.argumentName() == "level");
	EXPECT_TRUE(optOpt.takesArgument());
	EXPECT_TRUE(!optOpt.argumentRequired());
}

ADD_TEST(OptionsTest, TestOptionsAdd) {

	//removeAllOptions();

	EXPECT_TRUE(AddOption("helper", "H", "start helper"));
	EXPECT_TRUE(AddOption("help", "h", "print help text"));
	EXPECT_TRUE(AddOption("include-dir", "I", "specify a search path for locating header files", false, true, "path"));
	EXPECT_TRUE(AddOption("library-dir", "L", "specify a search path for locating library files", false, true, "path"));
	EXPECT_TRUE(AddOption("insert", "it", "insert something", false, true, "path"));
	EXPECT_TRUE(!AddOption("item", "", "insert something", false, true, "path"));
	EXPECT_TRUE(AddOption("include", "J", "specify a search path for locating header files", false, true, "path"));
	EXPECT_TRUE(!AddOption("include", "J", "specify a search path for locating header files"));

	EXPECT_TRUE(GetOption("include"));
	EXPECT_TRUE(GetOption("insert"));
	EXPECT_TRUE(!GetOption("Insert"));
	EXPECT_TRUE(!GetOption("item"));
	EXPECT_TRUE(!GetOption("i"));
	EXPECT_TRUE(!GetOption("he"));
	EXPECT_TRUE(!GetOption("in"));
	EXPECT_TRUE(GetOption("help"));
	EXPECT_TRUE(!GetOption("helpe"));
	EXPECT_TRUE(GetOption("helper"));

	_Options.remove("include-dir");
	EXPECT_TRUE(!GetOption("include-dir"));
}

void TestProcessInclude(Exception& ex, const string& name, const string& value) {
	EXPECT_TRUE(name == "include-dir");
	static int i = 0;
    static const char* res[] = {
		"include",
		"/usr/include",
		"/usr/local/include",
		"/proj/include",
		"/usr/include" };
	EXPECT_TRUE(value == res[i++]);
}

void TestProcessVerbose(Exception& ex, const string& name, const string& value) {
	EXPECT_TRUE(name == "verbose");
	EXPECT_TRUE(value.empty());
}

void TestProcessOptimize(Exception& ex, const string& name, const string& value) {
	EXPECT_TRUE(name == "optimize");
	static int counter = 0;
	if (++counter < 3)
		EXPECT_TRUE(value.empty())
	else if (counter == 3)
		EXPECT_TRUE(value == "1")
	else
		EXPECT_TRUE(value == "2")
}

ADD_TEST(OptionsTest, TestProcess) {
	//_Options.clear();
	Exception ex;
	EXPECT_TRUE(AddOption("include-dir", "I", "specify an include search path", false, true, "path", true));

    const char* arg[] = { "row for path",
					"/I:include",
					"-I=/usr/include",
					"/include-dir:/usr/local/include",
					"-include-dir=/proj/include",
					"/include-dir=/usr/include"};

	EXPECT_TRUE(_Options.process(ex, (sizeof(arg) / sizeof(char *)), arg, TestProcessInclude));

	EXPECT_TRUE(!ProcessArg("/I"));
	
	EXPECT_TRUE(!ProcessArg("/include-dir"));

	EXPECT_TRUE(!ProcessArg("/Llib"));
	
	EXPECT_TRUE(AddOption("verbose", "v", "enable verbose mode", false, false));
	
	EXPECT_TRUE(ProcessArg("/v", TestProcessVerbose));
	
	EXPECT_TRUE(ProcessArg("/verbose", TestProcessVerbose));

	EXPECT_TRUE(!ProcessArg("/v2"));

	// TODO If argument specified but not expected => must be false
	//EXPECT_TRUE(!ProcessArg("/verbose:2"));

	EXPECT_TRUE(AddOption("optimize", "O", "enable optimization", false, false, "level", false));
	
	EXPECT_TRUE(ProcessArg("/O", TestProcessOptimize));
	EXPECT_TRUE(ProcessArg("-optimize=", TestProcessOptimize));

	EXPECT_TRUE(ProcessArg("-optimize:1", TestProcessOptimize));
	EXPECT_TRUE(ProcessArg("/O=2", TestProcessOptimize));
	
}
