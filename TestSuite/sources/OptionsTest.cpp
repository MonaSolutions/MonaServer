
#include "OptionsTest.h"
#include <vector>

using namespace Mona;

using namespace std;

Mona::Options OptionsTest::opts;

void OptionsTest::removeAllOptions() {
	vector<Option> optlist;
	auto it = opts.begin(), end = opts.end();
	for(;it != end;it++) {
		optlist.push_back(*it);
	}

	for(Option& opt : optlist) {
		opts.remove(opt.fullName());
	}
	optlist.clear();
}

::testing::AssertionResult OptionsTest::AddOption(const string& fullName, const string& shortName, const string& description, 
														 bool required, bool repeatable, const string& argName, bool argRequired) {

	Exception ex;
	opts.add(ex,
		Option(fullName, shortName, description)
			.required(false)
			.repeatable(false)
			.argument(argName, argRequired));

	if (ex)
		return ::testing::AssertionFailure() << "Exception : " << ex.error();

	return ::testing::AssertionSuccess();
}

::testing::AssertionResult OptionsTest::GetOption(const string& fullName) {
	
	Exception ex;
	opts.get(ex, fullName);
	
	if (ex)
		return ::testing::AssertionFailure() << "Exception : " << ex.error();

	return ::testing::AssertionSuccess();
}

TEST_F(OptionsTest, TestOption) {

	Option incOpt = Option("include-dir", "I", "specify an include search path")
		.required(false)
		.repeatable(true)
		.argument("path");
		
	Option libOpt = Option("library-dir", "L", "specify a library search path", false)
		.repeatable(true)
		.argument("path");
		
	Option outOpt = Option("output", "o", "specify the output file", true)
		.argument("file", true);

	Option vrbOpt = Option("verbose", "v")
		.description("enable verbose mode")
		.required(false)
		.repeatable(false);
		
	Option optOpt = Option("optimize", "O")
		.description("enable optimization")
		.required(false)
		.repeatable(false)
		.argument("level", false);
		
	EXPECT_EQ(incOpt.shortName(),"I");
	EXPECT_EQ(incOpt.fullName(),"include-dir");
	EXPECT_TRUE(incOpt.repeatable());
	EXPECT_TRUE(!incOpt.required());
	EXPECT_EQ(incOpt.argumentName(),"path");
	EXPECT_TRUE(incOpt.argumentRequired());
	EXPECT_TRUE(incOpt.takesArgument());
		
	EXPECT_EQ(libOpt.shortName(), "L");
	EXPECT_EQ(libOpt.fullName(), "library-dir");
	EXPECT_TRUE(libOpt.repeatable());
	EXPECT_TRUE(!libOpt.required());
	EXPECT_EQ(libOpt.argumentName(), "path");
	EXPECT_TRUE(libOpt.argumentRequired());
	EXPECT_TRUE(incOpt.takesArgument());

	EXPECT_EQ(outOpt.shortName(), "o");
	EXPECT_EQ(outOpt.fullName(), "output");
	EXPECT_TRUE(!outOpt.repeatable());
	EXPECT_TRUE(outOpt.required());
	EXPECT_EQ(outOpt.argumentName(), "file");
	EXPECT_TRUE(outOpt.argumentRequired());
	EXPECT_TRUE(incOpt.takesArgument());

	EXPECT_EQ(vrbOpt.shortName(), "v");
	EXPECT_EQ(vrbOpt.fullName(), "verbose");
	EXPECT_TRUE(!vrbOpt.repeatable());
	EXPECT_TRUE(!vrbOpt.required());
	EXPECT_TRUE(!vrbOpt.argumentRequired());
	EXPECT_TRUE(!vrbOpt.takesArgument());

	EXPECT_EQ(optOpt.shortName(), "O");
	EXPECT_EQ(optOpt.fullName(), "optimize");
	EXPECT_TRUE(!optOpt.repeatable());
	EXPECT_TRUE(!optOpt.required());
	EXPECT_EQ(optOpt.argumentName(), "level");
	EXPECT_TRUE(optOpt.takesArgument());
	EXPECT_TRUE(!optOpt.argumentRequired());
}

TEST_F(OptionsTest, TestOptionsAdd) {

	//removeAllOptions();

	EXPECT_TRUE(OptionsTest::AddOption("helper", "H", "start helper"));
	EXPECT_TRUE(OptionsTest::AddOption("help", "h", "print help text"));
	EXPECT_TRUE(OptionsTest::AddOption("include-dir", "I", "specify a search path for locating header files", false, true, "path"));
	EXPECT_TRUE(OptionsTest::AddOption("library-dir", "L", "specify a search path for locating library files", false, true, "path"));
	EXPECT_TRUE(OptionsTest::AddOption("insert", "it", "insert something", false, true, "path"));
	EXPECT_FALSE(OptionsTest::AddOption("item", "", "insert something", false, true, "path"));
	EXPECT_TRUE(OptionsTest::AddOption("include", "J", "specify a search path for locating header files", false, true, "path"));
	EXPECT_FALSE(OptionsTest::AddOption("include", "J", "specify a search path for locating header files"));

	EXPECT_TRUE(OptionsTest::GetOption("include"));
	EXPECT_TRUE(OptionsTest::GetOption("insert"));
	EXPECT_FALSE(OptionsTest::GetOption("Insert"));
	EXPECT_FALSE(OptionsTest::GetOption("item"));
	EXPECT_FALSE(OptionsTest::GetOption("i"));
	EXPECT_FALSE(OptionsTest::GetOption("he"));
	EXPECT_FALSE(OptionsTest::GetOption("in"));
	EXPECT_TRUE(OptionsTest::GetOption("help"));
	EXPECT_TRUE(OptionsTest::GetOption("helpe"));
	EXPECT_TRUE(OptionsTest::GetOption("helper"));

	Exception ex;
	const Option& opt1 = opts.get(ex, "include");
	EXPECT_EQ(opt1.fullName(), "include");

	const Option& opt2 = opts.get(ex,"item");
	EXPECT_EQ(opt2.fullName(), "item");

	const Option& opt4 = opts.get(ex,"include-d");
	EXPECT_EQ(opt4.fullName(), "include-dir");

	const Option& opt5 = opts.get(ex,"help");
	EXPECT_EQ(opt5.fullName(), "help");

	const Option& opt6 = opts.get(ex,"helper");
	EXPECT_EQ(opt6.fullName(), "helper");
}


TEST_F(OptionsTest, TestProcess) {

	//removeAllOptions();

	Exception ex;
	opts.add(ex,
			 Option("include-dir", "I", "specify an include search path")
		.required(false)
		.repeatable(true)
		.argument("path"));

	/*string arg[] = {"Iinclude",
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
	EXPECT_TRUE(arg.empty());*/
}

/*
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
