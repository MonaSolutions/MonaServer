
#include "OptionsTest.h"
#include "Mona/Logs.h"
#include <vector>

using namespace Mona;
using namespace std;

bool OptionsTest::AddOption(const string& fullName, const string& shortName, const string& description, 
		bool required, bool repeatable, const string& argName, bool argRequired) {

	Exception ex;
	opts.add(ex, fullName.c_str(), shortName.c_str());

	if (ex) {
		DEBUG("Exception : ", ex.error());
		return false;
	}

	return true;
}

bool OptionsTest::GetOption(const string& fullName) {
	
	Exception ex;
	opts.get(ex, fullName);
	
	if (ex) {
		DEBUG("Exception : ", ex.error());
		return false;
	}

	return true;
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

	EXPECT_TRUE(OptionsTest::AddOption("helper", "H", "start helper"));
	EXPECT_TRUE(OptionsTest::AddOption("help", "h", "print help text"));
	EXPECT_TRUE(OptionsTest::AddOption("include-dir", "I", "specify a search path for locating header files", false, true, "path"));
	EXPECT_TRUE(OptionsTest::AddOption("library-dir", "L", "specify a search path for locating library files", false, true, "path"));
	EXPECT_TRUE(OptionsTest::AddOption("insert", "it", "insert something", false, true, "path"));
	EXPECT_TRUE(!OptionsTest::AddOption("item", "", "insert something", false, true, "path"));
	EXPECT_TRUE(OptionsTest::AddOption("include", "J", "specify a search path for locating header files", false, true, "path"));
	EXPECT_TRUE(!OptionsTest::AddOption("include", "J", "specify a search path for locating header files"));

	EXPECT_TRUE(OptionsTest::GetOption("include"));
	EXPECT_TRUE(OptionsTest::GetOption("insert"));
	EXPECT_TRUE(!OptionsTest::GetOption("Insert"));
	EXPECT_TRUE(!OptionsTest::GetOption("item"));
	EXPECT_TRUE(!OptionsTest::GetOption("i"));
	EXPECT_TRUE(!OptionsTest::GetOption("he"));
	EXPECT_TRUE(!OptionsTest::GetOption("in"));
	EXPECT_TRUE(OptionsTest::GetOption("help"));
	EXPECT_TRUE(!OptionsTest::GetOption("helpe"));
	EXPECT_TRUE(OptionsTest::GetOption("helper"));
}


/*ADD_TEST(OptionsTest, TestProcess) {

	//removeAllOptions();

	Exception ex;
	opts.add(ex,
			 Option("include-dir", "I", "specify an include search path")
		.required(false)
		.repeatable(true)
		.argument("path"));

	string arg[] = {"Iinclude",
					"I/usr/include",
					"include:/usr/local/include",
					"include=/proj/include",
					"include-dir=/usr/include",
	};
	opts.process(ex, );
	EXPECT_TRUE(arg == "include");
	incOpt.process("I/usr/include", arg);
	EXPECT_TRUE(arg == "/usr/include");
	incOpt.process("include:/usr/local/include", arg);
	EXPECT_TRUE(arg == "/usr/local/include");
	incOpt.process("include=/proj/include", arg);
	EXPECT_TRUE(arg == "/proj/include");
	incOpt.process("include-dir=/usr/include", arg);
	EXPECT_TRUE(arg == "/usr/include");
	incOpt.process("Include-dir:/proj/include", arg);
	EXPECT_TRUE(arg == "/proj/include");
	
	try
	{
		incOpt.process("I", arg);
		fail("argument required - must throw");
	}
	catch (Poco::Util::MissingArgumentException&)
	{
	}

	try
	{
		incOpt.process("Include", arg);
		fail("argument required - must throw");
	}
	catch (Poco::Util::MissingArgumentException&)
	{
	}
	
	try
	{
		incOpt.process("Llib", arg);
		fail("wrong option - must throw");
	}
	catch (Poco::Util::UnknownOptionException&)
	{
	}
	
	Option vrbOpt = Option("verbose", "v")
		.description("enable verbose mode")
		.required(false)
		.repeatable(false);
	
	vrbOpt.process("v", arg);
	EXPECT_TRUE(arg.empty());
	vrbOpt.process("verbose", arg);
	EXPECT_TRUE(arg.empty());
	
	try
	{
		vrbOpt.process("v2", arg);
		fail("no argument expected - must throw");
	}
	catch (Poco::Util::UnexpectedArgumentException&)
	{
	}

	try
	{
		vrbOpt.process("verbose:2", arg);
		fail("no argument expected - must throw");
	}
	catch (Poco::Util::UnexpectedArgumentException&)
	{
	}
	
	Option optOpt = Option("optimize", "O")
		.description("enable optimization")
		.required(false)
		.repeatable(false)
		.argument("level", false);
		
	optOpt.process("O", arg);
	EXPECT_TRUE(arg.empty());
	optOpt.process("O2", arg);
	EXPECT_TRUE(arg == "2");
	optOpt.process("optimize:1", arg);
	EXPECT_TRUE(arg == "1");
	optOpt.process("opt", arg);
	EXPECT_TRUE(arg.empty());
	optOpt.process("opt=3", arg);
	EXPECT_TRUE(arg == "3");
	optOpt.process("opt=", arg);
	EXPECT_TRUE(arg.empty());
}


void OptionTest::testProcess2()
{
	Option incOpt = Option("include-dir", "", "specify an include search path")
		.required(false)
		.repeatable(true)
		.argument("path");

	string arg;
	incOpt.process("include:/usr/local/include", arg);
	EXPECT_TRUE(arg == "/usr/local/include");
	incOpt.process("include=/proj/include", arg);
	EXPECT_TRUE(arg == "/proj/include");
	incOpt.process("include-dir=/usr/include", arg);
	EXPECT_TRUE(arg == "/usr/include");
	incOpt.process("Include-dir:/proj/include", arg);
	EXPECT_TRUE(arg == "/proj/include");
	
	try
	{
		incOpt.process("Iinclude", arg);
		fail("unknown option - must throw");
	}
	catch (Poco::Util::UnknownOptionException&)
	{
	}
	
	try
	{
		incOpt.process("I", arg);
		fail("argument required - must throw");
	}
	catch (Poco::Util::MissingArgumentException&)
	{
	}

	try
	{
		incOpt.process("Include", arg);
		fail("argument required - must throw");
	}
	catch (Poco::Util::MissingArgumentException&)
	{
	}
	
	try
	{
		incOpt.process("Llib", arg);
		fail("wrong option - must throw");
	}
	catch (Poco::Util::UnknownOptionException&)
	{
	}
	
	Option vrbOpt = Option("verbose", "")
		.description("enable verbose mode")
		.required(false)
		.repeatable(false);
	
	vrbOpt.process("v", arg);
	EXPECT_TRUE(arg.empty());
	vrbOpt.process("verbose", arg);
	EXPECT_TRUE(arg.empty());
	
	try
	{
		vrbOpt.process("v2", arg);
		fail("no argument expected - must throw");
	}
	catch (Poco::Util::UnknownOptionException&)
	{
	}

	try
	{
		vrbOpt.process("verbose:2", arg);
		fail("no argument expected - must throw");
	}
	catch (Poco::Util::UnexpectedArgumentException&)
	{
	}
}
*/
